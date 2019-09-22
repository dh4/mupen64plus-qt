/***
 * Copyright (c) 2013, Dan Hasting
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

#include "../global.h"
#include "../common.h"

#include "thegamesdbscraper.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTime>

#include <QtSql/QSqlQuery>


RomCollection::RomCollection(QStringList fileTypes, QStringList romPaths, QWidget *parent) : QObject(parent)
{
    this->fileTypes = fileTypes;
    this->romPaths = romPaths;
    this->romPaths.removeAll("");
    this->parent = parent;

    setupDatabase();
}


Rom RomCollection::addRom(QByteArray *romData, QString fileName, QString directory, QString zipFile,
                          QSqlQuery query, bool ddRom)
{
    Rom currentRom;

    currentRom.fileName = fileName;
    currentRom.directory = directory;

    if (ddRom)
        currentRom.internalName = "";
    else
        currentRom.internalName = QString(romData->mid(32, 20)).trimmed();

    currentRom.romMD5 = QString(QCryptographicHash::hash(*romData,
                                QCryptographicHash::Md5).toHex());
    currentRom.zipFile = zipFile;
    currentRom.sortSize = romData->size();

    query.bindValue(":filename",      currentRom.fileName);
    query.bindValue(":directory",     currentRom.directory);
    query.bindValue(":internal_name", currentRom.internalName);
    query.bindValue(":md5",           currentRom.romMD5);
    query.bindValue(":zip_file",      currentRom.zipFile);
    query.bindValue(":size",          currentRom.sortSize);

    if (ddRom)
        query.bindValue(":dd_rom", 1);
    else
        query.bindValue(":dd_rom", 0);

    query.exec();

    if (!ddRom)
        initializeRom(&currentRom, false);

    return currentRom;
}


int RomCollection::addRoms()
{
    emit updateStarted();

    //Count files so we know how to setup the progress dialog
    int totalCount = 0;

    foreach (QString romPath, romPaths) {
        QDir romDir(romPath);

        if (romDir.exists()) {
            QStringList files = scanDirectory(romDir);
            totalCount += files.size();
        }
    }

    QList<Rom> roms;
    QList<Rom> ddRoms;

    database.open();
    QSqlQuery query("DELETE FROM rom_collection", database);

    if (totalCount != 0) {
        int count = 0;
        setupProgressDialog(totalCount);

        query.prepare(QString("INSERT INTO rom_collection ")
                      + "(filename, directory, internal_name, md5, zip_file, size, dd_rom) "
                      + "VALUES (:filename, :directory, :internal_name, :md5, :zip_file, :size, :dd_rom)");

        scraper = new TheGamesDBScraper(parent);

        foreach (QString romPath, romPaths)
        {
            QDir romDir(romPath);
            QStringList files = scanDirectory(romDir);

            int romCount = 0;

            foreach (QString fileName, files)
            {
                QString completeFileName = romDir.absoluteFilePath(fileName);
                QFile file(completeFileName);

                //If file is a zip file, extract info from any zipped ROMs
                if (QFileInfo(file).suffix().toLower() == "zip") {
                    foreach (QString zippedFile, getZippedFiles(completeFileName))
                    {
                        //check for ROM files
                        QByteArray *romData = getZippedRom(zippedFile, completeFileName);

                        if (fileTypes.contains("*.v64"))
                            *romData = byteswap(*romData);

                        if (romData->left(4).toHex() == "80371240") { //Z64 ROM
                            roms.append(addRom(romData, zippedFile, romPath, fileName, query));
                            romCount++;
                        } else if (romData->left(4).toHex() == "e848d316") { //64DD ROM
                            ddRoms.append(addRom(romData, zippedFile, romPath, fileName, query, true));
                            romCount++;
                        }

                        delete romData;
                    }
                } else { //Just a normal file
                    file.open(QIODevice::ReadOnly);
                    QByteArray *romData = new QByteArray(file.readAll());
                    file.close();

                    if (fileTypes.contains("*.v64"))
                        *romData = byteswap(*romData);

                    if (romData->left(4).toHex() == "80371240") { //Z64 ROM
                        roms.append(addRom(romData, fileName, romPath, "", query));
                        romCount++;
                    } else if (romData->left(4).toHex() == "e848d316") { //64DD ROM
                        ddRoms.append(addRom(romData, fileName, romPath, "", query, true));
                        romCount++;
                    }

                    delete romData;
                }

                count++;
                progress->setValue(count);
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }

            if (romCount == 0)
                QMessageBox::warning(parent, tr("Warning"), tr("No ROMs found in ") + romPath + ".");
        }

        delete scraper;
        progress->close();
    } else if (romPaths.size() != 0) {
        QMessageBox::warning(parent, tr("Warning"), tr("No ROMs found."));
    }

    database.close();

    //Emit signals for regular roms
    std::sort(roms.begin(), roms.end(), romSorter);

    for (int i = 0; i < roms.size(); i++)
        emit romAdded(&roms[i], i);

    //Emit signals for 64DD roms
    std::sort(ddRoms.begin(), ddRoms.end(), romSorter);

    for (int i = 0; i < ddRoms.size(); i++)
        emit ddRomAdded(&ddRoms[i]);

    emit updateEnded(roms.size());

    return roms.size();
}


int RomCollection::cachedRoms(bool imageUpdated, bool onStartup)
{
    emit updateStarted(imageUpdated);

    database.open();
    QSqlQuery query(QString("SELECT filename, directory, md5, internal_name, zip_file, size, dd_rom ")
                    + "FROM rom_collection", database);

    query.last();
    int romCount = query.at() + 1;
    query.seek(-1);

    if (romCount == -1) //Nothing cached so try adding ROMs instead
        return addRoms();


    //Check if user has data from TheGamesDB API v1 and update them to v2 data
    if (onStartup) {
        bool onV1 = false;
        QDir cacheDir(getCacheLocation());

        if (!cacheDir.exists() && SETTINGS.value("Other/downloadinfo", "").toString() == "true")
            onV1 = true;

        if (onV1)
            return addRoms();
    }


    QList<Rom> roms;
    QList<Rom> ddRoms;

    int count = 0;
    bool showProgress = false;
    QTime checkPerformance;

    while (query.next())
    {
        Rom currentRom;

        currentRom.fileName = query.value(0).toString();
        currentRom.directory = query.value(1).toString();
        currentRom.romMD5 = query.value(2).toString();
        currentRom.internalName = query.value(3).toString();
        currentRom.zipFile = query.value(4).toString();
        currentRom.sortSize = query.value(5).toInt();
        int ddRom = query.value(6).toInt();

        //Check performance of adding first item to see if progress dialog needs to be shown
        if (count == 0) checkPerformance.start();

        if (ddRom == 1)
            ddRoms.append(currentRom);
        else {
            initializeRom(&currentRom, true);
            roms.append(currentRom);
        }

        if (count == 0) {
            int runtime = checkPerformance.elapsed();

            //check if operation expected to take longer than two seconds
            if (runtime * romCount > 2000) {
                setupProgressDialog(romCount);
                showProgress = true;
            }
        }

        count++;

        if (showProgress) {
            progress->setValue(count);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }

    database.close();

    if (showProgress)
        progress->close();

    //Emit signals for regular roms
    std::sort(roms.begin(), roms.end(), romSorter);

    for (int i = 0; i < roms.size(); i++)
        emit romAdded(&roms[i], i);

    //Emit signals for 64DD roms
    std::sort(ddRoms.begin(), ddRoms.end(), romSorter);

    for (int i = 0; i < ddRoms.size(); i++)
        emit ddRomAdded(&ddRoms[i]);

    emit updateEnded(roms.size(), true);

    return roms.size();
}


QStringList RomCollection::getFileTypes(bool archives)
{
    QStringList returnList = fileTypes;

    if (!archives)
        returnList.removeOne("*.zip");

    return returnList;
}


void RomCollection::initializeRom(Rom *currentRom, bool cached)
{
    QSettings *romCatalog = new QSettings(parent);

    QString catalogFile = SETTINGS.value("Paths/catalog", "").toString();
    if (catalogFile == "") {
        QString dataPath = SETTINGS.value("Paths/data", "").toString();
        QDir dataDir(dataPath);

        if (QFileInfo(dataDir.absoluteFilePath("mupen64plus.ini")).exists())
            catalogFile = dataDir.absoluteFilePath("mupen64plus.ini");
    }

    QDir romDir(currentRom->directory);

    //Default text for GoodName to notify user
    currentRom->goodName = getTranslation("Requires catalog file");
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
        QVariant gNameRaw = romCatalog->value(currentRom->romMD5+"/GoodName",getTranslation("Unknown ROM"));
        currentRom->goodName = gNameRaw.toStringList().join(", ");

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
        if (currentRom->goodName != getTranslation("Unknown ROM") &&
            currentRom->goodName != getTranslation("Requires catalog file")) {
            scraper->downloadGameInfo(currentRom->romMD5, currentRom->goodName);
        } else {
            //tweak internal name by adding spaces to get better results
            QString search = currentRom->internalName;
            search.replace(QRegExp("([a-z])([A-Z])"),"\\1 \\2");
            search.replace(QRegExp("([^ \\d])(\\d)"),"\\1 \\2");
            scraper->downloadGameInfo(currentRom->romMD5, search);
        }

    }

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true") {
        QString dataFile = getCacheLocation() + currentRom->romMD5.toLower() + "/data.json";
        QFile file(dataFile);

        file.open(QIODevice::ReadOnly);
        QString data = file.readAll();
        file.close();

        QJsonDocument document = QJsonDocument::fromJson(data.toUtf8());
        QJsonObject json = document.object();

        //Remove any non-standard characters
        QString regex = "[^A-Za-z 0-9 \\.,\\?'""!@#\\$%\\^&\\*\\(\\)-_=\\+;:<>\\/\\\\|\\}\\{\\[\\]`~é]*";

        currentRom->gameTitle = json.value("game_title").toString().remove(QRegExp(regex));
        if (currentRom->gameTitle == "") currentRom->gameTitle = getTranslation("Not found");

        currentRom->releaseDate = json.value("release_date").toString();
        currentRom->sortDate = json.value("release_date").toString();
        currentRom->releaseDate.replace(QRegExp("(\\d{4})-(\\d{2})-(\\d{2})"), "\\2/\\3/\\1");

        currentRom->overview = json.value("overview").toString().remove(QRegExp(regex));
        currentRom->esrb = json.value("rating").toString();

        currentRom->genre = json.value("genres").toString();
        currentRom->publisher = json.value("publisher").toString();
        currentRom->developer = json.value("developer").toString();

        foreach (QString ext, QStringList() << "jpg" << "png")
        {
            QString imageFile = getCacheLocation() + currentRom->romMD5.toLower() + "/boxart-front." + ext;
            QFile cover(imageFile);

            if (cover.exists() && currentRom->image.load(imageFile)) {
                currentRom->imageExists = true;
                break;
            }
        }
    }
}


QStringList RomCollection::scanDirectory(QDir romDir)
{
    QStringList files = romDir.entryList(fileTypes, QDir::Files | QDir::NoSymLinks);

    QStringList dirs = romDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    foreach (QString dir, dirs)
    {
        QString subDir = romDir.absolutePath() + "/" + dir;
        QStringList subFiles = QDir(subDir).entryList(fileTypes, QDir::Files | QDir::NoSymLinks);
        foreach (QString subFile, subFiles) files << dir + "/" + subFile;
    }

    return files;
}


void RomCollection::setupDatabase()
{
    // Bump this when updating rom_collection structure
    // Will cause clients to delete and recreate the table
    int dbVersion = 2;

    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(getDataLocation() + "/"+AppNameLower+".sqlite");

    if (!database.open())
        QMessageBox::warning(parent, tr("Database Not Loaded"),
                             tr("Could not connect to Sqlite database. Application may misbehave."));

    QSqlQuery version = database.exec("PRAGMA user_version");
    version.next();

    if (version.value(0).toInt() != dbVersion) { //old database version, reset rom_collection
        version.finish();

        database.exec("DROP TABLE rom_collection");
        database.exec("PRAGMA user_version = " + QString::number(dbVersion));
    }

    database.exec(QString()
                    + "CREATE TABLE IF NOT EXISTS rom_collection ("
                        + "rom_id INTEGER PRIMARY KEY ASC, "
                        + "filename TEXT NOT NULL, "
                        + "directory TEXT NOT NULL, "
                        + "md5 TEXT NOT NULL, "
                        + "internal_name TEXT, "
                        + "zip_file TEXT, "
                        + "size INTEGER, "
                        + "dd_rom INTEGER)");

    database.close();
}


void RomCollection::setupProgressDialog(int size)
{
    progress = new QProgressDialog(tr("Loading ROMs..."), tr("Cancel"), 0, size, parent);
    progress->setWindowFlags(progress->windowFlags() & ~Qt::WindowCloseButtonHint);
    progress->setWindowFlags(progress->windowFlags() & ~Qt::WindowMinimizeButtonHint);
    progress->setWindowFlags(progress->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    progress->setCancelButton(nullptr);
    progress->setWindowModality(Qt::WindowModal);

    progress->show();
}


void RomCollection::updatePaths(QStringList romPaths)
{
    this->romPaths = romPaths;
    this->romPaths.removeAll("");
}
