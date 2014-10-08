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

#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("Mupen64Plus-Qt"));
    setWindowIcon(QIcon(":/images/mupen64plus.png"));

    setupDatabase();
    autoloadSettings();
    emulation = new EmulatorHandler(this);

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


Rom MainWindow::addRom(QByteArray *romData, QString fileName, QString zipFile, QSqlQuery query)
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

    initializeRom(&currentRom, romDir, false, this);

    return currentRom;
}


void MainWindow::addRoms()
{
    database.open();

    QSqlQuery query("DELETE FROM rom_collection", database);
    query.prepare(QString("INSERT INTO rom_collection ")
                  + "(filename, internal_name, md5, zip_file, size) "
                  + "VALUES (:filename, :internal_name, :md5, :zip_file, :size)");

    QList<Rom> roms;

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
            QStringList files = romDir.entryList(QStringList() << "*.z64" << "*.n64" << "*.v64" << "*.zip",
                                                 QDir::Files | QDir::NoSymLinks);

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
                            if (ext == ".z64" || ext == ".n64" || ext == ".v64") {
                                QByteArray *romData = getZippedRom(zippedFile, completeFileName);
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
        if (SETTINGS.value("View/layout", "None") == "Table View")
            addToTableView(&currentRom);
        else if (SETTINGS.value("View/layout", "None") == "Grid View")
            addToGridView(&currentRom, i);
        else if (SETTINGS.value("View/layout", "None") == "List View")
            addToListView(&currentRom, i);

        i++;
    }

    if (roms.size() != 0) {
        if (tableVisible.join("") != "")
            romTree->setEnabled(true);

        gridView->setEnabled(true);
        listView->setEnabled(true);
    }

    database.close();
}


void MainWindow::addToGridView(Rom *currentRom, int count)
{
    ClickableWidget *gameGridItem = new ClickableWidget(gridWidget);
    gameGridItem->setMinimumHeight(getGridSize("height"));
    gameGridItem->setMaximumHeight(getGridSize("height"));
    gameGridItem->setMinimumWidth(getGridSize("width"));
    gameGridItem->setMaximumWidth(getGridSize("width"));
    gameGridItem->setGraphicsEffect(getShadow(false));

    //Assign ROM data to widget for use in click events
    gameGridItem->setProperty("fileName", currentRom->fileName);
    if (currentRom->goodName == "Unknown ROM" || currentRom->goodName == "Requires catalog file")
        gameGridItem->setProperty("search", currentRom->internalName);
    else
        gameGridItem->setProperty("search", currentRom->goodName);
    gameGridItem->setProperty("romMD5", currentRom->romMD5);
    gameGridItem->setProperty("zipFile", currentRom->zipFile);

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

        text = getRomInfo(labelText, currentRom);

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


void MainWindow::addToListView(Rom *currentRom, int count)
{
    QStringList visible = SETTINGS.value("List/columns", "Filename|Internal Name|Size").toString().split("|");

    if (visible.join("") == "" && SETTINGS.value("List/displaycover", "") != "true")
        //Otherwise no columns, so don't bother populating
        return;

    ClickableWidget *gameListItem = new ClickableWidget(listWidget);
    gameListItem->setContentsMargins(0, 0, 20, 0);

    //Assign ROM data to widget for use in click events
    gameListItem->setProperty("fileName", currentRom->fileName);
    if (currentRom->goodName == "Unknown ROM" || currentRom->goodName == "Requires catalog file")
        gameListItem->setProperty("search", currentRom->internalName);
    else
        gameListItem->setProperty("search", currentRom->goodName);
    gameListItem->setProperty("romMD5", currentRom->romMD5);
    gameListItem->setProperty("zipFile", currentRom->zipFile);

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

        addition += getRomInfo(current, currentRom, true) + "<br />";

        if (i == 0 && SETTINGS.value("List/firstitemheader","true") == "true")
            addition += "</h2>";

        if (addition != "<style>h2 { margin: 0; }</style><b>" + current + ":</b> <br />")
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


void MainWindow::addToTableView(Rom *currentRom)
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

    //Zip file
    fileItem->setText(3, currentRom->zipFile);

    int i = 4, c = 0;
    bool addImage = false;

    foreach (QString current, visible)
    {
        QString text = getRomInfo(current, currentRom);
        fileItem->setText(i, text);

        if (current == "GoodName" || current == "Game Title") {
            if (text == "Unknown ROM" || text == "Requires catalog file" || text == "Not found") {
                fileItem->setForeground(i, QBrush(Qt::gray));
                fileItem->setData(i, Qt::UserRole, "ZZZ"); //end of sorting
            } else
                fileItem->setData(i, Qt::UserRole, text);
        }

        if (current == "Size")
            fileItem->setData(i, Qt::UserRole, currentRom->sortSize);

        if (current == "Release Date")
            fileItem->setData(i, Qt::UserRole, currentRom->sortDate);

        if (current == "Game Cover") {
            c = i;
            addImage = true;
        }

        QStringList center, right;

        center << "MD5" << "CRC1" << "CRC2" << "Rumble" << "ESRB" << "Genre" << "Publisher" << "Developer";
        right << "Size" << "Players" << "Save Type" << "Release Date" << "Rating";

        if (center.contains(current))
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        else if (right.contains(current))
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);

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


void MainWindow::autoloadSettings()
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
        QString currentDir = QCoreApplication::applicationDirPath();

        if (QFileInfo(currentDir+"/mupen64plus.exe").exists())
            SETTINGS.setValue("Paths/mupen64plus", currentDir+"/mupen64plus.exe");

        if (QFileInfo(currentDir+"/mupen64plus-video-rice.dll").exists())
            SETTINGS.setValue("Paths/plugins", currentDir);

        if (QFileInfo(currentDir+"/mupen64plus.ini").exists())
            SETTINGS.setValue("Paths/data", currentDir);
#endif

#ifdef Q_OS_OSX
        //Check for Mupen64Plus App within the same directory
        QString currentDir = QCoreApplication::applicationDirPath();

        QString mupen64App = currentDir+"/mupen64plus.app/Contents";
        if (QFileInfo(mupen64App+"/MacOS/mupen64plus").exists()) {
            SETTINGS.setValue("Paths/mupen64plus", mupen64App+"/MacOS/mupen64plus");
            SETTINGS.setValue("Paths/plugins", mupen64App+"/MacOS");
            SETTINGS.setValue("Paths/data", mupen64App+"/Resources");
        }
#endif
    }
}


