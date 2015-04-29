/***
 * Copyright (c) 2013, Presence
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the organization nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ***/

#include "romcollection.h"


RomCollection::RomCollection(QStringList fileTypes, QString romPath, QWidget *parent) : QObject(parent)
{
    this->fileTypes = fileTypes;
    this->romPath = romPath;
    this->parent = parent;

    this->romDir = QDir(romPath);

    setupDatabase();
}


Rom RomCollection::addRom(QByteArray *romData, QString fileName, QString zipFile, QSqlQuery query)
{
    Rom currentRom;

    currentRom.fileName = fileName;
    currentRom.internalName = QString(romData->mid(32, 20)).trimmed();
    currentRom.romMD5 = QString(QCryptographicHash::hash(*romData,
                                QCryptographicHash::Md5).toHex());
    currentRom.zipFile = zipFile;
    currentRom.sortSize = romData->size();

    query.bindValue(":filename",      currentRom.fileName);
    query.bindValue(":internal_name", currentRom.internalName);
    query.bindValue(":md5",           currentRom.romMD5);
    query.bindValue(":zip_file",      currentRom.zipFile);
    query.bindValue(":size",          currentRom.sortSize);
    query.exec();

    initializeRom(&currentRom, romDir, false);

    return currentRom;
}


void RomCollection::addRoms()
{
    emit updateStarted();

    database.open();

    QSqlQuery query("DELETE FROM rom_collection", database);
    query.prepare(QString("INSERT INTO rom_collection ")
                  + "(filename, internal_name, md5, zip_file, size) "
                  + "VALUES (:filename, :internal_name, :md5, :zip_file, :size)");

    scrapper = new TheGamesDBScrapper(parent);

    QList<Rom> roms;

    if (romPath != "") {
        if (romDir.exists()) {
            QStringList files = romDir.entryList(fileTypes, QDir::Files | QDir::NoSymLinks);

            if (files.size() > 0) {
                setupProgressDialog(files.size());

                int count = 0;
                foreach (QString fileName, files)
                {
                    QString completeFileName = romDir.absoluteFilePath(fileName);
                    QFile file(completeFileName);

                    //If file is a zip file, extract info from any zipped ROMs
                    if (QFileInfo(file).suffix().toLower() == "zip") {
                        foreach (QString zippedFile, getZippedFiles(completeFileName))
                        {
                            QString ext = zippedFile.right(4).toLower();

                            //check for ROM files
                            if (fileTypes.contains("*" + ext)) {
                                QByteArray *romData = getZippedRom(zippedFile, completeFileName);

                                if (fileTypes.contains("*.v64"))
                                    *romData = byteswap(*romData);

                                if (romData->left(4).toHex() == "80371240") //Else invalid
                                    roms.append(addRom(romData, zippedFile, fileName, query));

                                delete romData;
                            }
                        }
                    } else { //Just a normal ROM file
                        file.open(QIODevice::ReadOnly);
                        QByteArray *romData = new QByteArray(file.readAll());
                        file.close();

                        if (fileTypes.contains("*.v64"))
                            *romData = byteswap(*romData);

                        if (romData->left(4).toHex() == "80371240") //Else invalid
                            roms.append(addRom(romData, fileName, "", query));

                        delete romData;
                    }

                    count++;
                    progress->setValue(count);
                }

                progress->close();
            } else {
            QMessageBox::warning(parent, "Warning", "No ROMs found.");
            }
        } else {
            QMessageBox::warning(parent, "Warning", "Failed to open ROM directory.");
        }
    }

    delete scrapper;

    database.close();

    qSort(roms.begin(), roms.end(), romSorter);

    for (int i = 0; i < roms.size(); i++)
        emit romAdded(&roms[i], i);

    emit updateEnded(roms.size());
}


