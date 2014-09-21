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

#include "mupen64plusqt.h"
#include "aboutdialog.h"
#include "configeditor.h"
#include "global.h"
#include "settingsdialog.h"


Mupen64PlusQt::Mupen64PlusQt(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("Mupen64Plus-Qt"));
    setWindowIcon(QIcon(":/images/mupen64plus.png"));

    autoloadSettings();

    romPath = SETTINGS.value("Paths/roms","").toString();
    romDir = QDir(romPath);

    widget = new QWidget(this);
    setCentralWidget(widget);
    setGeometry(QRect(SETTINGS.value("Geometry/windowx", 0).toInt(),
                      SETTINGS.value("Geometry/windowy", 0).toInt(),
                      SETTINGS.value("Geometry/width", 900).toInt(),
                      SETTINGS.value("Geometry/height", 600).toInt()));

    createMenu();
    createRomView();

    layout = new QVBoxLayout(widget);
    layout->setMenuBar(menuBar);

    layout->addWidget(emptyView);
    layout->addWidget(romTree);
    layout->addWidget(gridView);
    layout->addWidget(listView);

    layout->setMargin(0);

    widget->setLayout(layout);
    widget->setMinimumSize(300, 200);
}


void Mupen64PlusQt::addRoms()
{
    QList<Rom> roms;

    SETTINGS.setValue("ROMs/cache", "");
    QStringList tableVisible = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");
    resetLayouts(tableVisible);

    romTree->setEnabled(false);
    gridView->setEnabled(false);
    listView->setEnabled(false);
    romTree->clear();
    downloadAction->setEnabled(false);
    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    if (romPath != "") {
        if (romDir.exists()) {
            QStringList files = romDir.entryList(QStringList() << "*.z64" << "*.n64" << "*.v64",
                                                 QDir::Files | QDir::NoSymLinks);

            if (files.size() > 0) {
                setupProgressDialog(files);

                int count = 0;
                foreach (QString fileName, files)
                {
                    Rom currentRom;

                    QFile file(romDir.absoluteFilePath(fileName));

                    file.open(QIODevice::ReadOnly);
                    romData = new QByteArray(file.readAll());
                    file.close();

                    *romData = byteswap(*romData);

                    currentRom.fileName = fileName;
                    currentRom.internalName = QString(romData->mid(32, 20)).trimmed();
                    currentRom.romMD5 = QString(QCryptographicHash::hash(*romData,
                                                QCryptographicHash::Md5).toHex());

                    delete romData;

                    initializeRom(&currentRom, false);
                    roms.append(currentRom);

                    QString currentSetting = SETTINGS.value("ROMs/cache", "").toString();
                    QString newSetting = currentSetting
                                         + currentRom.fileName + "|"
                                         + currentRom.internalName + "|"
                                         + currentRom.romMD5 + "||";

                    SETTINGS.setValue("ROMs/cache", newSetting);

                    count++;
                    progress->setValue(count);
                }

                progress->close();
            } else {
            QMessageBox::warning(this, "Warning", "No ROMs found.");
            }
        } else {
            QMessageBox::warning(this, "Warning", "Failed to open ROM directory.");
        }
    }

    qSort(roms.begin(), roms.end(), romSorter);

    int i = 0;
    foreach (Rom currentRom, roms)
    {
        if (SETTINGS.value("View/layout","Table View") == "Table View")
            addToTableView(&currentRom);
        else if (SETTINGS.value("View/layout","Table View") == "Grid View")
            addToGridView(&currentRom, i);
        else if (SETTINGS.value("View/layout","Table View") == "List View")
            addToListView(&currentRom, i);

        i++;
    }

    if (roms.size() != 0) {
        if (tableVisible.join("") != "")
            romTree->setEnabled(true);

        gridView->setEnabled(true);
        listView->setEnabled(true);
    }
}


void Mupen64PlusQt::addToGridView(Rom *currentRom, int count)
{
    ClickableWidget *gameGridItem = new ClickableWidget(gridWidget);
    gameGridItem->setMinimumHeight(getGridSize("height"));
    gameGridItem->setMaximumHeight(getGridSize("height"));
    gameGridItem->setMinimumWidth(getGridSize("width"));
    gameGridItem->setMaximumWidth(getGridSize("width"));
    gameGridItem->setGraphicsEffect(getShadow(false));

    //Assign ROM data to widget for use in click events
    gameGridItem->setProperty("fileName",currentRom->fileName);
    if (currentRom->goodName == "Unknown ROM" || currentRom->goodName == "Requires catalog file")
        gameGridItem->setProperty("search",currentRom->internalName);
    else
        gameGridItem->setProperty("search",currentRom->goodName);
    gameGridItem->setProperty("romMD5",currentRom->romMD5);

    QGridLayout *gameGridLayout = new QGridLayout(gameGridItem);
    gameGridLayout->setColumnStretch(0, 1);
    gameGridLayout->setColumnStretch(3, 1);
    gameGridLayout->setRowMinimumHeight(1, getImageSize("Grid").height());

    QLabel *gridImageLabel = new QLabel(gameGridItem);
    gridImageLabel->setMinimumHeight(getImageSize("Grid").height());
    gridImageLabel->setMinimumWidth(getImageSize("Grid").width());
    QPixmap image;

    if (currentRom->imageExists)
        image = currentRom->image.scaled(getImageSize("Grid"), Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation);
    else
        image = QPixmap(":/images/not-found.png").scaled(getImageSize("Grid"), Qt::IgnoreAspectRatio,
                                                         Qt::SmoothTransformation);

    gridImageLabel->setPixmap(image);
    gameGridLayout->addWidget(gridImageLabel, 1, 1);

    if (SETTINGS.value("Grid/label","true") == "true") {
        QLabel *gridTextLabel = new QLabel(gameGridItem);

        //Don't allow label to be wider than image
        gridTextLabel->setMaximumWidth(getImageSize("Grid").width());

        QString text = "";
        QString labelText = SETTINGS.value("Grid/labeltext","Filename").toString();

        if (labelText == "Filename")
            text = currentRom->baseName;
        else if (labelText == "Filename (extension)")
            text = currentRom->fileName;
        else if (labelText == "GoodName")
            text = currentRom->goodName;
        else if (labelText == "Internal Name")
            text = currentRom->internalName;
        else if (labelText == "Game Title")
            text = currentRom->gameTitle;
        else if (labelText == "Release Date")
            text = currentRom->releaseDate;
        else if (labelText == "Genre")
            text = currentRom->genre;

        gridTextLabel->setText(text);

        QString textHex = getColor(SETTINGS.value("Grid/labelcolor","White").toString()).name();
        int fontSize = getGridSize("font");

#ifdef Q_OS_OSX //OSX is funky with the label text
        if (text.length() > 30)
            fontSize -= 2;
#endif

        gridTextLabel->setStyleSheet("QLabel { font-weight: bold; color: " + textHex + "; font-size: "
                                     + QString::number(fontSize) + "px; }");
        gridTextLabel->setWordWrap(true);
        gridTextLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

        gameGridLayout->addWidget(gridTextLabel, 2, 1);
    }

    gameGridItem->setLayout(gameGridLayout);

    int columnCount = SETTINGS.value("Grid/columncount", "4").toInt();
    gridLayout->addWidget(gameGridItem, count / columnCount + 1, count % columnCount + 1);
    gridWidget->adjustSize();

    connect(gameGridItem, SIGNAL(singleClicked(QWidget*)), this, SLOT(highlightGridWidget(QWidget*)));
    connect(gameGridItem, SIGNAL(doubleClicked(QWidget*)), this, SLOT(runEmulatorFromWidget(QWidget*)));
}


