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

#ifndef COMMON_H
#define COMMON_H

#include <QColor>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QObject>
#include <QSize>
#include <QString>
#include <QTextStream>
#include <QUrl>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtXml/QDomDocument>

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include "global.h"


typedef struct {
    QString fileName;
    QString romMD5;
    QString internalName;
    QString zipFile;

    QString baseName;
    QString size;
    int sortSize;

    QString goodName;
    QString CRC1;
    QString CRC2;
    QString players;
    QString saveType;
    QString rumble;

    QString gameTitle;
    QString releaseDate;
    QString sortDate;
    QString overview;
    QString esrb;
    QString genre;
    QString publisher;
    QString developer;
    QString rating;

    QPixmap image;

    int count;
    bool imageExists;
} Rom;

void downloadGameInfo(QString identifier, QString searchName, QWidget *parent = 0, QString gameID = "",
                      bool force = false);
void initializeRom(Rom *currentRom, QDir romDir, bool cached, QWidget *parent = 0);

bool romSorter(const Rom &firstRom, const Rom &lastRom);
int getGridSize(QString which);

QByteArray byteswap(QByteArray romData);
QByteArray getUrlContents(QUrl url);
QStringList getZippedFiles(QString completeFileName);
QByteArray *getZippedRom(QString romFileName, QString zipFile);
QColor getColor(QString color, int transparency = 255);
QGraphicsDropShadowEffect *getShadow(bool active);
QSize getImageSize(QString view);
QString getDataLocation();
QString getRomInfo(QString identifier, const Rom *rom, bool removeWarn = false, bool sort = false);

#endif // COMMON_H