void RomCollection::cachedRoms(bool imageUpdated)
{
    emit updateStarted(imageUpdated);

    database.open();
    QSqlQuery query("SELECT filename, md5, internal_name, zip_file, size FROM rom_collection", database);

    query.last();
    int romCount = query.at() + 1;
    query.seek(-1);

    if (romCount == -1) { //Nothing cached so try adding ROMs instead
        addRoms();
        return;
    }

    QList<Rom> roms;

    int count = 0;
    bool showProgress = false;
    QTime checkPerformance;

    while (query.next())
    {
        Rom currentRom;

        currentRom.fileName = query.value(0).toString();
        currentRom.romMD5 = query.value(1).toString();
        currentRom.internalName = query.value(2).toString();
        currentRom.zipFile = query.value(3).toString();
        currentRom.sortSize = query.value(4).toInt();

        //Check performance of adding first item to see if progress dialog needs to be shown
        if (count == 0) checkPerformance.start();

        initializeRom(&currentRom, romDir, true);
        roms.append(currentRom);

        if (count == 0) {
            int runtime = checkPerformance.elapsed();

            //check if operation expected to take longer than two seconds
            if (runtime * romCount > 2000) {
                setupProgressDialog(romCount);
                showProgress = true;
            }
        }

        count++;

        if (showProgress)
            progress->setValue(count);
    }

    database.close();

    if (showProgress)
        progress->close();

    qSort(roms.begin(), roms.end(), romSorter);

    for (int i = 0; i < roms.size(); i++)
        emit romAdded(&roms[i], i);

    emit updateEnded(roms.size(), true);
}


QStringList RomCollection::getFileTypes(bool archives)
{
    QStringList returnList = fileTypes;

    if (!archives)
        returnList.removeOne("*.zip");

    return returnList;
}


void RomCollection::initializeRom(Rom *currentRom, QDir romDir, bool cached)
{
    QSettings *romCatalog = new QSettings(parent);
    QString dataPath = SETTINGS.value("Paths/data", "").toString();
    QDir dataDir(dataPath);
    QString catalogFile = dataDir.absoluteFilePath("mupen64plus.ini");

    //Default text for GoodName to notify user
    currentRom->goodName = "Requires catalog file";
    currentRom->imageExists = false;

    bool getGoodName = false;
    if (QFileInfo(catalogFile).exists()) {
        romCatalog = new QSettings(catalogFile, QSettings::IniFormat, parent);
        getGoodName = true;
    }

    QFile file(romDir.absoluteFilePath(currentRom->fileName));

    currentRom->romMD5 = currentRom->romMD5.toUpper();
    currentRom->baseName = QFileInfo(file).completeBaseName();
    currentRom->size = QObject::tr("%1 MB").arg((currentRom->sortSize + 1023) / 1024 / 1024);

    if (getGoodName) {
        //Join GoodName on ", ", otherwise entries with a comma won't show
        QVariant goodNameVariant = romCatalog->value(currentRom->romMD5+"/GoodName","Unknown ROM");
        currentRom->goodName = goodNameVariant.toStringList().join(", ");

        QStringList CRC = romCatalog->value(currentRom->romMD5+"/CRC","").toString().split(" ");

        if (CRC.size() == 2) {
            currentRom->CRC1 = CRC[0];
            currentRom->CRC2 = CRC[1];
        }

        QString newMD5 = romCatalog->value(currentRom->romMD5+"/RefMD5","").toString();
        if (newMD5 == "")
            newMD5 = currentRom->romMD5;

        currentRom->players = romCatalog->value(newMD5+"/Players","").toString();
        currentRom->saveType = romCatalog->value(newMD5+"/SaveType","").toString();
        currentRom->rumble = romCatalog->value(newMD5+"/Rumble","").toString();
    }

    if (!cached && SETTINGS.value("Other/downloadinfo", "").toString() == "true") {
        if (currentRom->goodName != "Unknown ROM" && currentRom->goodName != "Requires catalog file") {
            scrapper->downloadGameInfo(currentRom->romMD5, currentRom->goodName);
        } else {
            //tweak internal name by adding spaces to get better results
            QString search = currentRom->internalName;
            search.replace(QRegExp("([a-z])([A-Z])"),"\\1 \\2");
            search.replace(QRegExp("([^ \\d])(\\d)"),"\\1 \\2");
            scrapper->downloadGameInfo(currentRom->romMD5, search);
        }

    }

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true") {
        QString cacheDir = getDataLocation() + "/cache";

        QString dataFile = cacheDir + "/" + currentRom->romMD5.toLower() + "/data.xml";
        QFile file(dataFile);

        file.open(QIODevice::ReadOnly);
        QString dom = file.readAll();
        file.close();

        QDomDocument xml;
        xml.setContent(dom);
        QDomNode game = xml.elementsByTagName("Game").at(0);

        //Remove any non-standard characters
        QString regex = "[^A-Za-z 0-9 \\.,\\?'""!@#\\$%\\^&\\*\\(\\)-_=\\+;:<>\\/\\\\|\\}\\{\\[\\]`~]*";

        currentRom->gameTitle = game.firstChildElement("GameTitle").text().remove(QRegExp(regex));
        if (currentRom->gameTitle == "") currentRom->gameTitle = "Not found";

        currentRom->releaseDate = game.firstChildElement("ReleaseDate").text();

        //Fix missing 0's in date
        currentRom->releaseDate.replace(QRegExp("^(\\d)/(\\d{2})/(\\d{4})"), "0\\1/\\2/\\3");
        currentRom->releaseDate.replace(QRegExp("^(\\d{2})/(\\d)/(\\d{4})"), "\\1/0\\2/\\3");
        currentRom->releaseDate.replace(QRegExp("^(\\d)/(\\d)/(\\d{4})"), "0\\1/0\\2/\\3");

        currentRom->sortDate = currentRom->releaseDate;
        currentRom->sortDate.replace(QRegExp("(\\d{2})/(\\d{2})/(\\d{4})"), "\\3-\\1-\\2");

        currentRom->overview = game.firstChildElement("Overview").text().remove(QRegExp(regex));
        currentRom->esrb = game.firstChildElement("ESRB").text();

        int count = 0;
        QDomNode genreNode = game.firstChildElement("Genres").firstChild();
        while(!genreNode.isNull())
        {
            if (count != 0)
                currentRom->genre += "/" + genreNode.toElement().text();
            else
                currentRom->genre = genreNode.toElement().text();

            genreNode = genreNode.nextSibling();
            count++;
        }

        currentRom->publisher = game.firstChildElement("Publisher").text();
        currentRom->developer = game.firstChildElement("Developer").text();
        currentRom->rating = game.firstChildElement("Rating").text();

        foreach (QString ext, QStringList() << "jpg" << "png")
        {
            QString imageFile = getDataLocation() + "/cache/"
                                + currentRom->romMD5.toLower() + "/boxart-front." + ext;
            QFile cover(imageFile);

            if (cover.exists()&& currentRom->image.load(imageFile)) {
                currentRom->imageExists = true;
                break;
            }
        }
    }
}


