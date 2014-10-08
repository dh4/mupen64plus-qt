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

#include "common.h"


QByteArray byteswap(QByteArray romData)
{
    QByteArray flipped;

    if (romData.left(4).toHex() == "37804012") {
        for (int i = 0; i < romData.length(); i += 2)
        {
            flipped.append(romData[i + 1]);
            flipped.append(romData[i]);
        }
        return flipped;
    } else {
        return romData;
    }
}


void downloadGameInfo(QString identifier, QString searchName, QWidget *parent, QString gameID, bool force)
{
    if (identifier != "") {
        bool updated = false;

        QString gameCache = getDataLocation() + "/cache/" + identifier.toLower();
        QDir cache(gameCache);

        if (!cache.exists()) {
            cache.mkpath(gameCache);
        }

        //Get game XML info from thegamesdb.net
        QString dataFile = gameCache + "/data.xml";
        QFile file(dataFile);

        if (!file.exists() || file.size() == 0 || force) {
            QUrl url;

            //Remove [!], (U), etc. from GoodName for searching
            searchName.remove(QRegExp("\\W*(\\(|\\[).+(\\)|\\])\\W*"));

            //Few game specific hacks
            //TODO: Contact thegamesdb.net and see if these can be fixed on their end
            if (searchName == "Legend of Zelda, The - Majora's Mask")
                searchName = "Majora's Mask";
            else if (searchName == "Legend of Zelda, The - Ocarina of Time - Master Quest")
                searchName = "Master Quest";
            else if (searchName.toLower() == "f-zero x")
                gameID = "10836";

            //If user submits gameID, use that
            if (gameID != "")
                url.setUrl("http://thegamesdb.net/api/GetGame.php?id="
                           + gameID + "&platform=Nintendo 64");
            else
                url.setUrl("http://thegamesdb.net/api/GetGame.php?name="
                           + searchName + "&platform=Nintendo 64");

            QString dom = getUrlContents(url);

            QDomDocument xml;
            xml.setContent(dom);
            QDomNode node = xml.elementsByTagName("Data").at(0).firstChildElement("Game");

            int count = 0, found = 0;

            while(!node.isNull())
            {
                QDomElement element = node.firstChildElement("GameTitle").toElement();

                if (force) { //from user dialog
                    QDomElement date = node.firstChildElement("ReleaseDate").toElement();

                    QString check = "Game: " + element.text();
                    check.remove(QRegExp(QString("[^A-Za-z 0-9 \\.,\\?'""!@#\\$%\\^&\\*\\")
                                         + "(\\)-_=\\+;:<>\\/\\\\|\\}\\{\\[\\]`~]*"));
                    if (date.text() != "") check += "\nReleased on: " + date.text();
                    check += "\n\nDoes this look correct?";

                    int answer = QMessageBox::question(parent, QObject::tr("Game Information Download"),
                                                       check, QMessageBox::Yes | QMessageBox::No);

                    if (answer == QMessageBox::Yes) {
                        found = count;
                        updated = true;
                        break;
                    }
                } else {
                    //We only want one game, so search for a perfect match in the GameTitle element.
                    //Otherwise this will default to 0 (the first game found)
                    if(element.text() == searchName)
                        found = count;
                }

                node = node.nextSibling();
                count++;
            }

            if (!force || updated) {
                file.open(QIODevice::WriteOnly);
                QTextStream stream(&file);

                QDomNodeList gameList = xml.elementsByTagName("Game");
                gameList.at(found).save(stream, QDomNode::EncodingFromDocument);

                file.close();
            }

            if (force && !updated) {
                QString message;

                if (count == 0)
                    message = QObject::tr("No results found.");
                else
                    message = QObject::tr("No more results found.");

                QMessageBox::information(parent, QObject::tr("Game Information Download"), message);
            }
        }


        //Get front cover
        QString boxartURL = "";
        QString coverFile = gameCache + "/boxart-front.jpg";
        QFile cover(coverFile);

        if (!cover.exists() || (force && updated)) {
            file.open(QIODevice::ReadOnly);
            QString dom = file.readAll();
            file.close();

            QDomDocument xml;
            xml.setContent(dom);
            QDomNode node = xml.elementsByTagName("Game").at(0).firstChildElement("Images").firstChild();

            while(!node.isNull())
            {
                QDomElement element = node.toElement();
                if(element.tagName() == "boxart" && element.attribute("side") == "front")
                    boxartURL = element.attribute("thumb");

                node = node.nextSibling();
            }

            if (boxartURL != "") {
                QUrl url("http://thegamesdb.net/banners/" + boxartURL);

                cover.open(QIODevice::WriteOnly);
                cover.write(getUrlContents(url));
                cover.close();
            }
        }

        if (updated)
            QMessageBox::information(parent, QObject::tr("Game Information Download"),
                                     QObject::tr("Download Complete!"));
    }
}


