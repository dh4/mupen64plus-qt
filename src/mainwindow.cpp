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

#include "global.h"
#include "common.h"

#include "dialogs/aboutdialog.h"
#include "dialogs/configeditor.h"
#include "dialogs/controlinfodialog.h"
#include "dialogs/downloaddialog.h"
#include "dialogs/gamesettingsdialog.h"
#include "dialogs/logdialog.h"
#include "dialogs/settingsdialog.h"

#include "emulation/emulatorhandler.h"

#include "roms/romcollection.h"
#include "roms/thegamesdbscraper.h"

#include "views/gridview.h"
#include "views/listview.h"
#include "views/tableview.h"

#include <QActionGroup>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QOperatingSystemVersion>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>

#if defined(Q_OS_WIN) || defined(Q_OS_OSX)
#include <QCoreApplication>
#endif


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(AppName);
    setWindowIcon(QIcon(":/images/"+ParentNameLower+".png"));
    installEventFilter(this);

    autoloadSettings();

    emulation = new EmulatorHandler(this);
    romCollection = new RomCollection(QStringList() << "*.z64" << "*.v64" << "*.n64" << "*.zip",
                                      QStringList() << SETTINGS.value("Paths/roms","").toString().split("|"),
                                      this);
    createMenu();
    createRomView();

    connect(emulation, SIGNAL(started()), this, SLOT(disableButtons()));
    connect(emulation, SIGNAL(finished()), this, SLOT(enableButtons()));
    connect(emulation, SIGNAL(showLog()), this, SLOT(openLog()));

    connect(romCollection, SIGNAL(updateStarted(bool)), this, SLOT(disableViews(bool)));
    connect(romCollection, SIGNAL(romAdded(Rom*, int)), this, SLOT(addToView(Rom*, int)));
    connect(romCollection, SIGNAL(updateEnded(int, bool)), this, SLOT(enableViews(int, bool)));

    romCollection->cachedRoms(false, true);


    mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);
    setGeometry(QRect(SETTINGS.value("Geometry/windowx", 0).toInt(),
                      SETTINGS.value("Geometry/windowy", 0).toInt(),
                      SETTINGS.value("Geometry/width", 900).toInt(),
                      SETTINGS.value("Geometry/height", 600).toInt()));

    if (SETTINGS.value("View/fullscreen", "").toString() == "true")
        updateFullScreenMode();

    mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setMenuBar(menuBar);

    mainLayout->addWidget(emptyView);
    mainLayout->addWidget(tableView);
    mainLayout->addWidget(gridView);
    mainLayout->addWidget(listView);
    mainLayout->addWidget(disabledView);

    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainWidget->setLayout(mainLayout);
    mainWidget->setMinimumSize(300, 200);
}


void MainWindow::addToView(Rom *currentRom, int count)
{
    QString visibleLayout = SETTINGS.value("View/layout", "none").toString();

    if (visibleLayout == "table")
        tableView->addToTableView(currentRom);
    else if (visibleLayout == "grid")
        gridView->addToGridView(currentRom, count, false);
    else if (visibleLayout == "list")
        listView->addToListView(currentRom, count, false);
}


void MainWindow::autoloadSettings()
{
    QString emulatorPath = SETTINGS.value("Paths/mupen64plus", "").toString();
    QString dataPath = SETTINGS.value("Paths/data", "").toString();
    QString pluginPath = SETTINGS.value("Paths/plugins", "").toString();

    if (emulatorPath == "" && dataPath == "" && pluginPath == "") {
#ifdef OS_LINUX_OR_BSD
        //If user has not entered any settings, check common locations for them
        QStringList emulatorCheck, dataCheck, pluginCheck;

        emulatorCheck << "/usr/bin/mupen64plus"
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


        foreach (QString check, emulatorCheck)
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

    checkConfigLocation();
}


void MainWindow::checkConfigLocation()
{
    //Check default location for mupen64plus.cfg in case user wants to use editor
    QString configPath = SETTINGS.value("Paths/config", "").toString();

    if (configPath == "") {
        QString homeDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();

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

    tableView->saveColumnWidths();

    event->accept();
}


void MainWindow::createMenu()
{
    menuBar = new QMenuBar(this);


    //File
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

    connect(openAction, SIGNAL(triggered()), this, SLOT(openRom()));
    connect(refreshAction, SIGNAL(triggered()), romCollection, SLOT(addRoms()));
    connect(downloadAction, SIGNAL(triggered()), this, SLOT(openDownloader()));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(openDeleteDialog()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));


    //Emulation
    emulationMenu = new QMenu(tr("&Emulation"), this);
    startAction = emulationMenu->addAction(tr("&Start"));
    stopAction = emulationMenu->addAction(tr("St&op"));
    emulationMenu->addSeparator();
    logAction = emulationMenu->addAction(tr("View Log"));

    startAction->setIcon(QIcon::fromTheme("media-playback-start"));
    stopAction->setIcon(QIcon::fromTheme("media-playback-stop"));

    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    menuBar->addMenu(emulationMenu);

    connect(startAction, SIGNAL(triggered()), this, SLOT(launchRomFromMenu()));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stopEmulator()));
    connect(logAction, SIGNAL(triggered()), this, SLOT(openLog()));


    //Settings
    settingsMenu = new QMenu(tr("&Settings"), this);
    editorAction = settingsMenu->addAction(tr("Edit mupen64plus.cfg..."));
    settingsMenu->addSeparator();
    configureGameAction = settingsMenu->addAction(tr("Configure &Game..."));