void Mupen64PlusQt::addToListView(Rom *currentRom, int count)
{
    QStringList visible = SETTINGS.value("List/columns", "Filename|Internal Name|Size").toString().split("|");

    if (visible.join("") == "" && SETTINGS.value("List/displaycover", "true") != "true")
        //Otherwise no columns, so don't bother populating
        return;

    ClickableWidget *gameListItem = new ClickableWidget(listWidget);
    gameListItem->setContentsMargins(0, 0, 20, 0);

    //Assign ROM data to widget for use in click events
    gameListItem->setProperty("fileName",currentRom->fileName);
    if (currentRom->goodName == "Unknown ROM" || currentRom->goodName == "Requires catalog file")
        gameListItem->setProperty("search",currentRom->internalName);
    else
        gameListItem->setProperty("search",currentRom->goodName);
    gameListItem->setProperty("romMD5",currentRom->romMD5);

    QGridLayout *gameListLayout = new QGridLayout(gameListItem);
    gameListLayout->setColumnStretch(3, 1);

    //Add image
    if (SETTINGS.value("List/displaycover", "") == "true") {
        QLabel *listImageLabel = new QLabel(gameListItem);

        QPixmap image;

        if (currentRom->imageExists)
            image = currentRom->image.scaled(getImageSize("List"), Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation);
        else
            image = QPixmap(":/images/not-found.png").scaled(getImageSize("List"), Qt::KeepAspectRatio,
                                                             Qt::SmoothTransformation);

        listImageLabel->setPixmap(image);

        gameListLayout->addWidget(listImageLabel, 0, 1);
    }

    //Create text label
    QLabel *listTextLabel = new QLabel("", gameListItem);
    QString listText = "";

    int i = 0;

    foreach (QString current, visible)
    {
        QString addition = "<style>h2 { margin: 0; }</style>";

        if (i == 0 && SETTINGS.value("List/firstitemheader","true") == "true")
            addition += "<h2>";
        else
            addition += "<b>" + current + ":</b> ";

        if (current == "GoodName") {
            if (currentRom->goodName != "Unknown ROM" && currentRom->goodName != "Requires catalog file")
                addition += currentRom->goodName + "<br />";
        }
        else if (current == "Filename") {
            addition += currentRom->baseName + "<br />";
        }
        else if (current == "Filename (extension)") {
            addition += currentRom->fileName + "<br />";
        }
        else if (current == "Internal Name") {
            addition += currentRom->internalName + "<br />";
        }
        else if (current == "Size") {
            addition += currentRom->size + "<br />";
        }
        else if (current == "MD5") {
            addition += currentRom->romMD5.toLower() + "<br />";
        }
        else if (current == "CRC1") {
            addition += currentRom->CRC1.toLower() + "<br />";
        }
        else if (current == "CRC2") {
            addition += currentRom->CRC2.toLower() + "<br />";
        }
        else if (current == "Players") {
            addition += currentRom->players + "<br />";
        }
        else if (current == "Rumble") {
            addition += currentRom->rumble + "<br />";
        }
        else if (current == "Save Type") {
            addition += currentRom->saveType + "<br />";
        }
        else if (current == "Game Title") {
            if (currentRom->gameTitle != "Not found")
                addition += currentRom->gameTitle + "<br />";
        }
        else if (current == "Release Date") {
            addition += currentRom->releaseDate + "<br />";
        }
        else if (current == "Overview") {
            addition += currentRom->overview + "<br />";
        }
        else if (current == "ESRB") {
            addition += currentRom->esrb + "<br />";
        }
        else if (current == "Genre") {
            addition += currentRom->genre + "<br />";
        }
        else if (current == "Publisher") {
            addition += currentRom->publisher + "<br />";
        }
        else if (current == "Developer") {
            addition += currentRom->developer + "<br />";
        }
        else if (current == "Rating") {
            addition += currentRom->rating + "<br />";
        }
        else //Invalid column name in config file
            addition += "<br />";

        if (i == 0 && SETTINGS.value("List/firstitemheader","true") == "true")
            addition += "</h2>";

        if (addition != "<b>" + current + ":</b> <br />" && addition != "<b>" + current + ":</b> ")
            listText += addition;

        i++;
    }

    //Remove last break tag
    listText.remove(QRegExp("<br />$"));

    listTextLabel->setText(listText);
    listTextLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    listTextLabel->setWordWrap(true);
    gameListLayout->addWidget(listTextLabel, 0, 3);

    gameListLayout->setColumnMinimumWidth(0, 20);
    gameListLayout->setColumnMinimumWidth(2, 10);
    gameListItem->setLayout(gameListLayout);

    if (count != 0) {
        QFrame *separator = new QFrame();
        separator->setFrameShape(QFrame::HLine);
        separator->setStyleSheet("margin:0;padding:0;");
        listLayout->addWidget(separator);
    }

    listLayout->addWidget(gameListItem);

    connect(gameListItem, SIGNAL(singleClicked(QWidget*)), this, SLOT(highlightListWidget(QWidget*)));
    connect(gameListItem, SIGNAL(doubleClicked(QWidget*)), this, SLOT(runEmulatorFromWidget(QWidget*)));
}


