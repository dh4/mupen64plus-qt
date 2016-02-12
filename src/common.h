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

#ifndef COMMON_H
#define COMMON_H

#include <QGraphicsDropShadowEffect>
#include <QString>
#include <QPixmap>

class QColor;
class QSize;


struct Rom {
    QString fileName;
    QString directory;
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
};

bool romSorter(const Rom &firstRom, const Rom &lastRom);
int getDefaultWidth(QString id, int imageWidth);
int getGridSize(QString which);

QByteArray byteswap(QByteArray romData);
QStringList getZippedFiles(QString completeFileName);
QByteArray *getZippedRom(QString romFileName, QString zipFile);
QColor getColor(QString color, int transparency = 255);
QString getTranslation(QString text);
QGraphicsDropShadowEffect *getShadow(bool active);
QSize getImageSize(QString view);
QString getDataLocation();
QString getRomInfo(QString identifier, const Rom *rom, bool removeWarn = false, bool sort = false);
QString getVersion();

#endif // COMMON_H
