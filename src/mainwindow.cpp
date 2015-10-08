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

#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("Mupen64Plus-Qt"));
    setWindowIcon(QIcon(":/images/mupen64plus.png"));

    autoloadSettings();

    emulation = new EmulatorHandler(this);

    connect(emulation, SIGNAL(started()), this, SLOT(disableButtons()));
    connect(emulation, SIGNAL(finished()), this, SLOT(enableButtons()));
    connect(emulation, SIGNAL(showLog()), this, SLOT(openLog()));


    romCollection = new RomCollection(QStringList() << "*.z64" << "*.v64" << "*.n64" << "*.zip",
                                      QStringList() << SETTINGS.value("Paths/roms","").toString().split("|"),
                                      this);

    connect(romCollection, SIGNAL(updateStarted(bool)), this, SLOT(disableViews(bool)));
    connect(romCollection, SIGNAL(romAdded(Rom*, int)), this, SLOT(addToView(Rom*, int)));
    connect(romCollection, SIGNAL(updateEnded(int, bool)), this, SLOT(enableViews(int, bool)));


    mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);
    setGeometry(QRect(SETTINGS.value("Geometry/windowx", 0).toInt(),
                      SETTINGS.value("Geometry/windowy", 0).toInt(),
                      SETTINGS.value("Geometry/width", 900).toInt(),
                      SETTINGS.value("Geometry/height", 600).toInt()));

    createMenu();
    createRomView();

    mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setMenuBar(menuBar);

    mainLayout->addWidget(emptyView);
    mainLayout->addWidget(tableView);
    mainLayout->addWidget(gridView);
    mainLayout->addWidget(listView);

    mainLayout->setMargin(0);

    mainWidget->setLayout(mainLayout);
    mainWidget->setMinimumSize(300, 200);
}


void MainWindow::addToView(Rom *currentRom, int count)
{
    if (SETTINGS.value("View/layout", "None") == "Table View")
        addToTableView(currentRom);
    else if (SETTINGS.value("View/layout", "None") == "Grid View")
        addToGridView(currentRom, count);
    else if (SETTINGS.value("View/layout", "None") == "List View")
        addToListView(currentRom, count);
}


void MainWindow::addToGridView(Rom *currentRom, int count)
{
    ClickableWidget *gameGridItem = new ClickableWidget(gridWidget);
    gameGridItem->setMinimumWidth(getGridSize("width"));
    gameGridItem->setMaximumWidth(getGridSize("width"));
    gameGridItem->setGraphicsEffect(getShadow(false));

    //Assign ROM data to widget for use in click events
    gameGridItem->setProperty("fileName", currentRom->fileName);
    gameGridItem->setProperty("directory", currentRom->directory);
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

    if (currentRom->imageExists) {
        //Use uniform aspect ratio to account for fluctuations in TheGamesDB box art
        Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio;

        //Don't warp aspect ratio though if image is too far away from standard size (JP box art)
        float aspectRatio = float(currentRom->image.width()) / currentRom->image.height();

        if (aspectRatio < 1.1 || aspectRatio > 1.8)
            aspectRatioMode = Qt::KeepAspectRatio;

        image = currentRom->image.scaled(getImageSize("Grid"), aspectRatioMode, Qt::SmoothTransformation);
    } else
        image = QPixmap(":/images/not-found.png").scaled(getImageSize("Grid"), Qt::IgnoreAspectRatio,
                                                         Qt::SmoothTransformation);

    gridImageLabel->setPixmap(image);
    gridImageLabel->setAlignment(Qt::AlignCenter);
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

        gridTextLabel->setStyleSheet("QLabel { font-weight: bold; color: " + textHex + "; font-size: "
                                     + QString::number(fontSize) + "px; }");
        gridTextLabel->setWordWrap(true);
        gridTextLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

        gameGridLayout->addWidget(gridTextLabel, 2, 1);
    }

    gameGridItem->setLayout(gameGridLayout);

    gameGridItem->setMinimumHeight(gameGridItem->sizeHint().height());

    int columnCount = SETTINGS.value("Grid/columncount", "4").toInt();
    gridLayout->addWidget(gameGridItem, count / columnCount + 1, count % columnCount + 1);
    gridWidget->adjustSize();

    connect(gameGridItem, SIGNAL(singleClicked(QWidget*)), this, SLOT(highlightGridWidget(QWidget*)));
    connect(gameGridItem, SIGNAL(doubleClicked(QWidget*)), this, SLOT(launchRomFromWidget(QWidget*)));
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
    gameListItem->setProperty("directory", currentRom->directory);
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
        listImageLabel->setMinimumHeight(getImageSize("List").height());
        listImageLabel->setMinimumWidth(getImageSize("List").width());

        QPixmap image;

        if (currentRom->imageExists)
            image = currentRom->image.scaled(getImageSize("List"), Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation);
        else
            image = QPixmap(":/images/not-found.png").scaled(getImageSize("List"), Qt::KeepAspectRatio,
                                                             Qt::SmoothTransformation);

        listImageLabel->setPixmap(image);
        listImageLabel->setAlignment(Qt::AlignCenter);
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
    connect(gameListItem, SIGNAL(doubleClicked(QWidget*)), this, SLOT(launchRomFromWidget(QWidget*)));
}


