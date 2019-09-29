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

#include "common.h"

#include "global.h"

#include <QColor>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QLocale>
#include <QSize>

#include <quazip5/quazip.h>
#include <quazip5/quazipfile.h>

#ifdef Q_OS_WIN
#include <QCoreApplication>
#else
#include <QDesktopServices>
#endif


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


QString getCacheLocation()
{
    return getDataLocation() + "/cache_v2/";
}


QString getDataLocation()
{
    QString dataDir;

#ifdef Q_OS_WIN
    dataDir = QCoreApplication::applicationDirPath();
#else
    dataDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation)
                    .replace(ParentName+"/"+AppName,AppNameLower);
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


QString getDefaultLanguage()
{
    QString systemLanguage = QLocale::system().name().left(2);

    //Add other languages here as translations are done
    if (systemLanguage == "fr")
        return "FR";
    else if (systemLanguage == "ru")
        return "RU";
    else
        return "EN";
}


int getDefaultWidth(QString id, int imageWidth)
{
    if (id == "Overview")
        return 400;
    else if (id == "GoodName" || id.left(8) == "Filename" || id == "Game Title")
        return 300;
    else if (id == "MD5")
        return 250;
    else if (id == "Internal Name" || id == "Publisher" || id == "Developer")
        return 200;
    else if (id == "ESRB" || id == "Genre")
        return 150;
    else if (id == "Save Type" || id == "Release Date")
        return 100;
    else if (id == "CRC1" || id == "CRC2")
        return 90;
    else if (id == "Size" || id == "Rumble" || id == "Players" || id == "Rating")
        return 75;
    else if (id == "Game Cover")
        return imageWidth;
    else
        return 100;
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
            if (size == "Super")       return 360;
        } else {
            if (size == "Extra Small") return 47;
            if (size == "Small")       return 71;
            if (size == "Medium")      return 122;
            if (size == "Large")       return 172;
            if (size == "Extra Large") return 224;
            if (size == "Super")       return 330;
        }
    } else if (which == "width") {
        if (size == "Extra Small") return 60;
        if (size == "Small")       return 90;
        if (size == "Medium")      return 160;
        if (size == "Large")       return 225;
        if (size == "Extra Large") return 300;
        if (size == "Super")       return 440;
    } else if (which == "font") {
        if (size == "Extra Small") return 5;
        if (size == "Small")       return 7;
        if (size == "Medium")      return 10;
        if (size == "Large")       return 12;
        if (size == "Extra Large") return 13;
        if (size == "Super")       return 15;
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
        if (size == "Super")       return QSize(210, 150);
    } else if (view == "Grid" || view == "List") {
        if (size == "Extra Small") return QSize(48, 35);
        if (size == "Small")       return QSize(69, 50);
        if (size == "Medium")      return QSize(138, 100);
        if (size == "Large")       return QSize(203, 150);
        if (size == "Extra Large") return QSize(276, 200);
        if (size == "Super")       return QSize(425, 300);
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
    else if (text == getTranslation("Unknown ROM") ||
             text == getTranslation("Requires catalog file") ||
             text == getTranslation("Not found")) {
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


int getTableDataIndexFromName(QString infoName)
{
    if (infoName == "fileName")
        return 0;
    else if (infoName == "dirName")
        return 1;
    else if (infoName == "search")
        return 2;
    else if (infoName == "romMD5")
        return 3;
    else if (infoName == "zipFile")
        return 4;

    return 0;
}


int getTextSize()
{
    QString size = SETTINGS.value("List/textsize","Medium").toString();

    if (size == "Extra Small") return 7;
    if (size == "Small")       return 9;
    if (size == "Medium")      return 10;
    if (size == "Large")       return 12;
    if (size == "Extra Large") return 14;
    return 10;
}


QString getTranslation(QString text)
{
    if (text == "GoodName")                     return QObject::tr("GoodName");
    else if (text == "Filename")                return QObject::tr("Filename");
    else if (text == "Filename (extension)")    return QObject::tr("Filename (extension)");
    else if (text == "Zip File")                return QObject::tr("Zip File");
    else if (text == "Internal Name")           return QObject::tr("Internal Name");
    else if (text == "Size")                    return QObject::tr("Size");
    else if (text == "MD5")                     return QObject::tr("MD5");
    else if (text == "CRC1")                    return QObject::tr("CRC1");
    else if (text == "CRC2")                    return QObject::tr("CRC2");
    else if (text == "Players")                 return QObject::tr("Players");
    else if (text == "Rumble")                  return QObject::tr("Rumble");
    else if (text == "Save Type")               return QObject::tr("Save Type");
    else if (text == "Game Title")              return QObject::tr("Game Title");
    else if (text == "Release Date")            return QObject::tr("Release Date");
    else if (text == "Overview")                return QObject::tr("Overview");
    else if (text == "ESRB")                    return QObject::tr("ESRB");
    else if (text == "Genre")                   return QObject::tr("Genre");
    else if (text == "Publisher")               return QObject::tr("Publisher");
    else if (text == "Developer")               return QObject::tr("Developer");
    else if (text == "Rating")                  return QObject::tr("Rating");
    else if (text == "Game Cover")              return QObject::tr("Game Cover");
    else if (text == "Unknown ROM")             return QObject::tr("Unknown ROM");
    else if (text == "Requires catalog file")   return QObject::tr("Requires catalog file");
    else if (text == "Not found")               return QObject::tr("Not found");

    return text;
}


QString getVersion()
{
    QFile versionFile(":/other/VERSION");
    versionFile.open(QIODevice::ReadOnly);
    QString version = versionFile.readAll();
    versionFile.close();

    return version;
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


bool romSorter(const Rom &firstRom, const Rom &lastRom)
{
    QString sort, direction;

    QString layout = SETTINGS.value("View/layout", "None").toString();
    if (layout == "grid") {
        sort = SETTINGS.value("Grid/sort", "Filename").toString();
        direction = SETTINGS.value("Grid/sortdirection", "ascending").toString();
    } else if (layout == "list") {
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