void MainWindow::cachedRoms(bool imageUpdated)
{
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

    romTree->setEnabled(false);
    gridView->setEnabled(false);
    listView->setEnabled(false);
    romTree->clear();
    downloadAction->setEnabled(false);
    startAction->setEnabled(false);
    stopAction->setEnabled(false);


    //Save position in current layout
    positionx = 0;
    positiony = 0;

    if (SETTINGS.value("View/layout", "None") == "Table View") {
        positionx = romTree->horizontalScrollBar()->value();
        positiony = romTree->verticalScrollBar()->value();
    } else if (SETTINGS.value("View/layout", "None") == "Grid View") {
        positionx = gridView->horizontalScrollBar()->value();
        positiony = gridView->verticalScrollBar()->value();
    } else if (SETTINGS.value("View/layout", "None") == "List View") {
        positionx = listView->horizontalScrollBar()->value();
        positiony = listView->verticalScrollBar()->value();
    }


    QStringList tableVisible = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");
    resetLayouts(tableVisible, imageUpdated);

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

        initializeRom(&currentRom, romDir, true, this);
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

    int i = 0;
    foreach (Rom currentRom, roms)
    {
        if (SETTINGS.value("View/layout", "None") == "Table View")
            addToTableView(&currentRom);
        else if (SETTINGS.value("View/layout", "None") == "Grid View")
            addToGridView(&currentRom, i);
        else if (SETTINGS.value("View/layout", "None") == "List View")
            addToListView(&currentRom, i);

        i++;
    }

    if (roms.size() != 0) {
        if (tableVisible.join("") != "")
            romTree->setEnabled(true);

        gridView->setEnabled(true);
        listView->setEnabled(true);

        QTimer *timer = new QTimer(this);
        timer->setSingleShot(true);
        timer->setInterval(0);
        timer->start();

        if (SETTINGS.value("View/layout", "None") == "Table View")
            connect(timer, SIGNAL(timeout()), this, SLOT(setTablePosition()));
        else if (SETTINGS.value("View/layout", "None") == "Grid View")
            connect(timer, SIGNAL(timeout()), this, SLOT(setGridPosition()));
        else if (SETTINGS.value("View/layout", "None") == "List View")
            connect(timer, SIGNAL(timeout()), this, SLOT(setListPosition()));
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
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


void MainWindow::createMenu()
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

    QString layoutValue = SETTINGS.value("View/layout", "None").toString();

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
    connect(configureAction, SIGNAL(triggered()), this, SLOT(openSettings()));
    connect(editorAction, SIGNAL(triggered()), this, SLOT(openEditor()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(openAbout()));
    connect(layoutGroup, SIGNAL(triggered(QAction*)), this, SLOT(updateLayoutSetting()));
}


void MainWindow::createRomView()
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


    QString visibleLayout = SETTINGS.value("View/layout", "None").toString();

    if (visibleLayout == "Table View")
        romTree->setHidden(false);
    else if (visibleLayout == "Grid View")
        gridView->setHidden(false);
    else if (visibleLayout == "List View")
        listView->setHidden(false);
    else
        emptyView->setHidden(false);

    cachedRoms();

    connect(romTree, SIGNAL(clicked(QModelIndex)), this, SLOT(enableButtons()));
    connect(romTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(runEmulatorFromRomTree()));
    connect(headerView, SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(saveSortOrder(int,Qt::SortOrder)));
}


void MainWindow::enableButtons()
{
    toggleMenus(true);
}


QString MainWindow::getCurrentRomInfo(int index)
{
    if (index < 3) {
        const char *infoChar;

        switch (index) {
            case 0:  infoChar = "fileName"; break;
            case 1:  infoChar = "search";   break;
            case 2:  infoChar = "romMD5";   break;
            default: infoChar = "";         break;
        }

        QString visibleLayout = SETTINGS.value("View/layout", "None").toString();

        if (visibleLayout == "Table View")
            return romTree->currentItem()->data(index, 0).toString();
        else if (visibleLayout == "Grid View" && gridCurrent)
            return gridLayout->itemAt(currentGridRom)->widget()->property(infoChar).toString();
        else if (visibleLayout == "List View" && listCurrent)
            return listLayout->itemAt(currentListRom)->widget()->property(infoChar).toString();
    }

    return "";
}


void MainWindow::highlightGridWidget(QWidget *current)
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


void MainWindow::highlightListWidget(QWidget *current)
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


void MainWindow::openAbout()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}