void MainWindow::addToTableView(Rom *currentRom)
{
    QStringList visible = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");

    if (visible.join("") == "") //Otherwise no columns, so don't bother populating
        return;

    fileItem = new TreeWidgetItem(tableView);

    //Filename for launching ROM
    fileItem->setText(0, currentRom->fileName);

    //Directory ROM is located in
    fileItem->setText(1, currentRom->directory);

    //GoodName or Internal Name for searching
    if (currentRom->goodName == "Unknown ROM" || currentRom->goodName == "Requires catalog file")
        fileItem->setText(2, currentRom->internalName);
    else
        fileItem->setText(2, currentRom->goodName);

    //MD5 for cache info
    fileItem->setText(3, currentRom->romMD5.toLower());

    //Zip file
    fileItem->setText(4, currentRom->zipFile);

    int i = 5, c = 0;
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

    tableView->addTopLevelItem(fileItem);


    if (currentRom->imageExists && addImage) {
        QPixmap image(currentRom->image.scaled(getImageSize("Table"), Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation));

        QWidget *imageContainer = new QWidget(tableView);
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

        tableView->setItemWidget(fileItem, c, imageContainer);
    }
}


void MainWindow::autoloadSettings()
{
    QString mupen64Path = SETTINGS.value("Paths/mupen64plus", "").toString();
    QString dataPath = SETTINGS.value("Paths/data", "").toString();
    QString pluginPath = SETTINGS.value("Paths/plugins", "").toString();

    if (mupen64Path == "" && dataPath == "" && pluginPath == "") {
#ifdef OS_LINUX_OR_BSD
        //If user has not entered any settings, check common locations for them
        QStringList mupen64Check, dataCheck, pluginCheck;

        mupen64Check << "/usr/bin/mupen64plus"
                     << "/usr/games/mupen64plus"
                     << "/usr/local/bin/mupen64plus";

        pluginCheck  << "/usr/lib/mupen64plus"
                     << "/usr/lib64/mupen64plus"
                     << "/usr/lib/x86_64-linux-gnu/mupen64plus"
                     << "/usr/lib/i386-linux-gnu/mupen64plus"
                     << "/usr/lib/mupen64plus/mupen64plus"
                     << "/usr/lib64/mupen64plus/mupen64plus/"
                     << "/usr/local/lib/mupen64plus";

        dataCheck    << "/usr/share/mupen64plus"
                     << "/usr/share/games/mupen64plus"
                     << "/usr/local/share/mupen64plus";


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

        if (QFileInfo(currentDir+"/mupen64plus-ui-console.exe").exists())
            SETTINGS.setValue("Paths/mupen64plus", currentDir+"/mupen64plus-ui-console.exe");
        else if (QFileInfo(currentDir+"/mupen64plus.exe").exists())
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

    //Check default location for mupen64plus.cfg in case user wants to use editor
    QString configPath = SETTINGS.value("Paths/config", "").toString();

    if (configPath == "") {
#if QT_VERSION >= 0x050000
        QString homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
#else
        QString homeDir = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#endif

#ifdef Q_OS_WIN
        QString configCheck = homeDir + "/AppData/Roaming/Mupen64Plus/";
#else
        QString configCheck = homeDir + "/.config/mupen64plus";
#endif

        if (QFileInfo(configCheck+"/mupen64plus.cfg").exists())
            SETTINGS.setValue("Paths/config", configCheck);
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
    deleteAction = fileMenu->addAction(tr("D&elete Current Info..."));
#ifndef Q_OS_OSX //OSX does not show the quit action so the separator is unneeded
    fileMenu->addSeparator();
#endif
    quitAction = fileMenu->addAction(tr("&Quit"));

    openAction->setIcon(QIcon::fromTheme("document-open"));
    refreshAction->setIcon(QIcon::fromTheme("view-refresh"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));

    downloadAction->setEnabled(false);
    deleteAction->setEnabled(false);

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
               << deleteAction
               << configureAction
               << editorAction
               << quitAction;

    //Create list of actions that are disabled when Mupen64Plus is not running
    menuDisable << stopAction;

    connect(openAction, SIGNAL(triggered()), this, SLOT(openRom()));
    connect(refreshAction, SIGNAL(triggered()), romCollection, SLOT(addRoms()));
    connect(downloadAction, SIGNAL(triggered()), this, SLOT(openDownloader()));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(openDeleteDialog()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(startAction, SIGNAL(triggered()), this, SLOT(launchRomFromMenu()));
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

    emptyIcon = new QLabel(emptyView);
    emptyIcon->setPixmap(QPixmap(":/images/mupen64plus.png"));

    emptyLayout->addWidget(emptyIcon, 1, 1);
    emptyLayout->setColumnStretch(0, 1);
    emptyLayout->setColumnStretch(2, 1);
    emptyLayout->setRowStretch(0, 1);
    emptyLayout->setRowStretch(2, 1);

    emptyView->setLayout(emptyLayout);


    //Create table view
    tableView = new QTreeWidget(this);
    tableView->setWordWrap(false);
    tableView->setAllColumnsShowFocus(true);
    tableView->setRootIsDecorated(false);
    tableView->setSortingEnabled(true);
    tableView->setStyleSheet("QTreeView { border: none; } QTreeView::item { height: 25px; }");

    headerView = new QHeaderView(Qt::Horizontal, this);
    tableView->setHeader(headerView);
    tableView->setHidden(true);


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
        tableView->setHidden(false);
    else if (visibleLayout == "Grid View")
        gridView->setHidden(false);
    else if (visibleLayout == "List View")
        listView->setHidden(false);
    else
        emptyView->setHidden(false);

    romCollection->cachedRoms();

    connect(tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(enableButtons()));
    connect(tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(launchRomFromTable()));
    connect(headerView, SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(saveSortOrder(int,Qt::SortOrder)));
}


void MainWindow::disableButtons()
{
    toggleMenus(false);
}


void MainWindow::disableViews(bool imageUpdated)
{
    resetLayouts(imageUpdated);
    tableView->clear();

    tableView->setEnabled(false);
    gridView->setEnabled(false);
    listView->setEnabled(false);
    downloadAction->setEnabled(false);
    deleteAction->setEnabled(false);
    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    //Save position in current layout
    positionx = 0;
    positiony = 0;

    if (SETTINGS.value("View/layout", "None") == "Table View") {
        positionx = tableView->horizontalScrollBar()->value();
        positiony = tableView->verticalScrollBar()->value();
    } else if (SETTINGS.value("View/layout", "None") == "Grid View") {
        positionx = gridView->horizontalScrollBar()->value();
        positiony = gridView->verticalScrollBar()->value();
    } else if (SETTINGS.value("View/layout", "None") == "List View") {
        positionx = listView->horizontalScrollBar()->value();
        positiony = listView->verticalScrollBar()->value();
    }
}


void MainWindow::enableButtons()
{
    toggleMenus(true);
}


void MainWindow::enableViews(int romCount, bool cached)
{
    if (romCount != 0) { //else no ROMs, so leave views disabled
        QStringList tableVisible = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");

        if (tableVisible.join("") != "")
            tableView->setEnabled(true);

        gridView->setEnabled(true);
        listView->setEnabled(true);

        if (cached) {
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
}


QString MainWindow::getCurrentRomInfo(int index)
{
    if (index < 3) {
        const char *infoChar;
        int table;

        switch (index) {
            case 0:  infoChar = "fileName"; table = 0; break;
            case 1:  infoChar = "search";   table = 2; break;
            case 2:  infoChar = "romMD5";   table = 3; break;
            default: infoChar = "";         table = 0; break;
        }

        QString visibleLayout = SETTINGS.value("View/layout", "None").toString();

        if (visibleLayout == "Table View")
            return tableView->currentItem()->data(table, 0).toString();
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


void MainWindow::launchRomFromMenu()
{
    QString visibleLayout = layoutGroup->checkedAction()->data().toString();

    if (visibleLayout == "Table View")
        launchRomFromTable();
    else if (visibleLayout == "Grid View" && gridCurrent)
        launchRomFromWidget(gridLayout->itemAt(currentGridRom)->widget());
    else if (visibleLayout == "List View" && listCurrent)
        launchRomFromWidget(listLayout->itemAt(currentListRom)->widget());
}


void MainWindow::launchRomFromTable()
{
    QString romFileName = QVariant(tableView->currentItem()->data(0, 0)).toString();
    QString romDirName = QVariant(tableView->currentItem()->data(1, 0)).toString();
    QString zipFileName = QVariant(tableView->currentItem()->data(4, 0)).toString();
    emulation->startEmulator(QDir(romDirName), romFileName, zipFileName);
}


void MainWindow::launchRomFromWidget(QWidget *current)
{
    QString romFileName = current->property("fileName").toString();
    QString romDirName = current->property("directory").toString();
    QString zipFileName = current->property("zipFile").toString();
    emulation->startEmulator(QDir(romDirName), romFileName, zipFileName);
}


void MainWindow::launchRomFromZip()
{
    QString fileName = zipList->currentItem()->text();
    zipDialog->close();

    emulation->startEmulator(QDir(), fileName, openPath);
}


void MainWindow::openAbout()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}


void MainWindow::openDeleteDialog()
{
    scrapper = new TheGamesDBScrapper(this);
    scrapper->deleteGameInfo(getCurrentRomInfo(0), getCurrentRomInfo(2));
    delete scrapper;

    romCollection->cachedRoms();
}


void MainWindow::openDownloader()
{
    DownloadDialog downloadDialog(getCurrentRomInfo(0), getCurrentRomInfo(1), getCurrentRomInfo(2), this);
    downloadDialog.exec();

    romCollection->cachedRoms();
}


void MainWindow::openEditor()
{
    QString configPath = SETTINGS.value("Paths/config", "").toString();
    QDir configDir = QDir(configPath);
    QString configFile = configDir.absoluteFilePath("mupen64plus.cfg");
    QFile config(configFile);

    if (configPath == "" || !config.exists()) {
        QMessageBox::information(this, "Not Found", QString("Editor requires config directory to be ")
                                 + "set to a directory with mupen64plus.cfg.\n\n"
                                 + "See here for the default config location:\n"
                                 + "https://code.google.com/p/mupen64plus/wiki/FileLocations");
    } else {
        ConfigEditor configEditor(configFile, this);
        configEditor.exec();
    }
}


void MainWindow::openLog()
{
    if (emulation->lastOutput == "") {
        QMessageBox::information(this, "No Output", QString("There is no log. Either Mupen64Plus has not ")
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
    QString downloadBefore = SETTINGS.value("Other/downloadinfo", "").toString();

    SettingsDialog settingsDialog(this, 0);
    settingsDialog.exec();

    QString tableImageAfter = SETTINGS.value("Table/imagesize", "Medium").toString();
    QString columnsAfter = SETTINGS.value("Table/columns", "Filename|Size").toString();
    QString downloadAfter = SETTINGS.value("Other/downloadinfo", "").toString();

    //Reset columns widths if user has selected different columns to display
    if (columnsBefore != columnsAfter) {
        SETTINGS.setValue("Table/width", "");
        tableView->setColumnCount(3);
        tableView->setHeaderLabels(QStringList(""));
    }

    QStringList romSave = SETTINGS.value("Paths/roms","").toString().split("|");
    if (romCollection->romPaths != romSave) {
        romCollection->updatePaths(romSave);
        romCollection->addRoms();
    } else if (downloadBefore == "" && downloadAfter == "true") {
        romCollection->addRoms();
    } else {
        if (tableImageBefore != tableImageAfter)
            romCollection->cachedRoms(true);
        else
            romCollection->cachedRoms(false);
    }

    setGridBackground();
    toggleMenus(true);
}


void MainWindow::openRom()
{
    QString filter = "N64 ROMs (";
    foreach (QString type, romCollection->getFileTypes(true)) filter += type + " ";
    filter += ");;All Files (*)";

    openPath = QFileDialog::getOpenFileName(this, tr("Open ROM File"), romCollection->romPaths.at(0), filter);
    if (openPath != "") {
        if (QFileInfo(openPath).suffix() == "zip") {
            QStringList zippedFiles = getZippedFiles(openPath);

            QString last;
            int count = 0;

            foreach (QString file, zippedFiles) {
                QString ext = file.right(4).toLower();

                if (romCollection->getFileTypes().contains("*" + ext)) {
                    last = file;
                    count++;
                }
            }

            if (count == 0)
                QMessageBox::information(this, tr("No ROMs"), tr("No ROMs found in ZIP file."));
            else if (count == 1)
                emulation->startEmulator(QDir(QFileInfo(openPath).dir()), last, openPath);
            else { //More than one ROM in zip file, so let user select
                openZipDialog(zippedFiles);
            }
        } else
            emulation->startEmulator(QDir(QFileInfo(openPath).dir()), openPath);
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

        if (romCollection->getFileTypes().contains("*" + ext))
            zipList->addItem(file);
    }
    zipList->setCurrentRow(0);

    zipButtonBox = new QDialogButtonBox(Qt::Horizontal, zipDialog);
    zipButtonBox->addButton(tr("Launch"), QDialogButtonBox::AcceptRole);
    zipButtonBox->addButton(QDialogButtonBox::Cancel);

    zipLayout->addWidget(zipList, 0, 0);
    zipLayout->addWidget(zipButtonBox, 1, 0);

    connect(zipList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(launchRomFromZip()));
    connect(zipButtonBox, SIGNAL(accepted()), this, SLOT(launchRomFromZip()));
    connect(zipButtonBox, SIGNAL(rejected()), zipDialog, SLOT(close()));

    zipDialog->setLayout(zipLayout);

    zipDialog->exec();
}


void MainWindow::resetLayouts(bool imageUpdated)
{
    QStringList tableVisible = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");

    int hidden = 5;

    saveColumnWidths();
    QStringList widths = SETTINGS.value("Table/width", "").toString().split("|");

    headerLabels.clear();
    headerLabels << "" << "" << "" << "" << "" << tableVisible; //First 5 blank for hidden columns

    //Remove Game Cover title for aesthetics
    for (int i = 0; i < headerLabels.size(); i++)
        if (headerLabels.at(i) == "Game Cover") headerLabels.replace(i, "");

    tableView->setColumnCount(headerLabels.size());
    tableView->setHeaderLabels(headerLabels);
    headerView->setSortIndicatorShown(false);

    int height = 0, width = 0;
    if (tableVisible.contains("Game Cover")) {
        //Get optimal height/width for cover column
        height = getImageSize("Table").height() * 1.1;
        width = getImageSize("Table").width() * 1.2;

        tableView->setStyleSheet("QTreeView { border: none; } QTreeView::item { height: "
                               + QString::number(height) + "px; }");
    } else
        tableView->setStyleSheet("QTreeView { border: none; } QTreeView::item { height: 25px; }");

    QStringList sort = SETTINGS.value("Table/sort", "").toString().split("|");
    if (sort.size() == 2) {
        if (sort[1] == "descending")
            headerView->setSortIndicator(tableVisible.indexOf(sort[0]) + hidden, Qt::DescendingOrder);
        else
            headerView->setSortIndicator(tableVisible.indexOf(sort[0]) + hidden, Qt::AscendingOrder);
    }

    tableView->setColumnHidden(0, true); //Hidden filename for launching emulator
    tableView->setColumnHidden(1, true); //Hidden directory of ROM location
    tableView->setColumnHidden(2, true); //Hidden goodname for searching
    tableView->setColumnHidden(3, true); //Hidden md5 for cache info
    tableView->setColumnHidden(4, true); //Hidden column for zip file

    int i = hidden;
    foreach (QString current, tableVisible)
    {
        if (i == hidden) {
            int c = i;
            if (current == "Game Cover") c++; //If first column is game cover, use next column

            if (SETTINGS.value("Table/stretchfirstcolumn", "true") == "true")
#if QT_VERSION >= 0x050000
                tableView->header()->setSectionResizeMode(c, QHeaderView::Stretch);
#else
                tableView->header()->setResizeMode(c, QHeaderView::Stretch);
#endif
            else
#if QT_VERSION >= 0x050000
                tableView->header()->setSectionResizeMode(c, QHeaderView::Interactive);
#else
                tableView->header()->setResizeMode(c, QHeaderView::Interactive);
#endif
        }

        if (widths.size() == tableVisible.size())
            tableView->setColumnWidth(i, widths[i - hidden].toInt());
        else
            tableView->setColumnWidth(i, getDefaultWidth(current, width));

        //Overwrite saved value if switching image sizes
        if (imageUpdated && current == "Game Cover")
            tableView->setColumnWidth(i, width);

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


void MainWindow::saveColumnWidths()
{
    QStringList widths;

    for (int i = 5; i < tableView->columnCount(); i++)
    {
        widths << QString::number(tableView->columnWidth(i));
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
    tableView->horizontalScrollBar()->setValue(positionx);
    tableView->verticalScrollBar()->setValue(positiony);
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

    tableView->setEnabled(active);
    gridView->setEnabled(active);
    listView->setEnabled(active);

    if (tableView->currentItem() == NULL && !gridCurrent && !listCurrent) {
        downloadAction->setEnabled(false);
        deleteAction->setEnabled(false);
        startAction->setEnabled(false);
    }

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "") {
        downloadAction->setEnabled(false);
        deleteAction->setEnabled(false);
    }
}

void MainWindow::updateLayoutSetting()
{
    QString visibleLayout = layoutGroup->checkedAction()->data().toString();
    SETTINGS.setValue("View/layout", visibleLayout);

    emptyView->setHidden(true);
    tableView->setHidden(true);
    gridView->setHidden(true);
    listView->setHidden(true);

    romCollection->cachedRoms();

    if (visibleLayout == "Table View")
        tableView->setHidden(false);
    else if (visibleLayout == "Grid View")
        gridView->setHidden(false);
    else if (visibleLayout == "List View")
        listView->setHidden(false);
    else
        emptyView->setHidden(false);

    startAction->setEnabled(false);
    downloadAction->setEnabled(false);
    deleteAction->setEnabled(false);
}