QString getDataLocation()
{
    QString dataDir;

#ifdef Q_OS_WIN
    dataDir = QCoreApplication::applicationDirPath();
#else

#if QT_VERSION >= 0x050000
    dataDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation)
                    .replace("Mupen64Plus/Mupen64Plus-Qt","mupen64plus-qt");
#else
    dataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation)
                    .remove("data/").replace("Mupen64Plus/Mupen64Plus-Qt","mupen64plus-qt");
#endif

#endif

     QDir data(dataDir);
     if (!data.exists())
         data.mkpath(dataDir);

     return dataDir;
}


QColor getColor(QString color, int transparency)
{
    if (transparency <= 255) {
        if (color == "Black")           return QColor(0,   0,   0,   transparency);
        else if (color == "White")      return QColor(255, 255, 255, transparency);
        else if (color == "Light Gray") return QColor(200, 200, 200, transparency);
        else if (color == "Dark Gray")  return QColor(50,  50,  59,  transparency);
        else if (color == "Green")      return QColor(0,   255, 0,   transparency);
        else if (color == "Cyan")       return QColor(30,  175, 255, transparency);
        else if (color == "Blue")       return QColor(0,   0,   255, transparency);
        else if (color == "Purple")     return QColor(128, 0,   128, transparency);
        else if (color == "Red")        return QColor(255, 0,   0,   transparency);
        else if (color == "Pink")       return QColor(246, 96,  171, transparency);
        else if (color == "Orange")     return QColor(255, 165, 0,   transparency);
        else if (color == "Yellow")     return QColor(255, 255, 0,   transparency);
        else if (color == "Brown")      return QColor(127, 70,  44,  transparency);
    }

    return QColor(0, 0, 0, 255);
}


int getGridSize(QString which)
{
    QString size = SETTINGS.value("Grid/imagesize","Medium").toString();

    if (which == "height") {
        if (SETTINGS.value("Grid/label", "true").toString() == "true") {
            if (size == "Extra Small") return 65;
            if (size == "Small")       return 90;
            if (size == "Medium")      return 145;
            if (size == "Large")       return 190;
            if (size == "Extra Large") return 250;
        } else {
            if (size == "Extra Small") return 47;
            if (size == "Small")       return 71;
            if (size == "Medium")      return 122;
            if (size == "Large")       return 172;
            if (size == "Extra Large") return 224;
        }
    } else if (which == "width") {
        if (size == "Extra Small") return 60;
        if (size == "Small")       return 90;
        if (size == "Medium")      return 160;
        if (size == "Large")       return 225;
        if (size == "Extra Large") return 300;
    } else if (which == "font") {
        if (size == "Extra Small") return 5;
        if (size == "Small")       return 7;
        if (size == "Medium")      return 10;
        if (size == "Large")       return 12;
        if (size == "Extra Large") return 13;
    }
    return 0;
}


QSize getImageSize(QString view)
{
    QString size = SETTINGS.value(view+"/imagesize","Medium").toString();

    if (view == "Table") {
        if (size == "Extra Small") return QSize(33, 24);
        if (size == "Small")       return QSize(48, 35);
        if (size == "Medium")      return QSize(69, 50);
        if (size == "Large")       return QSize(103, 75);
        if (size == "Extra Large") return QSize(138, 100);
    } else if (view == "Grid" || view == "List") {
        if (size == "Extra Small") return QSize(48, 35);
        if (size == "Small")       return QSize(69, 50);
        if (size == "Medium")      return QSize(138, 100);
        if (size == "Large")       return QSize(203, 150);
        if (size == "Extra Large") return QSize(276, 200);
    }

    return QSize();
}


