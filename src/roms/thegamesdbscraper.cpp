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

#include "thegamesdbscraper.h"

#include "../global.h"
#include "../common.h"

#include <QDir>
#include <QEventLoop>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>
#include <QUrl>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtXml/QDomDocument>


TheGamesDBScraper::TheGamesDBScraper(QWidget *parent, bool force) : QObject(parent)
{
    this->parent = parent;
    this->force = force;
    this->keepGoing = true;
}


void TheGamesDBScraper::deleteGameInfo(QString fileName, QString identifier)
{
    QString text;
    text = QString(tr("<b>NOTE:</b> If you are deleting this game's information because the game doesn't "))
                 + tr("exist on TheGamesDB and <AppName> pulled the information for different game, it's ")
                 + tr("better to create an account on")+" <a href=\"http://thegamesdb.net/\">TheGamesDB</a> "
                 + tr("and add the game so other users can benefit as well.")
                 + "<br /><br />"
                 + tr("This will cause <AppName> to not update the information for this game until you ")
                 + tr("force it with \"Download/Update Info...\"")
                 + "<br /><br />"
                 + tr("Delete the current information for") + " <b>" + fileName + "</b>?";
    text.replace("<AppName>",AppName);

    int answer = QMessageBox::question(parent, tr("Delete Game Information"), text,
                                       QMessageBox::Yes | QMessageBox::No);

    if (answer == QMessageBox::Yes) {
        QString gameCache = getDataLocation() + "/cache/" + identifier.toLower();

        QString dataFile = gameCache + "/data.xml";
        QFile file(dataFile);

        // Remove game information
        file.open(QIODevice::WriteOnly);
        QTextStream stream(&file);
        stream << "NULL";
        file.close();

        // Remove cover image
        QString coverFile = gameCache + "/boxart-front.";

        QFile coverJPG(coverFile + "jpg");
        QFile coverPNG(coverFile + "png");

        if (coverJPG.exists())
            coverJPG.remove();
        if (coverPNG.exists())
            coverPNG.remove();

        coverJPG.open(QIODevice::WriteOnly);
        QTextStream streamImage(&coverJPG);
        streamImage << "";
        coverJPG.close();
    }
}


void TheGamesDBScraper::downloadGameInfo(QString identifier, QString searchName, QString gameID)
{
    if (keepGoing && identifier != "") {
        if (force) parent->setEnabled(false);

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
            else if (searchName == "Legend of Zelda, The - Ocarina of Time" ||
                     searchName == "THE LEGEND OF ZELDA")
                searchName = "Ocarina of Time";
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
                    if (date.text() != "") check += "\n" + tr("Released on: ") + date.text();
                    check += "\n\n" + tr("Does this look correct?");

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
        QString boxartExt = "";
        QString coverFile = gameCache + "/boxart-front.";

        QFile coverJPG(coverFile + "jpg");
        QFile coverPNG(coverFile + "png");

        if ((!coverJPG.exists() && !coverPNG.exists()) || (force && updated)) {
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

                //Check to save as JPG or PNG
                boxartExt = QFileInfo(boxartURL).completeSuffix().toLower();
                QFile cover(coverFile + boxartExt);

                cover.open(QIODevice::WriteOnly);
                cover.write(getUrlContents(url));
                cover.close();
            }
        }

        if (updated)
            QMessageBox::information(parent, QObject::tr("Game Information Download"),
                                     QObject::tr("Download Complete!"));

        if (force) parent->setEnabled(true);
    }
}


QByteArray TheGamesDBScraper::getUrlContents(QUrl url)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager;

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", AppName.toUtf8().constData());
    QNetworkReply *reply = manager->get(request);

    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.start(10000);
    loop.exec();

    if(timer.isActive()) { //Got reply
        timer.stop();

        if(reply->error() > 0)
            showError(reply->errorString());
        else
            return reply->readAll();

    } else //Request timed out
        showError(tr("Request timed out. Check your network settings."));

    return QByteArray();
}


void TheGamesDBScraper::showError(QString error)
{
    QString question = "\n\n" + tr("Continue scraping information?");

    if (force)
        QMessageBox::information(parent, tr("Network Error"), error);
    else {
        int answer = QMessageBox::question(parent, tr("Network Error"), error + question,
                                           QMessageBox::Yes | QMessageBox::No);

        if (answer == QMessageBox::No)
            keepGoing = false;
    }
}