void MainWindow::openDownloader()
{
    DownloadDialog downloadDialog(getCurrentRomInfo(0), getCurrentRomInfo(1), getCurrentRomInfo(2), this);
    downloadDialog.exec();

    cachedRoms();
}


void MainWindow::openEditor()
{
    QString configPath = SETTINGS.value("Paths/config", "").toString();
    QDir configDir = QDir(configPath);
    QString configFile = configDir.absoluteFilePath("mupen64plus.cfg");
    QFile config(configFile);

    if (configPath == "" || !config.exists()) {
        QMessageBox::information(this, "Not Found", QString("Editor requires config directory to be ")
                                 + "set to a directory with mupen64plus.cfg.");
    } else {
        ConfigEditor configEditor(configFile, this);
        configEditor.exec();
    }
}


void MainWindow::openLog()
{
    if (emulation->lastOutput == "") {
        QMessageBox::information(this, "No Output", QString("There is no log. Either CEN64 has not ")
                                 + "yet run or there was no output from the last run.");
    } else {
        LogDialog logDialog(emulation->lastOutput, this);
        logDialog.exec();
    }
}


void MainWindow::openSettings()
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


void MainWindow::openRom()
{
    openPath = QFileDialog::getOpenFileName(this, tr("Open ROM File"), romPath,
                                                tr("N64 ROMs (*.z64 *.n64 *.v64 *.zip);;All Files (*)"));
    if (openPath != "") {
        if (QFileInfo(openPath).suffix() == "zip") {
            QuaZip zipFile(openPath);
            zipFile.open(QuaZip::mdUnzip);

            QStringList zippedFiles = zipFile.getFileNameList();

            QString last;
            int count = 0;

            foreach (QString file, zippedFiles) {
                QString ext = file.right(4).toLower();

                if (ext == ".z64" || ext == ".n64" || ext == ".v64") {
                    last = file;
                    count++;
                }
            }

            if (count == 0)
                QMessageBox::information(this, tr("No ROMs"), tr("No ROMs found in ZIP file."));
            else if (count == 1)
                runEmulator(last, openPath);
            else { //More than one ROM in zip file, so let user select
                openZipDialog(zippedFiles);
            }
        } else
            runEmulator(openPath);
    }
}


void MainWindow::openZipDialog(QStringList zippedFiles)
{
    zipDialog = new QDialog(this);
    zipDialog->setWindowTitle(tr("Select ROM"));
    zipDialog->setMinimumSize(200, 150);
    zipDialog->resize(300, 150);

    zipLayout = new QGridLayout(zipDialog);
    zipLayout->setContentsMargins(5, 10, 5, 10);

    zipList = new QListWidget(zipDialog);
    foreach (QString file, zippedFiles) {
        QString ext = file.right(4);

        if (ext == ".z64" || ext == ".n64")
            zipList->addItem(file);
    }
    zipList->setCurrentRow(0);

    zipButtonBox = new QDialogButtonBox(Qt::Horizontal, zipDialog);
    zipButtonBox->addButton(tr("Launch"), QDialogButtonBox::AcceptRole);
    zipButtonBox->addButton(QDialogButtonBox::Cancel);

    zipLayout->addWidget(zipList, 0, 0);
    zipLayout->addWidget(zipButtonBox, 1, 0);

    connect(zipList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(runEmulatorFromZip()));
    connect(zipButtonBox, SIGNAL(accepted()), this, SLOT(runEmulatorFromZip()));
    connect(zipButtonBox, SIGNAL(rejected()), zipDialog, SLOT(close()));

    zipDialog->setLayout(zipLayout);

    zipDialog->exec();
}