#ifndef Q_OS_OSX //OSX does not show the quit action so the separator is unneeded
    settingsMenu->addSeparator();
#endif
    configureAction = settingsMenu->addAction(tr("&Configure..."));
    configureAction->setIcon(QIcon::fromTheme("preferences-other"));

    configureGameAction->setEnabled(false);

    menuBar->addMenu(settingsMenu);

    connect(editorAction, SIGNAL(triggered()), this, SLOT(openEditor()));
    connect(configureGameAction, SIGNAL(triggered()), this, SLOT(openGameSettings()));
    connect(configureAction, SIGNAL(triggered()), this, SLOT(openSettings()));


    //View
    viewMenu = new QMenu(tr("&View"), this);
    layoutMenu = viewMenu->addMenu(tr("&Layout"));
    layoutGroup = new QActionGroup(this);

    QList<QStringList> layouts;
    layouts << (QStringList() << tr("None")       << "none")
            << (QStringList() << tr("Table View") << "table")
            << (QStringList() << tr("Grid View")  << "grid")
            << (QStringList() << tr("List View")  << "list");

    QString layoutValue = SETTINGS.value("View/layout", "none").toString();

    foreach (QStringList layoutName, layouts)
    {
        QAction *layoutItem = layoutMenu->addAction(layoutName.at(0));
        layoutItem->setData(layoutName.at(1));
        layoutItem->setCheckable(true);
        layoutGroup->addAction(layoutItem);

        //Only enable layout changes when emulator is not running
        menuEnable << layoutItem;

        if(layoutValue == layoutName.at(1))
            layoutItem->setChecked(true);
    }

    viewMenu->addSeparator();

    //OSX El Capitan adds it's own full-screen option
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    if (!(QOperatingSystemVersion::current() >= QOperatingSystemVersion::OSXElCapitan &&
            QOperatingSystemVersion::currentType() == QOperatingSystemVersion::MacOS))
#else
    if (QSysInfo::macVersion() < QSysInfo::MV_ELCAPITAN || QSysInfo::macVersion() == QSysInfo::MV_None)