void Mupen64PlusQt::addToTableView(Rom *currentRom)
{
    QStringList visible = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");

    if (visible.join("") == "") //Otherwise no columns, so don't bother populating
        return;

    fileItem = new TreeWidgetItem(romTree);

    //Filename for launching ROM
    fileItem->setText(0, currentRom->fileName);

    //GoodName or Internal Name for searching
    if (currentRom->goodName == "Unknown ROM" || currentRom->goodName == "Requires catalog file")
        fileItem->setText(1, currentRom->internalName);
    else
        fileItem->setText(1, currentRom->goodName);

    //MD5 for cache info
    fileItem->setText(2, currentRom->romMD5.toLower());

    int i = 3, c = 0;
    bool addImage = false;

    foreach (QString current, visible)
    {
        if (current == "GoodName") {
            fileItem->setText(i, currentRom->goodName);
            if (currentRom->goodName == "Unknown ROM" || currentRom->goodName == "Requires catalog file") {
                fileItem->setForeground(i, QBrush(Qt::gray));
                fileItem->setData(i, Qt::UserRole, "ZZZ"); //end of sorting
            } else
                fileItem->setData(i, Qt::UserRole, currentRom->goodName);
        }
        else if (current == "Filename") {
            fileItem->setText(i, currentRom->baseName);
        }
        else if (current == "Filename (extension)") {
            fileItem->setText(i, currentRom->fileName);
        }
        else if (current == "Internal Name") {
            fileItem->setText(i, currentRom->internalName);
        }
        else if (current == "Size") {
            fileItem->setText(i, currentRom->size);
            fileItem->setData(i, Qt::UserRole, currentRom->sortSize);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (current == "MD5") {
            fileItem->setText(i, currentRom->romMD5.toLower());
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else if (current == "CRC1") {
            fileItem->setText(i, currentRom->CRC1.toLower());
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else if (current == "CRC2") {
            fileItem->setText(i, currentRom->CRC2.toLower());
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else if (current == "Players") {
            fileItem->setText(i, currentRom->players);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (current == "Rumble") {
            fileItem->setText(i, currentRom->rumble);
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else if (current == "Save Type") {
            fileItem->setText(i, currentRom->saveType);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (current == "Game Title") {
            fileItem->setText(i, currentRom->gameTitle);
            if (currentRom->gameTitle == "Not found") {
                fileItem->setForeground(i, QBrush(Qt::gray));
                fileItem->setData(i, Qt::UserRole, "ZZZ"); //end of sorting
            } else
                fileItem->setData(i, Qt::UserRole, currentRom->gameTitle);
        }
        else if (current == "Release Date") {
            fileItem->setText(i, currentRom->releaseDate);
            fileItem->setData(i, Qt::UserRole, currentRom->sortDate);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (current == "Overview") {
            fileItem->setText(i, currentRom->overview);
        }
        else if (current == "ESRB") {
            fileItem->setText(i, currentRom->esrb);
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else if (current == "Genre") {
            fileItem->setText(i, currentRom->genre);
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else if (current == "Publisher") {
            fileItem->setText(i, currentRom->publisher);
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else if (current == "Developer") {
            fileItem->setText(i, currentRom->developer);
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else if (current == "Rating") {
            fileItem->setText(i, currentRom->rating);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (current == "Game Cover") {
            //fileItem->setIcon(i, QIcon(image));
            fileItem->setText(i, "");
            c = i;
            addImage = true;
        }
        else //Invalid column name in config file
            fileItem->setText(i, "");

        i++;
    }

    romTree->addTopLevelItem(fileItem);


    if (currentRom->imageExists && addImage) {
        QPixmap image(currentRom->image.scaled(getImageSize("Table"), Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation));

        QWidget *imageContainer = new QWidget(romTree);
        QGridLayout *imageGrid = new QGridLayout(imageContainer);
        QLabel *imageLabel = new QLabel(imageContainer);

        imageLabel->setPixmap(image);
        imageGrid->addWidget(imageLabel, 1, 1);
        imageGrid->setColumnStretch(0, 1);
        imageGrid->setColumnStretch(2, 1);
        imageGrid->setRowStretch(0, 1);
        imageGrid->setRowStretch(2, 1);
        imageGrid->setContentsMargins(0,0,0,0);

        imageContainer->setLayout(imageGrid);

        romTree->setItemWidget(fileItem, c, imageContainer);
    }
}


void Mupen64PlusQt::autoloadSettings()
{
    QString mupen64Path = SETTINGS.value("Paths/mupen64plus", "").toString();
    QString dataPath = SETTINGS.value("Paths/data", "").toString();
    QString pluginPath = SETTINGS.value("Paths/plugins", "").toString();

    if (mupen64Path == "" && dataPath == "" && pluginPath == "") {
#ifdef Q_OS_LINUX
        //If user has not entered any settings, check common locations for them
        QStringList mupen64Check, dataCheck, pluginCheck;

        mupen64Check << "/usr/games/mupen64plus"
                     << "/usr/bin/mupen64plus";

        pluginCheck  << "/usr/lib/mupen64plus"
                     << "/usr/lib/x86_64-linux-gnu/mupen64plus"
                     << "/usr/lib/i386-linux-gnu/mupen64plus"
                     << "/usr/lib/mupen64plus/mupen64plus";

        dataCheck    << "/usr/share/mupen64plus"
                     << "/usr/share/games/mupen64plus";


        foreach (QString check, mupen64Check)
            if (QFileInfo(check).exists())
                SETTINGS.setValue("Paths/mupen64plus", check);

        foreach (QString check, pluginCheck)
            if (QFileInfo(check+"/mupen64plus-video-rice.so").exists())
                SETTINGS.setValue("Paths/plugins", check);

        foreach (QString check, dataCheck)
            if (QFileInfo(check+"/mupen64plus.ini").exists())
                SETTINGS.setValue("Paths/data", check);
#endif

#ifdef Q_OS_WIN
        //Check for Mupen64Plus within the same directory
        QString currentDir = QDir::currentPath();

        if (QFileInfo(currentDir+"/mupen64plus.exe").exists())
            SETTINGS.setValue("Paths/mupen64plus", currentDir+"/mupen64plus.exe");

        if (QFileInfo(currentDir+"/mupen64plus-video-rice.dll").exists())
            SETTINGS.setValue("Paths/plugins", currentDir);

        if (QFileInfo(currentDir+"/mupen64plus.ini").exists())
            SETTINGS.setValue("Paths/data", currentDir);
#endif

#ifdef Q_OS_OSX
        //Check for Mupen64Plus App within the same directory
        QString currentDir = QDir::currentPath();

        QString mupen64App = currentDir+"/mupen64plus.app/Contents";
        if (QFileInfo(mupen64App+"/MacOS/mupen64plus").exists()) {
            SETTINGS.setValue("Paths/mupen64plus", mupen64App+"/MacOS/mupen64plus");
            SETTINGS.setValue("Paths/plugins", mupen64App+"/MacOS");
            SETTINGS.setValue("Paths/data", mupen64App+"/Resources");
        }
#endif
    }
}


QByteArray Mupen64PlusQt::byteswap(QByteArray romData)
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


void Mupen64PlusQt::cacheGameInfo(QString identifier, QString searchName, QString gameID, bool force)
{
    if (identifier != "") {
        bool updated = false;

        QString gameCache = getCacheLocation() + "/" + identifier;
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

                    int answer = QMessageBox::question(this, tr("Game Information Download"),
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
                    message = tr("No results found.");
                else
                    message = tr("No more results found.");

                QMessageBox::information(this, tr("Game Information Download"), message);
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

        if (updated) {
            QMessageBox::information(this, tr("Game Information Download"), tr("Download Complete!"));
            cachedRoms();
        }
    }
}


void Mupen64PlusQt::cachedRoms(bool imageUpdated)
{
    QList<Rom> roms;

    romTree->setEnabled(false);
    gridView->setEnabled(false);
    listView->setEnabled(false);
    romTree->clear();
    downloadAction->setEnabled(false);
    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    QStringList tableVisible = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");
    resetLayouts(tableVisible, imageUpdated);

    QString cache = SETTINGS.value("ROMs/cache", "").toString();
    QStringList cachedRoms = cache.split("||");

    int count = 0;
    bool showProgress = false;
    QTime checkPerformance;

    foreach (QString current, cachedRoms)
    {
        QStringList romInfo = current.split("|");

        if (romInfo.size() == 3) {
            Rom currentRom;

            currentRom.fileName = romInfo[0];
            currentRom.romMD5 = romInfo[2];
            currentRom.internalName = romInfo[1];

            //Check performance of adding first item to see if progress dialog needs to be shown
            if (count == 0) checkPerformance.start();

            initializeRom(&currentRom, true);
            roms.append(currentRom);

            if (count == 0) {
                int runtime = checkPerformance.elapsed();

                //check if operation expected to take longer than two seconds
                if (runtime * cachedRoms.size() > 2000) {
                    setupProgressDialog(cachedRoms);
                    showProgress = true;
                }
            }

            count++;

            if (showProgress)
                progress->setValue(count);
        }
    }

    if (showProgress)
        progress->close();

    qSort(roms.begin(), roms.end(), romSorter);

    int i = 0;
    foreach (Rom currentRom, roms)
    {
        if (SETTINGS.value("View/layout","Table View") == "Table View")
            addToTableView(&currentRom);
        else if (SETTINGS.value("View/layout","Table View") == "Grid View")
            addToGridView(&currentRom, i);
        else if (SETTINGS.value("View/layout","Table View") == "List View")
            addToListView(&currentRom, i);

        i++;
    }

    if (roms.size() != 0) {
        if (tableVisible.join("") != "")
            romTree->setEnabled(true);

        gridView->setEnabled(true);
        listView->setEnabled(true);
    }
}

void Mupen64PlusQt::checkStatus(int status)
{
    if (status > 0)
        QMessageBox::warning(this, "Warning",
            "Mupen64Plus quit unexpectedly. Check to make sure you are using a valid ROM.");
}


void Mupen64PlusQt::closeEvent(QCloseEvent *event)
{
    SETTINGS.setValue("Geometry/windowx", geometry().x());
    SETTINGS.setValue("Geometry/windowy", geometry().y());
    SETTINGS.setValue("Geometry/width", geometry().width());
    SETTINGS.setValue("Geometry/height", geometry().height());
    if (isMaximized())
        SETTINGS.setValue("Geometry/maximized", true);
    else
        SETTINGS.setValue("Geometry/maximized", "");

    saveColumnWidths();

    event->accept();
}


void Mupen64PlusQt::createMenu()
{
    menuBar = new QMenuBar(this);


    fileMenu = new QMenu(tr("&File"), this);
    openAction = fileMenu->addAction(tr("&Open ROM..."));
    fileMenu->addSeparator();
    refreshAction = fileMenu->addAction(tr("&Refresh List"));
    downloadAction = fileMenu->addAction(tr("&Download/Update Info..."));
#ifndef Q_OS_OSX //OSX does not show the quit action so the separator is unneeded
    fileMenu->addSeparator();
#endif
    quitAction = fileMenu->addAction(tr("&Quit"));

    openAction->setIcon(QIcon::fromTheme("document-open"));
    refreshAction->setIcon(QIcon::fromTheme("view-refresh"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));

    downloadAction->setEnabled(false);

    menuBar->addMenu(fileMenu);


    emulationMenu = new QMenu(tr("&Emulation"), this);
    startAction = emulationMenu->addAction(tr("&Start"));
    stopAction = emulationMenu->addAction(tr("St&op"));
    emulationMenu->addSeparator();
    logAction = emulationMenu->addAction(tr("View Log..."));

    startAction->setIcon(QIcon::fromTheme("media-playback-start"));
    stopAction->setIcon(QIcon::fromTheme("media-playback-stop"));

    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    menuBar->addMenu(emulationMenu);


    settingsMenu = new QMenu(tr("&Settings"), this);
    layoutMenu = settingsMenu->addMenu(tr("&Layout"));
    layoutGroup = new QActionGroup(this);

    QStringList layouts;
    layouts << "None" << "Table View" << "Grid View" << "List View";

    QString layoutValue = SETTINGS.value("View/layout","Table View").toString();

    foreach (QString layoutName, layouts)
    {
        QAction *layoutItem = layoutMenu->addAction(layoutName);
        layoutItem->setData(layoutName);
        layoutItem->setCheckable(true);
        layoutGroup->addAction(layoutItem);

        //Only enable layout changes when Mupen64Plus is not running
        menuEnable << layoutItem;

        if(layoutValue == layoutName)
            layoutItem->setChecked(true);
    }

    editorAction = settingsMenu->addAction(tr("Edit mupen64plus.cfg..."));
#ifndef Q_OS_OSX //OSX does not show the configure action so the separator is unneeded
    settingsMenu->addSeparator();
#endif
    configureAction = settingsMenu->addAction(tr("&Configure..."));
    configureAction->setIcon(QIcon::fromTheme("preferences-other"));

    menuBar->addMenu(settingsMenu);


    helpMenu = new QMenu(tr("&Help"), this);
    aboutAction = helpMenu->addAction(tr("&About"));
    aboutAction->setIcon(QIcon::fromTheme("help-about"));
    menuBar->addMenu(helpMenu);


    //Create list of actions that are enabled only when Mupen64Plus is not running
    menuEnable << startAction
               << logAction
               << openAction
               << refreshAction
               << downloadAction
               << configureAction
               << editorAction
               << quitAction;

    //Create list of actions that are disabled when Mupen64Plus is not running
    menuDisable << stopAction;

    connect(openAction, SIGNAL(triggered()), this, SLOT(openRom()));
    connect(refreshAction, SIGNAL(triggered()), this, SLOT(addRoms()));
    connect(downloadAction, SIGNAL(triggered()), this, SLOT(openDownloader()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(startAction, SIGNAL(triggered()), this, SLOT(runEmulatorFromMenu()));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stopEmulator()));
    connect(logAction, SIGNAL(triggered()), this, SLOT(openLog()));
    connect(configureAction, SIGNAL(triggered()), this, SLOT(openOptions()));
    connect(editorAction, SIGNAL(triggered()), this, SLOT(openEditor()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(openAbout()));
    connect(layoutGroup, SIGNAL(triggered(QAction*)), this, SLOT(updateLayoutSetting()));
}


void Mupen64PlusQt::createRomView()
{
    //Create empty view
    emptyView = new QScrollArea(this);
    emptyView->setStyleSheet("QScrollArea { border: none; }");
    emptyView->setBackgroundRole(QPalette::Base);
    emptyView->setAutoFillBackground(true);
    emptyView->setHidden(true);

    emptyLayout = new QGridLayout(emptyView);

    icon = new QLabel(emptyView);
    icon->setPixmap(QPixmap(":/images/mupen64plus.png"));

    emptyLayout->addWidget(icon, 1, 1);
    emptyLayout->setColumnStretch(0, 1);
    emptyLayout->setColumnStretch(2, 1);
    emptyLayout->setRowStretch(0, 1);
    emptyLayout->setRowStretch(2, 1);

    emptyView->setLayout(emptyLayout);


    //Create table view
    romTree = new QTreeWidget(this);
    romTree->setWordWrap(false);
    romTree->setAllColumnsShowFocus(true);
    romTree->setRootIsDecorated(false);
    romTree->setSortingEnabled(true);
    romTree->setStyleSheet("QTreeView { border: none; } QTreeView::item { height: 25px; }");

    headerView = new QHeaderView(Qt::Horizontal, this);
    romTree->setHeader(headerView);
    romTree->setHidden(true);


    //Create grid view
    gridView = new QScrollArea(this);
    gridView->setObjectName("gridView");
    gridView->setStyleSheet("#gridView { border: none; }");
    gridView->setBackgroundRole(QPalette::Dark);
    gridView->setAlignment(Qt::AlignHCenter);
    gridView->setHidden(true);

    gridView->verticalScrollBar()->setObjectName("vScrollBar");
    gridView->horizontalScrollBar()->setObjectName("hScrollBar");

    setGridBackground();


    gridWidget = new QWidget(gridView);
    gridWidget->setObjectName("gridWidget");
    gridView->setWidget(gridWidget);

    gridLayout = new QGridLayout(gridWidget);
    gridLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    gridLayout->setRowMinimumHeight(0, 10);

    gridWidget->setLayout(gridLayout);

    gridCurrent = false;
    currentGridRom = 0;


    //Create list view
    listView = new QScrollArea(this);
    listView->setStyleSheet("QScrollArea { border: none; }");
    listView->setBackgroundRole(QPalette::Base);
    listView->setWidgetResizable(true);
    listView->setHidden(true);

    listWidget = new QWidget(listView);
    listView->setWidget(listWidget);

    listLayout = new QVBoxLayout(listWidget);
    listLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    listWidget->setLayout(listLayout);

    listCurrent = false;
    currentListRom = 0;


    QString visibleLayout = SETTINGS.value("View/layout", "Table View").toString();

    if (visibleLayout == "None")
        emptyView->setHidden(false);
    else if (visibleLayout == "Table View")
        romTree->setHidden(false);
    else if (visibleLayout == "Grid View")
        gridView->setHidden(false);
    else if (visibleLayout == "List View")
        listView->setHidden(false);

    cachedRoms();

    connect(romTree, SIGNAL(clicked(QModelIndex)), this, SLOT(enableButtons()));
    connect(romTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(runEmulatorFromRomTree()));
    connect(headerView, SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(saveSortOrder(int,Qt::SortOrder)));
}


void Mupen64PlusQt::enableButtons()
{
    toggleMenus(true);
}


QString Mupen64PlusQt::getCacheLocation()
{
#ifdef Q_OS_WIN
    return QDir::currentPath() + "/cache";
#else

#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation)
                    .replace("Mupen64Plus/Mupen64Plus-Qt","mupen64plus-qt/cache");
#else
    return QDesktopServices::storageLocation(QDesktopServices::DataLocation)
                    .replace("Mupen64Plus/Mupen64Plus-Qt","mupen64plus-qt/cache");
#endif

#endif

}


QString Mupen64PlusQt::getCurrentRomInfo(int index)
{
    if (index < 3) {
        const char *infoChar;

        switch (index) {
            case 0:  infoChar = "fileName"; break;
            case 1:  infoChar = "search";   break;
            case 2:  infoChar = "romMD5";   break;
            default: infoChar = "";         break;
        }

        QString visibleLayout = SETTINGS.value("View/layout", "Table View").toString();

        if (visibleLayout == "None")
            return "";
        else if (visibleLayout == "Table View")
            return romTree->currentItem()->data(index, 0).toString();
        else if (visibleLayout == "Grid View" && gridCurrent)
            return gridLayout->itemAt(currentGridRom)->widget()->property(infoChar).toString();
        else if (visibleLayout == "List View" && listCurrent)
            return listLayout->itemAt(currentListRom)->widget()->property(infoChar).toString();
    }

    return "";
}


QColor Mupen64PlusQt::getColor(QString color, int transparency)
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


int Mupen64PlusQt::getGridSize(QString which)
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


QSize Mupen64PlusQt::getImageSize(QString view)
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


QGraphicsDropShadowEffect *Mupen64PlusQt::getShadow(bool active)
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


QByteArray Mupen64PlusQt::getUrlContents(QUrl url)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", "Mupen64Plus-Qt");
    QNetworkReply *reply = manager->get(request);

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    return reply->readAll();
}


void Mupen64PlusQt::highlightGridWidget(QWidget *current)
{
    //Set all to inactive shadow
    QLayoutItem *gridItem;
    for (int item = 0; (gridItem = gridLayout->itemAt(item)) != NULL; item++)
    {
        gridItem->widget()->setGraphicsEffect(getShadow(false));

        if (gridItem->widget() == current)
            currentGridRom = item;
    }

    //Set current to active shadow
    current->setGraphicsEffect(getShadow(true));

    gridCurrent = true;
    toggleMenus(true);
}


void Mupen64PlusQt::highlightListWidget(QWidget *current)
{
    //Reset all margins
    QLayoutItem *listItem;
    for (int item = 0; (listItem = listLayout->itemAt(item)) != NULL; item++)
    {
        listItem->widget()->setContentsMargins(0, 0, 20, 0);

        if (listItem->widget() == current)
            currentListRom = item;
    }

    //Give current left margin to stand out
    current->setContentsMargins(20, 0, 0, 0);

    listCurrent = true;
    toggleMenus(true);
}


void Mupen64PlusQt::initializeRom(Rom *currentRom, bool cached)
{
    QString dataPath = SETTINGS.value("Paths/data", "").toString();
    dataDir = QDir(dataPath);
    QString catalogFile = dataDir.absoluteFilePath("mupen64plus.ini");

    //Default text for GoodName to notify user
    currentRom->goodName = "Requires catalog file";
    currentRom->imageExists = false;

    bool getGoodName = false;
    if (QFileInfo(catalogFile).exists()) {
        romCatalog = new QSettings(catalogFile, QSettings::IniFormat, this);
        getGoodName = true;
    }

    currentRom->romMD5 = currentRom->romMD5.toUpper();

    QFile file(romDir.absoluteFilePath(currentRom->fileName));
    qint64 size = QFileInfo(file).size();

    currentRom->baseName = QFileInfo(file).completeBaseName();
    currentRom->size = tr("%1 MB").arg(int((size + 1023) / 1024 / 1024));
    currentRom->sortSize = (int)size;

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
            cacheGameInfo(currentRom->romMD5.toLower(), currentRom->goodName);
        } else {
            //tweak internal name by adding spaces to get better results
            QString search = currentRom->internalName;
            search.replace(QRegExp("([a-z])([A-Z])"),"\\1 \\2");
            search.replace(QRegExp("([^ ])(\\d)"),"\\1 \\2");
            cacheGameInfo(currentRom->romMD5.toLower(), search);
        }

    }

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true") {
        QString cacheDir = getCacheLocation();

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

        QString imageFile = getCacheLocation() + "/" + currentRom->romMD5.toLower() + "/boxart-front.jpg";
        QFile cover(imageFile);

        if (cover.exists()) {
            currentRom->image.load(imageFile);
            currentRom->imageExists = true;
        }
    }
}


void Mupen64PlusQt::openAbout()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}


void Mupen64PlusQt::openDownloader()
{
    QString fileText = getCurrentRomInfo(0);
    QString defaultText = getCurrentRomInfo(1);

    downloadDialog = new QDialog(this);
    downloadDialog->setWindowTitle(tr("Search Game Information"));

    downloadLayout = new QGridLayout(downloadDialog);

    fileLabel = new QLabel(tr("<b>File:</b> ") + fileText, downloadDialog);

    gameNameLabel = new QLabel(tr("Name of Game:"), downloadDialog);
    gameIDLabel = new QLabel(tr("or Game ID:"), downloadDialog);

    defaultText.remove(QRegExp("\\W*(\\(|\\[).+(\\)|\\])\\W*"));
    gameNameField = new QLineEdit(defaultText, downloadDialog);
    gameIDField = new QLineEdit(downloadDialog);

    gameIDField->setToolTip(tr("From thegamesdb.net URL of game"));

    downloadButtonBox = new QDialogButtonBox(Qt::Horizontal, downloadDialog);
    downloadButtonBox->addButton(tr("Search"), QDialogButtonBox::AcceptRole);
    downloadButtonBox->addButton(QDialogButtonBox::Cancel);

    downloadLayout->addWidget(fileLabel, 0, 0, 1, 2);
    downloadLayout->addWidget(gameNameLabel, 1, 0);
    downloadLayout->addWidget(gameIDLabel, 2, 0);
    downloadLayout->addWidget(gameNameField, 1, 1);
    downloadLayout->addWidget(gameIDField, 2, 1);
    downloadLayout->addWidget(downloadButtonBox, 4, 0, 1, 3);
    downloadLayout->setRowStretch(3,1);
    downloadLayout->setColumnStretch(1,1);

    downloadLayout->setColumnMinimumWidth(1, 300);
    downloadLayout->setRowMinimumHeight(0, 20);
    downloadLayout->setRowMinimumHeight(3, 20);

    connect(downloadButtonBox, SIGNAL(accepted()), this, SLOT(runDownloader()));
    connect(downloadButtonBox, SIGNAL(rejected()), downloadDialog, SLOT(close()));

    downloadDialog->setLayout(downloadLayout);

    downloadDialog->exec();
}


void Mupen64PlusQt::openEditor()
{
    QString configPath = SETTINGS.value("Paths/config", "").toString();
    QDir configDir = QDir(configPath);
    QString configFile = configDir.absoluteFilePath("mupen64plus.cfg");
    QFile config(configFile);

    if (configPath == "" || !config.exists()) {
        QMessageBox::information(this, "Not Found", QString("Editor requires config directory to be\n")
                                 + "set to a directory with mupen64plus.cfg.");
    } else {
        ConfigEditor configEditor(configFile, this);
        configEditor.exec();
    }
}


void Mupen64PlusQt::openLog()
{
    if (lastOutput == "") {
        QMessageBox::information(this, "No Output", QString("There is no log. Either Mupen64Plus has not ")
                                 + "yet\nrun or there was no output from the last run.");
    } else {
        logDialog = new QDialog(this);
        logDialog->setWindowTitle(tr("Mupen64Plus Log"));
        logDialog->setMinimumSize(600, 400);

        logLayout = new QGridLayout(logDialog);
        logLayout->setContentsMargins(5, 10, 5, 10);

        logArea = new QTextEdit(logDialog);
        logArea->setWordWrapMode(QTextOption::NoWrap);

        QFont font;
#ifdef Q_OS_LINUX
        font.setFamily("Monospace");
        font.setPointSize(9);
#else
        font.setFamily("Courier");
        font.setPointSize(10);
#endif
        font.setFixedPitch(true);
        logArea->setFont(font);

        logArea->setPlainText(lastOutput);

        logButtonBox = new QDialogButtonBox(Qt::Horizontal, logDialog);
        logButtonBox->addButton(tr("Close"), QDialogButtonBox::AcceptRole);

        logLayout->addWidget(logArea, 0, 0);
        logLayout->addWidget(logButtonBox, 1, 0);

        connect(logButtonBox, SIGNAL(accepted()), logDialog, SLOT(close()));

        logDialog->setLayout(logLayout);

        logDialog->exec();
    }
}


void Mupen64PlusQt::openOptions()
{
    QString tableImageBefore = SETTINGS.value("Table/imagesize", "Medium").toString();
    QString columnsBefore = SETTINGS.value("Table/columns", "Filename|Size").toString();

    SettingsDialog settingsDialog(this, 0);
    settingsDialog.exec();

    QString tableImageAfter = SETTINGS.value("Table/imagesize", "Medium").toString();
    QString columnsAfter = SETTINGS.value("Table/columns", "Filename|Size").toString();

    //Reset columns widths if user has selected different columns to display
    if (columnsBefore != columnsAfter) {
        SETTINGS.setValue("Table/width", "");
        romTree->setColumnCount(3);
        romTree->setHeaderLabels(QStringList(""));
    }

    QString romSave = SETTINGS.value("Paths/roms","").toString();
    if (romPath != romSave) {
        romPath = romSave;
        romDir = QDir(romPath);
        addRoms();
    } else {
        if (tableImageBefore != tableImageAfter)
            cachedRoms(true);
        else
            cachedRoms(false);
    }

    setGridBackground();
    toggleMenus(true);
}


void Mupen64PlusQt::openRom()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open ROM File"), romPath,
                                                tr("N64 ROMs (*.z64 *.n64 *.v64);;All Files (*)"));
    if (path != "")
        runEmulator(path);
}


void Mupen64PlusQt::readMupen64PlusOutput()
{
    lastOutput = mupen64proc->readAllStandardOutput();
}


void Mupen64PlusQt::resetLayouts(QStringList tableVisible, bool imageUpdated)
{
    int hidden = 3;

    saveColumnWidths();
    QStringList widths = SETTINGS.value("Table/width", "").toString().split("|");

    headerLabels.clear();
    headerLabels << "" << "" << "" << tableVisible; //First three blank for hidden columns

    //Remove Game Cover title for aesthetics
    for (int i = 0; i < headerLabels.size(); i++)
        if (headerLabels.at(i) == "Game Cover") headerLabels.replace(i, "");

    romTree->setColumnCount(headerLabels.size());
    romTree->setHeaderLabels(headerLabels);
    headerView->setSortIndicatorShown(false);

    int height = 0, width = 0;
    if (tableVisible.contains("Game Cover")) {
        QString size = SETTINGS.value("Table/imagesize","Medium").toString();
        if (size == "Extra Small") { height = 25;  width = 40;  }
        if (size == "Small")       { height = 38;  width = 55;  }
        if (size == "Medium")      { height = 55;  width = 75;  }
        if (size == "Large")       { height = 80;  width = 110; }
        if (size == "Extra Large") { height = 110; width = 150; }

        romTree->setStyleSheet("QTreeView { border: none; } QTreeView::item { height: "
                               + QString::number(height) + "px; }");
    } else
        romTree->setStyleSheet("QTreeView { border: none; } QTreeView::item { height: 25px; }");

    QStringList sort = SETTINGS.value("Table/sort", "").toString().split("|");
    if (sort.size() == 2) {
        if (sort[1] == "descending")
            headerView->setSortIndicator(tableVisible.indexOf(sort[0]) + hidden, Qt::DescendingOrder);
        else
            headerView->setSortIndicator(tableVisible.indexOf(sort[0]) + hidden, Qt::AscendingOrder);
    }

    romTree->setColumnHidden(0, true); //Hidden filename for launching emulator
    romTree->setColumnHidden(1, true); //Hidden goodname for searching
    romTree->setColumnHidden(2, true); //Hidden md5 for cache info

    int i = hidden;
    foreach (QString current, tableVisible)
    {
        if (i == hidden) {
            int c = i;
            if (current == "Game Cover") //If first column is game cover, use next column
                c++;

            if (SETTINGS.value("Table/stretchfirstcolumn", "true") == "true") {
#if QT_VERSION >= 0x050000
                romTree->header()->setSectionResizeMode(c, QHeaderView::Stretch);
#else
                romTree->header()->setResizeMode(c, QHeaderView::Stretch);
#endif
            } else {
#if QT_VERSION >= 0x050000
                romTree->header()->setSectionResizeMode(c, QHeaderView::Interactive);
#else
                romTree->header()->setResizeMode(c, QHeaderView::Interactive);
#endif
            }
        }

        if (widths.size() == tableVisible.size()) {
            romTree->setColumnWidth(i, widths[i - hidden].toInt());
        } else {
            if (current == "Overview")
                romTree->setColumnWidth(i, 400);
            else if (current == "GoodName" || current.left(8) == "Filename" || current == "Game Title")
                romTree->setColumnWidth(i, 300);
            else if (current == "MD5")
                romTree->setColumnWidth(i, 250);
            else if (current == "Internal Name" || current == "Publisher" || current == "Developer")
                romTree->setColumnWidth(i, 200);
            else if (current == "ESRB" || current == "Genre")
                romTree->setColumnWidth(i, 150);
            else if (current == "Save Type" || current == "Release Date")
                romTree->setColumnWidth(i, 100);
            else if (current == "CRC1" || current == "CRC2")
                romTree->setColumnWidth(i, 90);
            else if (current == "Size" || current == "Rumble" || current == "Players" || current == "Rating")
                romTree->setColumnWidth(i, 75);
            else if (current == "Game Cover")
                romTree->setColumnWidth(i, width);
        }

        //Overwrite saved value if switching image sizes
        if (imageUpdated && current == "Game Cover")
            romTree->setColumnWidth(i, width);

        i++;
    }


    //Reset grid view
    QLayoutItem *gridItem;
    while ((gridItem = gridLayout->takeAt(0)) != NULL)
    {
        delete gridItem->widget();
        delete gridItem;
    }

    gridCurrent = false;


    //Reset list view
    QLayoutItem *listItem;
    while ((listItem = listLayout->takeAt(0)) != NULL)
    {
        delete listItem->widget();
        delete listItem;
    }

    listCurrent = false;
}


void Mupen64PlusQt::runDownloader()
{
    downloadDialog->close();
    cacheGameInfo(getCurrentRomInfo(2).toLower(), gameNameField->text(), gameIDField->text(), true);

    delete downloadDialog;
}


void Mupen64PlusQt::runEmulator(QString completeRomPath)
{
    QString mupen64Path = SETTINGS.value("Paths/mupen64plus", "").toString();
    QString dataPath = SETTINGS.value("Paths/data", "").toString();
    QString configPath = SETTINGS.value("Paths/config", "").toString();
    QString pluginPath = SETTINGS.value("Paths/plugins", "").toString();
    dataDir = QDir(dataPath);
    configDir = QDir(configPath);
    pluginDir = QDir(pluginPath);

    QString emuMode = SETTINGS.value("Emulation/mode", "").toString();

    QString resolution = SETTINGS.value("Graphics/resolution", "").toString();

    QString videoPlugin = SETTINGS.value("Plugins/video", "").toString();
    QString audioPlugin = SETTINGS.value("Plugins/audio", "").toString();
    QString inputPlugin = SETTINGS.value("Plugins/input", "").toString();
    QString rspPlugin = SETTINGS.value("Plugins/rsp", "").toString();

    QFile mupen64File(mupen64Path);
    QFile romFile(completeRomPath);

    if(!mupen64File.exists() || QFileInfo(mupen64File).isDir() || !QFileInfo(mupen64File).isExecutable()) {
        QMessageBox::warning(this, "Warning", "Mupen64Plus executable not found.");
        return;
    }

    if(!romFile.exists() || QFileInfo(romFile).isDir()) {
        QMessageBox::warning(this, "Warning", "ROM file not found.");
        return;
    }

    QStringList args;

    if (SETTINGS.value("saveoptions", "").toString() == "true")
        args << "--saveoptions";
    else
        args << "--nosaveoptions";

    if (dataPath != "" && dataDir.exists())
        args << "--datadir" << dataPath;
    if (configPath != "" && configDir.exists())
        args << "--configdir" << configPath;
    if (pluginPath != "" && pluginDir.exists())
        args << "--plugindir" << pluginPath;

    if (emuMode != "")
        args << "--emumode" << emuMode;

    if (SETTINGS.value("Graphics/osd", "").toString() == "true")
        args << "--osd";
    else
        args << "--noosd";

    if (SETTINGS.value("Graphics/fullscreen", "").toString() == "true")
        args << "--fullscreen";
    else
        args << "--windowed";

    if (resolution != "")
        args << "--resolution" << resolution;

    if (videoPlugin != "")
        args << "--gfx" << videoPlugin;
    if (audioPlugin != "")
        args << "--audio" << audioPlugin;
    if (inputPlugin != "")
        args << "--input" << inputPlugin;
    if (rspPlugin != "")
        args << "--rsp" << rspPlugin;

    args << completeRomPath;

    toggleMenus(false);

    mupen64proc = new QProcess(this);
    connect(mupen64proc, SIGNAL(finished(int)), this, SLOT(readMupen64PlusOutput()));
    connect(mupen64proc, SIGNAL(finished(int)), this, SLOT(enableButtons()));
    connect(mupen64proc, SIGNAL(finished(int)), this, SLOT(checkStatus(int)));

    mupen64proc->setWorkingDirectory(QFileInfo(mupen64File).dir().canonicalPath());
    mupen64proc->setProcessChannelMode(QProcess::MergedChannels);
    mupen64proc->start(mupen64Path, args);
}


void Mupen64PlusQt::runEmulatorFromMenu()
{
    QString visibleLayout = layoutGroup->checkedAction()->data().toString();

    if (visibleLayout == "None")
        return;
    else if (visibleLayout == "Table View")
        runEmulatorFromRomTree();
    else if (visibleLayout == "Grid View" && gridCurrent)
        runEmulatorFromWidget(gridLayout->itemAt(currentGridRom)->widget());
    else if (visibleLayout == "List View" && listCurrent)
        runEmulatorFromWidget(listLayout->itemAt(currentListRom)->widget());
}


void Mupen64PlusQt::runEmulatorFromRomTree()
{
    QString completeRomFileName = QVariant(romTree->currentItem()->data(0, 0)).toString();
    QString completeRomPath = romDir.absoluteFilePath(completeRomFileName);
    runEmulator(completeRomPath);
}


void Mupen64PlusQt::runEmulatorFromWidget(QWidget *current)
{
    QString completeRomFileName = current->property("fileName").toString();
    QString completeRomPath = romDir.absoluteFilePath(completeRomFileName);
    runEmulator(completeRomPath);
}


void Mupen64PlusQt::saveColumnWidths()
{
    QStringList widths;

    for (int i = 3; i < romTree->columnCount(); i++)
    {
        widths << QString::number(romTree->columnWidth(i));
    }

    if (widths.size() > 0)
        SETTINGS.setValue("Table/width", widths.join("|"));
}


void Mupen64PlusQt::saveSortOrder(int column, Qt::SortOrder order)
{
    QString columnName = headerLabels.value(column);

    if (order == Qt::DescendingOrder)
        SETTINGS.setValue("Table/sort", columnName + "|descending");
    else
        SETTINGS.setValue("Table/sort", columnName + "|ascending");
}


void Mupen64PlusQt::setGridBackground()
{
    gridView->setStyleSheet("#gridView { border: none; }");

    QString background = SETTINGS.value("Grid/background", "").toString();
    if (background != "") {
        QFile backgroundFile(background);

        if (backgroundFile.exists() && !QFileInfo(backgroundFile).isDir())
            gridView->setStyleSheet(QString()
                + "#gridView { "
                    + "border: none; "
                    + "background: url(" + background + "); "
                    + "background-attachment: fixed; "
                    + "background-position: top center; "
                + "} #gridWidget { background: transparent; } "
            );
    }
}


void Mupen64PlusQt::setupProgressDialog(QStringList item)
{
    progress = new QProgressDialog("Loading ROMs...", "Cancel", 0, item.size(), this);
#if QT_VERSION >= 0x050000
    progress->setWindowFlags(progress->windowFlags() & ~Qt::WindowCloseButtonHint);
    progress->setWindowFlags(progress->windowFlags() & ~Qt::WindowMinimizeButtonHint);
    progress->setWindowFlags(progress->windowFlags() & ~Qt::WindowContextHelpButtonHint);
#else
    progress->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
#endif
    progress->setCancelButton(0);
    progress->setWindowModality(Qt::WindowModal);
}


void Mupen64PlusQt::stopEmulator()
{
    mupen64proc->terminate();
}


void Mupen64PlusQt::toggleMenus(bool active)
{
    foreach (QAction *next, menuEnable)
        next->setEnabled(active);

    foreach (QAction *next, menuDisable)
        next->setEnabled(!active);

    romTree->setEnabled(active);
    gridView->setEnabled(active);
    listView->setEnabled(active);

    if (romTree->currentItem() == NULL && !gridCurrent && !listCurrent) {
        downloadAction->setEnabled(false);
        startAction->setEnabled(false);
    }

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "") {
        downloadAction->setEnabled(false);
    }
}

void Mupen64PlusQt::updateLayoutSetting()
{
    QString visibleLayout = layoutGroup->checkedAction()->data().toString();
    SETTINGS.setValue("View/layout", visibleLayout);

    emptyView->setHidden(true);
    romTree->setHidden(true);
    gridView->setHidden(true);
    listView->setHidden(true);

    cachedRoms();

    if (visibleLayout == "None")
        emptyView->setHidden(false);
    else if (visibleLayout == "Table View")
        romTree->setHidden(false);
    else if (visibleLayout == "Grid View")
        gridView->setHidden(false);
    else if (visibleLayout == "List View")
        listView->setHidden(false);

    startAction->setEnabled(false);
    downloadAction->setEnabled(false);
}


bool romSorter(const Rom &firstRom, const Rom &lastRom) {
    QString sort, direction;

    QString layout = SETTINGS.value("View/layout", "Table View").toString();
    if (layout == "Grid View") {
        sort = SETTINGS.value("Grid/sort", "Filename").toString();
        direction = SETTINGS.value("Grid/sortdirection", "ascending").toString();
    } else if (layout == "List View") {
        sort = SETTINGS.value("List/sort", "Filename").toString();
        direction = SETTINGS.value("List/sortdirection", "ascending").toString();
    } else //just return sort by filename
        return firstRom.fileName < lastRom.fileName;

    if (direction == "ascending") {
        if (sort == "Filename") {
            return firstRom.fileName < lastRom.fileName;
        } else if (sort == "GoodName") {
            return firstRom.goodName < lastRom.goodName;
        } else if (sort == "Internal Name") {
            return firstRom.internalName < lastRom.internalName;
        } else if (sort == "Size") {
            if (firstRom.sortSize != lastRom.sortSize)
                return firstRom.sortSize < lastRom.sortSize;
        } else if (sort == "Game Title") {
            return firstRom.gameTitle < lastRom.gameTitle;
        } else if (sort == "Release Date") {
            return firstRom.sortDate < lastRom.sortDate;
        } else if (sort == "ESRB") {
            if (firstRom.esrb != lastRom.esrb)
                return firstRom.esrb < lastRom.esrb;
        } else if (sort == "Genre") {
            if (firstRom.genre != lastRom.genre)
                return firstRom.genre < lastRom.genre;
        } else if (sort == "Publisher") {
            if (firstRom.publisher != lastRom.publisher)
                return firstRom.publisher < lastRom.publisher;
        } else if (sort == "Developer") {
            if (firstRom.developer != lastRom.developer)
                return firstRom.developer < lastRom.developer;
        } else if (sort == "Rating") {
            if (firstRom.rating != lastRom.rating)
                return firstRom.rating < lastRom.rating;
        }

        //If values are equal, sort based on name of game
        if (firstRom.gameTitle != "" && lastRom.gameTitle != "")
            return firstRom.gameTitle < lastRom.gameTitle;
        else if (firstRom.goodName != "" && lastRom.goodName != "")
            return firstRom.goodName < lastRom.goodName;
        else
            return firstRom.fileName < lastRom.fileName;

    } else if (direction == "descending") {
        if (sort == "Filename") {
            return firstRom.fileName > lastRom.fileName;
        } else if (sort == "GoodName") {
            return firstRom.goodName > lastRom.goodName;
        } else if (sort == "Internal Name") {
            return firstRom.internalName > lastRom.internalName;
        } else if (sort == "Size") {
            if (firstRom.sortSize != lastRom.sortSize)
                return firstRom.sortSize > lastRom.sortSize;
        } else if (sort == "Game Title") {
            return firstRom.gameTitle > lastRom.gameTitle;
        } else if (sort == "Release Date") {
            return firstRom.sortDate > lastRom.sortDate;
        } else if (sort == "ESRB") {
            if (firstRom.esrb != lastRom.esrb)
                return firstRom.esrb > lastRom.esrb;
        } else if (sort == "Genre") {
            if (firstRom.genre != lastRom.genre)
                return firstRom.genre > lastRom.genre;
        } else if (sort == "Publisher") {
            if (firstRom.publisher != lastRom.publisher)
                return firstRom.publisher > lastRom.publisher;
        } else if (sort == "Developer") {
            if (firstRom.developer != lastRom.developer)
                return firstRom.developer > lastRom.developer;
        } else if (sort == "Rating") {
            if (firstRom.rating != lastRom.rating)
                return firstRom.rating > lastRom.rating;
        }

        //If values are equal, sort based on name of game
        if (firstRom.gameTitle != "" && lastRom.gameTitle != "")
            return firstRom.gameTitle > lastRom.gameTitle;
        else if (firstRom.goodName != "" && lastRom.goodName != "")
            return firstRom.goodName > lastRom.goodName;
        else
            return firstRom.fileName > lastRom.fileName;
    }

    return firstRom.fileName < lastRom.fileName;
}