void MainWindow::resetLayouts(QStringList tableVisible, bool imageUpdated)
{
    int hidden = 4;

    saveColumnWidths();
    QStringList widths = SETTINGS.value("Table/width", "").toString().split("|");

    headerLabels.clear();
    headerLabels << "" << "" << "" << "" << tableVisible; //First 4 blank for hidden columns

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
    romTree->setColumnHidden(3, true); //Hidden column for zip file

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


void MainWindow::runEmulator(QString romFileName, QString zipFileName)
{
    emulation->startEmulator(romDir, romFileName, zipFileName);
    connect(emulation, SIGNAL(finished()), this, SLOT(enableButtons()));
}


void MainWindow::runEmulatorFromMenu()
{
    QString visibleLayout = layoutGroup->checkedAction()->data().toString();

    if (visibleLayout == "Table View")
        runEmulatorFromRomTree();
    else if (visibleLayout == "Grid View" && gridCurrent)
        runEmulatorFromWidget(gridLayout->itemAt(currentGridRom)->widget());
    else if (visibleLayout == "List View" && listCurrent)
        runEmulatorFromWidget(listLayout->itemAt(currentListRom)->widget());
}


void MainWindow::runEmulatorFromRomTree()
{
    QString romFileName = QVariant(romTree->currentItem()->data(0, 0)).toString();
    QString zipFileName = QVariant(romTree->currentItem()->data(3, 0)).toString();
    runEmulator(romFileName, zipFileName);
}


void MainWindow::runEmulatorFromWidget(QWidget *current)
{
    QString romFileName = current->property("fileName").toString();
    QString zipFileName = current->property("zipFile").toString();
    runEmulator(romFileName, zipFileName);
}


void MainWindow::runEmulatorFromZip()
{
    QString fileName = zipList->currentItem()->text();
    zipDialog->close();

    runEmulator(fileName, openPath);
}


void MainWindow::saveColumnWidths()
{
    QStringList widths;

    for (int i = 4; i < romTree->columnCount(); i++)
    {
        widths << QString::number(romTree->columnWidth(i));
    }

    if (widths.size() > 0)
        SETTINGS.setValue("Table/width", widths.join("|"));
}


void MainWindow::saveSortOrder(int column, Qt::SortOrder order)
{
    QString columnName = headerLabels.value(column);

    if (order == Qt::DescendingOrder)
        SETTINGS.setValue("Table/sort", columnName + "|descending");
    else
        SETTINGS.setValue("Table/sort", columnName + "|ascending");
}


void MainWindow::setGridBackground()
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


void MainWindow::setGridPosition()
{
    gridView->horizontalScrollBar()->setValue(positionx);
    gridView->verticalScrollBar()->setValue(positiony);
}


void MainWindow::setListPosition()
{
    listView->horizontalScrollBar()->setValue(positionx);
    listView->verticalScrollBar()->setValue(positiony);
}


void MainWindow::setTablePosition()
{
    romTree->horizontalScrollBar()->setValue(positionx);
    romTree->verticalScrollBar()->setValue(positiony);
}


void MainWindow::setupDatabase()
{
    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(getDataLocation() + "/mupen64plus-qt.sqlite");

    if (!database.open())
        QMessageBox::warning(this, "Database Not Loaded",
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


void MainWindow::setupProgressDialog(int size)
{
    progress = new QProgressDialog("Loading ROMs...", "Cancel", 0, size, this);
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


void MainWindow::stopEmulator()
{
    emulation->stopEmulator();
}


void MainWindow::toggleMenus(bool active)
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

void MainWindow::updateLayoutSetting()
{
    QString visibleLayout = layoutGroup->checkedAction()->data().toString();
    SETTINGS.setValue("View/layout", visibleLayout);

    emptyView->setHidden(true);
    romTree->setHidden(true);
    gridView->setHidden(true);
    listView->setHidden(true);

    cachedRoms();

    if (visibleLayout == "Table View")
        romTree->setHidden(false);
    else if (visibleLayout == "Grid View")
        gridView->setHidden(false);
    else if (visibleLayout == "List View")
        listView->setHidden(false);
    else
        emptyView->setHidden(false);

    startAction->setEnabled(false);
    downloadAction->setEnabled(false);
}