QString getRomInfo(QString identifier, const Rom *rom, bool removeWarn, bool sort)
{
    QString text = "";

    if (identifier == "GoodName")
        text = rom->goodName;
    else if (identifier == "Filename")
        text = rom->baseName;
    else if (identifier == "Filename (extension)")
        text = rom->fileName;
    else if (identifier == "Zip File")
        text = rom->zipFile;
    else if (identifier == "Internal Name")
        text = rom->internalName;
    else if (identifier == "Size")
        text = rom->size;
    else if (identifier == "MD5")
        text = rom->romMD5.toLower();
    else if (identifier == "CRC1")
        text = rom->CRC1.toLower();
    else if (identifier == "CRC2")
        text = rom->CRC2.toLower();
    else if (identifier == "Players")
        text = rom->players;
    else if (identifier == "Rumble")
        text = rom->rumble;
    else if (identifier == "Save Type")
        text = rom->saveType;
    else if (identifier == "Game Title")
        text = rom->gameTitle;
    else if (identifier == "Release Date")
        text = rom->releaseDate;
    else if (identifier == "Overview")
        text = rom->overview;
    else if (identifier == "ESRB")
        text = rom->esrb;
    else if (identifier == "Genre")
        text = rom->genre;
    else if (identifier == "Publisher")
        text = rom->publisher;
    else if (identifier == "Developer")
        text = rom->developer;
    else if (identifier == "Rating")
        text = rom->rating;

    if (!removeWarn)
        return text;
    else if (text == "Unknown ROM" || text == "Requires catalog file" || text == "Not found") {
        if (sort)
            return "ZZZ"; //Sort warnings at the end
        else
            return "";
    } else
        return text;
}


QGraphicsDropShadowEffect *getShadow(bool active)
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;

    if (active) {
        shadow->setBlurRadius(25.0);
        shadow->setColor(getColor(SETTINGS.value("Grid/activecolor","Cyan").toString(), 255));
        shadow->setOffset(0);
    } else {
        shadow->setBlurRadius(10.0);
        shadow->setColor(getColor(SETTINGS.value("Grid/inactivecolor","Black").toString(), 200));
        shadow->setOffset(0);
    }

    return shadow;
}


QByteArray getUrlContents(QUrl url)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager;

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", "CEN64-Qt");
    QNetworkReply *reply = manager->get(request);

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    return reply->readAll();
}


QStringList getZippedFiles(QString completeFileName)
{
    QuaZip zipFile(completeFileName);
    zipFile.open(QuaZip::mdUnzip);
    QStringList files = zipFile.getFileNameList();
    zipFile.close();

    return files;
}


QByteArray *getZippedRom(QString romFileName, QString zipFile)
{
    QuaZipFile zippedFile(zipFile, romFileName);

    zippedFile.open(QIODevice::ReadOnly);
    QByteArray *romData = new QByteArray();
    romData->append(zippedFile.readAll());
    zippedFile.close();

    return romData;
}


void initializeRom(Rom *currentRom, QDir romDir, bool cached, QWidget *parent)
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
            downloadGameInfo(currentRom->romMD5, currentRom->goodName, parent);
        } else {
            //tweak internal name by adding spaces to get better results
            QString search = currentRom->internalName;
            search.replace(QRegExp("([a-z])([A-Z])"),"\\1 \\2");
            search.replace(QRegExp("([^ \\d])(\\d)"),"\\1 \\2");
            downloadGameInfo(currentRom->romMD5, search, parent);
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

        QString imageFile = getDataLocation() + "/cache/"
                            + currentRom->romMD5.toLower() + "/boxart-front.jpg";
        QFile cover(imageFile);

        if (cover.exists()) {
            currentRom->image.load(imageFile);
            currentRom->imageExists = true;
        }
    }
}


bool romSorter(const Rom &firstRom, const Rom &lastRom)
{
    QString sort, direction;

    QString layout = SETTINGS.value("View/layout", "None").toString();
    if (layout == "Grid View") {
        sort = SETTINGS.value("Grid/sort", "Filename").toString();
        direction = SETTINGS.value("Grid/sortdirection", "ascending").toString();
    } else if (layout == "List View") {
        sort = SETTINGS.value("List/sort", "Filename").toString();
        direction = SETTINGS.value("List/sortdirection", "ascending").toString();
    } else //just return sort by filename
        return firstRom.fileName < lastRom.fileName;


    QString sortFirst = "", sortLast = "";

    if (sort == "Size") {
        int firstSize = firstRom.sortSize;
        int lastSize = lastRom.sortSize;

        if (direction == "descending")
            return firstSize > lastSize;
        else
            return firstSize < lastSize;
    } else if (sort == "Release Date") {
        sortFirst = firstRom.sortDate;
        sortLast = lastRom.sortDate;
    } else {
        sortFirst = getRomInfo(sort, &firstRom, true, true);
        sortLast = getRomInfo(sort, &lastRom, true, true);
    }

    if (sortFirst == sortLast) { //Equal so sort on filename
        sortFirst = firstRom.fileName;
        sortLast = lastRom.fileName;
    }

    if (direction == "descending")
        return sortFirst > sortLast;
    else
        return sortFirst < sortLast;
}