#endif
        fullScreenAction = viewMenu->addAction(tr("&Full-screen"));
    else
        fullScreenAction = new QAction(this);

    fullScreenAction->setCheckable(true);

    if (SETTINGS.value("View/fullscreen", "") == "true")
        fullScreenAction->setChecked(true);

    menuBar->addMenu(viewMenu);

    connect(layoutGroup, SIGNAL(triggered(QAction*)), this, SLOT(updateLayoutSetting()));
    connect(fullScreenAction, SIGNAL(triggered()), this, SLOT(updateFullScreenMode()));


    //Help
    helpMenu = new QMenu(tr("&Help"), this);
    documentationAction = helpMenu->addAction(tr("Documentation"));
    mupenDocsAction = helpMenu->addAction(tr("Mupen64Plus Docs"));
    helpMenu->addSeparator();
    controlInfoAction = helpMenu->addAction(tr("Default Controls"));
    helpMenu->addSeparator();
    aboutAction = helpMenu->addAction(tr("&About"));
    aboutAction->setIcon(QIcon::fromTheme("help-about"));
    menuBar->addMenu(helpMenu);

    connect(documentationAction, SIGNAL(triggered()), this, SLOT(openDocumentation()));
    connect(mupenDocsAction, SIGNAL(triggered()), this, SLOT(openMupenDocs()));
    connect(controlInfoAction, SIGNAL(triggered()), this, SLOT(openDefaultControls()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(openAbout()));


    //Create list of actions that are enabled only when emulator is not running
    menuEnable << startAction
               << logAction
               << openAction
               << refreshAction
               << downloadAction
               << deleteAction
               << configureAction
               << configureGameAction
               << editorAction
               << quitAction;

    //Create list of actions that are disabled when emulator is not running
    menuDisable << stopAction;

    //Create list of actions that are only active when a ROM is selected
    menuRomSelected << startAction
                    << deleteAction
                    << downloadAction
                    << configureGameAction;
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
    emptyIcon->setPixmap(QPixmap(":/images/"+ParentNameLower+".png"));

    emptyLayout->addWidget(emptyIcon, 1, 1);
    emptyLayout->setColumnStretch(0, 1);
    emptyLayout->setColumnStretch(2, 1);
    emptyLayout->setRowStretch(0, 1);
    emptyLayout->setRowStretch(2, 1);

    emptyView->setLayout(emptyLayout);


    //Create table view
    tableView = new TableView(this);
    connect(tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(enableButtons()));
    connect(tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(launchRomFromTable()));
    connect(tableView, SIGNAL(tableActive()), this, SLOT(enableButtons()));
    connect(tableView, SIGNAL(enterPressed()), this, SLOT(launchRomFromTable()));


    //Create grid view
    gridView = new GridView(this);
    connect(gridView, SIGNAL(gridItemSelected(bool)), this, SLOT(toggleMenus(bool)));


    //Create list view
    listView = new ListView(this);
    connect(listView, SIGNAL(listItemSelected(bool)), this, SLOT(toggleMenus(bool)));


    //Create disabled view
    disabledView = new QWidget(this);
    disabledView->setHidden(true);

    disabledLayout = new QVBoxLayout(disabledView);
    disabledLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    disabledView->setLayout(disabledLayout);

    QString disabledText = QString(tr("Add a directory containing ROMs under "))
                         + tr("Settings->Configure->Paths to use this view.");
    disabledLabel = new QLabel(disabledText, disabledView);
    disabledLabel->setWordWrap(true);
    disabledLabel->setAlignment(Qt::AlignCenter);
    disabledLayout->addWidget(disabledLabel);


    showActiveView();
}


void MainWindow::disableButtons()
{
    toggleMenus(false);
}


void MainWindow::disableViews(bool imageUpdated)
{
    QString visibleLayout = SETTINGS.value("View/layout", "none").toString();

    //Save position in current layout
    if (visibleLayout == "table")
        tableView->saveTablePosition();
    else if (visibleLayout == "grid")
        gridView->saveGridPosition();
    else if (visibleLayout == "list")
        listView->saveListPosition();

    resetLayouts(imageUpdated);
    tableView->clear();

    tableView->setEnabled(false);
    gridView->setEnabled(false);
    listView->setEnabled(false);

    foreach (QAction *next, menuRomSelected)
        next->setEnabled(false);
}


void MainWindow::enableButtons()
{
    toggleMenus(true);
}


void MainWindow::enableViews(int romCount, bool cached)
{
    QString visibleLayout = SETTINGS.value("View/layout", "none").toString();

    if (romCount != 0) { //else no ROMs, so leave views disabled
        QStringList tableVisible = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");

        if (tableVisible.join("") != "")
            tableView->setEnabled(true);

        gridView->setEnabled(true);
        listView->setEnabled(true);

        if (visibleLayout == "table")
            tableView->setFocus();
        else if (visibleLayout == "grid")
            gridView->setFocus();
        else if (visibleLayout == "list")
            listView->setFocus();

        //Check if disabled view is showing. If it is, re-enabled the selected view
        if (!disabledView->isHidden()) {
            disabledView->setHidden(true);
            showActiveView();
        }

        if (cached) {
            QTimer *timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(0);
            timer->start();

            if (visibleLayout == "table")
                connect(timer, SIGNAL(timeout()), tableView, SLOT(setTablePosition()));
            else if (visibleLayout == "grid")
                connect(timer, SIGNAL(timeout()), gridView, SLOT(setGridPosition()));
            else if (visibleLayout == "list")
                connect(timer, SIGNAL(timeout()), listView, SLOT(setListPosition()));
        }
    } else {
        if (visibleLayout != "none") {
            tableView->setHidden(true);
            gridView->setHidden(true);
            listView->setHidden(true);
            disabledView->setHidden(false);
        }
    }
}