void RomCollection::setupDatabase()
{
    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(getDataLocation() + "/mupen64plus-qt.sqlite");

    if (!database.open())
        QMessageBox::warning(parent, "Database Not Loaded",
                             "Could not connect to Sqlite database. Application may misbehave.");

    QSqlQuery query(QString()
                    + "CREATE TABLE IF NOT EXISTS rom_collection ("
                        + "rom_id INTEGER PRIMARY KEY ASC, "
                        + "filename TEXT NOT NULL, "
                        + "md5 TEXT NOT NULL, "
                        + "internal_name TEXT, "
                        + "zip_file TEXT, "
                        + "size INTEGER)", database);

    database.close();
}


void RomCollection::setupProgressDialog(int size)
{
    progress = new QProgressDialog("Loading ROMs...", "Cancel", 0, size, parent);
#if QT_VERSION >= 0x050000
    progress->setWindowFlags(progress->windowFlags() & ~Qt::WindowCloseButtonHint);
    progress->setWindowFlags(progress->windowFlags() & ~Qt::WindowMinimizeButtonHint);
    progress->setWindowFlags(progress->windowFlags() & ~Qt::WindowContextHelpButtonHint);
#else
    progress->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
#endif
    progress->setCancelButton(0);
    progress->setWindowModality(Qt::WindowModal);

    progress->show();
}


void RomCollection::updatePath(QString romPath)
{
    this->romPath = romPath;
    this->romDir = QDir(romPath);
}