bool MainWindow::eventFilter(QObject*, QEvent *event)
{
    //Show menu bar if mouse is at top of screen in full-screen mode
    if (event->type() == QEvent::HoverMove && isFullScreen()) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        int mousePos = mouseEvent->pos().y();

        if (mousePos < 5)
            showMenuBar(true);
        if (mousePos > 30)
            showMenuBar(false);
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        //Exit fullscreen mode if Esc key is pressed
        if (keyEvent->key() == Qt::Key_Escape && isFullScreen())
            updateFullScreenMode();
    }

    //OSX El Capitan adds it's own full-screen option, so handle the event change here
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::OSXElCapitan &&
            QOperatingSystemVersion::currentType() == QOperatingSystemVersion::MacOS) {
#else
    if (QSysInfo::macVersion() >= QSysInfo::MV_ELCAPITAN && QSysInfo::macVersion() != QSysInfo::MV_None) {
#endif
        if (event->type() == QEvent::WindowStateChange) {
            QWindowStateChangeEvent *windowEvent = static_cast<QWindowStateChangeEvent*>(event);

            if (windowEvent->oldState() == Qt::WindowNoState) {
                SETTINGS.setValue("View/fullscreen", true);
                tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                gridView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            } else {
                SETTINGS.setValue("View/fullscreen", "");
                tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                gridView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            }
        }
    }

    return false;
}


QString MainWindow::getCurrentRomInfoFromView(QString infoName)
{
    QString visibleLayout = SETTINGS.value("View/layout", "none").toString();

    if (visibleLayout == "table")
        return tableView->getCurrentRomInfo(infoName);
    else if (visibleLayout == "grid" && gridView->hasSelectedRom())
        return gridView->getCurrentRomInfo(infoName);
    else if (visibleLayout == "list" && listView->hasSelectedRom())
        return listView->getCurrentRomInfo(infoName);

    return "";
}


void MainWindow::launchRomFromMenu()
{
    QString visibleLayout = layoutGroup->checkedAction()->data().toString();

    if (visibleLayout == "table")
        launchRomFromTable();
    else if (visibleLayout == "grid")
        launchRomFromWidget(gridView->getCurrentRomWidget());
    else if (visibleLayout == "list")
        launchRomFromWidget(listView->getCurrentRomWidget());
}


void MainWindow::launchRomFromTable()
{
    QString romFileName = tableView->getCurrentRomInfo("fileName");
    QString romDirName = tableView->getCurrentRomInfo("dirName");
    QString zipFileName = tableView->getCurrentRomInfo("zipFile");
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

void MainWindow::openDefaultControls()
{
    ControlInfo controlInfo(this);
    controlInfo.exec();
}


void MainWindow::openDeleteDialog()
{
    scraper = new TheGamesDBScraper(this);
    scraper->deleteGameInfo(getCurrentRomInfoFromView("fileName"), getCurrentRomInfoFromView("romMD5"));
    delete scraper;

    romCollection->cachedRoms();
}


void MainWindow::openDocumentation()
{
    QDesktopServices::openUrl(QUrl("https://github.com/dh4/mupen64plus-qt#table-of-contents"));
}


void MainWindow::openDownloader()
{
    DownloadDialog downloadDialog(getCurrentRomInfoFromView("fileName"),
                                  getCurrentRomInfoFromView("search"),
                                  getCurrentRomInfoFromView("romMD5"),
                                  this);
    downloadDialog.exec();

    romCollection->cachedRoms();
}


void MainWindow::openEditor()
{
    checkConfigLocation();

    QString configPath = SETTINGS.value("Paths/config", "").toString();
    QDir configDir = QDir(configPath);
    QString configFile = configDir.absoluteFilePath("mupen64plus.cfg");
    QFile config(configFile);

    if (configPath == "" || !config.exists()) {
        QMessageBox::information(this, tr("Not Found"), QString(tr("Editor requires config directory to be "))
                                 + tr("set to a directory with mupen64plus.cfg.") + "<br /><br />"
                                 + tr("See here for the default config location:") + "<br />"
                                 + "<a href=\"https://mupen64plus.org/wiki/index.php?title=FileLocations\">"
                                 + "https://mupen64plus.org/wiki/index.php?title=FileLocations</a>");
    } else {
        ConfigEditor configEditor(configFile, this);
        configEditor.exec();
    }
}


void MainWindow::openGameSettings()
{
    GameSettingsDialog gameSettingsDialog(getCurrentRomInfoFromView("fileName"), this);
    gameSettingsDialog.exec();
}


void MainWindow::openLog()
{
    if (emulation->lastOutput == "") {
        QMessageBox::information(this, tr("No Output"),
            tr("There is no log. Either <ParentName> has not yet run or there was no output from the last run.")
            .replace("<ParentName>",ParentName));
    } else {
        LogDialog logDialog(emulation->lastOutput, this);
        logDialog.exec();
    }
}


void MainWindow::openMupenDocs()
{
    QDesktopServices::openUrl(QUrl("https://mupen64plus.org/docs/"));
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

    gridView->setGridBackground();
    listView->setListBackground();
    toggleMenus(true);
}


void MainWindow::openRom()
{
    QString filter = "N64 ROMs (";
    foreach (QString type, romCollection->getFileTypes(true)) filter += type + " ";
    filter += ");;" + tr("All Files") + " (*)";

    QString searchPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    if (romCollection->romPaths.count() > 0)
        searchPath = romCollection->romPaths.at(0);

    openPath = QFileDialog::getOpenFileName(this, tr("Open ROM File"), searchPath, filter);
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
    zipButtonBox->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);

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
    tableView->resetView(imageUpdated);
    gridView->resetView();
    listView->resetView();
}


void MainWindow::showActiveView()
{
    QString visibleLayout = SETTINGS.value("View/layout", "none").toString();

    if (visibleLayout == "table")
        tableView->setHidden(false);
    else if (visibleLayout == "grid")
        gridView->setHidden(false);
    else if (visibleLayout == "list")
        listView->setHidden(false);
    else
        emptyView->setHidden(false);
}


void MainWindow::showMenuBar(bool mouseAtTop)
{
    menuBar->setHidden(!mouseAtTop);
}



void MainWindow::showRomMenu(const QPoint &pos)
{
    QMenu *contextMenu = new QMenu(this);

    QAction *contextStartAction = contextMenu->addAction(tr("&Start"));
    contextStartAction->setIcon(QIcon::fromTheme("media-playback-start"));
    contextMenu->addSeparator();
    QAction *contextConfigureGameAction = contextMenu->addAction(tr("Configure &Game..."));

    connect(contextStartAction, SIGNAL(triggered()), this, SLOT(launchRomFromMenu()));
    connect(contextConfigureGameAction, SIGNAL(triggered()), this, SLOT(openGameSettings()));

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true") {
        contextMenu->addSeparator();
        QAction *contextDownloadAction = contextMenu->addAction(tr("&Download/Update Info..."));
        QAction *contextDeleteAction = contextMenu->addAction(tr("D&elete Current Info..."));

        connect(contextDownloadAction, SIGNAL(triggered()), this, SLOT(openDownloader()));
        connect(contextDeleteAction, SIGNAL(triggered()), this, SLOT(openDeleteDialog()));
    }


    QWidget *activeWidget = new QWidget(this);
    QString visibleLayout = SETTINGS.value("View/layout", "none").toString();

    if (visibleLayout == "table")
        activeWidget = tableView->viewport();
    else if (visibleLayout == "grid")
        activeWidget = gridView->getCurrentRomWidget();
    else if (visibleLayout == "list")
        activeWidget = listView->getCurrentRomWidget();

    contextMenu->exec(activeWidget->mapToGlobal(pos));
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

    if (!tableView->hasSelectedRom() &&
        !gridView->hasSelectedRom() &&
        !listView->hasSelectedRom()
    ) {
        foreach (QAction *next, menuRomSelected)
            next->setEnabled(false);
    }

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "") {
        downloadAction->setEnabled(false);
        deleteAction->setEnabled(false);
    }
}


void MainWindow::updateFullScreenMode()
{
    if (isFullScreen()) {
        fullScreenAction->setChecked(false);
        SETTINGS.setValue("View/fullscreen", "");

        menuBar->setHidden(false);
        tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        gridView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        showNormal();
    } else {
        fullScreenAction->setChecked(true);
        SETTINGS.setValue("View/fullscreen", true);

        menuBar->setHidden(true);
        tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        gridView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        showFullScreen();
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
    disabledView->setHidden(true);

    int romCount = romCollection->cachedRoms();

    if (romCount > 0 || visibleLayout == "none")
        showActiveView();

    //View was updated so no ROM will be selected. Update menu items accordingly
    foreach (QAction *next, menuRomSelected)
        next->setEnabled(false);
}
