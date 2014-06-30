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
    layout->addWidget(romTree);
    layout->setMargin(0);

    widget->setLayout(layout);
    widget->setMinimumSize(300, 200);
}


void Mupen64PlusQt::addRoms()
{
    SETTINGS.setValue("ROMs/cache", "");
    QStringList visible = SETTINGS.value("ROMs/columns", "Filename|Size").toString().split("|");
    resetRomTreeLayout(visible);

    romTree->setEnabled(false);
    romTree->clear();
    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    if (romPath != "") {
        if (romDir.exists()) {
            QStringList files = romDir.entryList(QStringList() << "*.z64" << "*.n64" << "*.v64",
                                                 QDir::Files | QDir::NoSymLinks);

            if (files.size() > 0) {
                QProgressDialog progress("Loading ROMs...", "Cancel", 0, files.size(), this);
#if QT_VERSION >= 0x050000
                progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowCloseButtonHint);
                progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowMinimizeButtonHint);
                progress.setWindowFlags(progress.windowFlags() & ~Qt::WindowContextHelpButtonHint);
#else
                progress.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
#endif
                progress.setCancelButton(0);
                progress.show();
                progress.setWindowModality(Qt::WindowModal);

                int count = 0;
                foreach (QString fileName, files)
                {
                    QFile file(romDir.absoluteFilePath(fileName));

                    file.open(QIODevice::ReadOnly);
                    romData = new QByteArray(file.readAll());
                    file.close();

                    *romData = byteswap(*romData);

                    QString internalName = QString(romData->mid(32, 20)).trimmed();

                    QString romMD5 = QString(QCryptographicHash::hash(*romData,
                                             QCryptographicHash::Md5).toHex());

                    delete romData;

                    if (visible.join("") != "") //Otherwise no columns, so don't bother populating
                        addToRomTree(fileName, romMD5, internalName, visible);

                    QString currentSetting = SETTINGS.value("ROMs/cache", "").toString();
                    QString newSetting = currentSetting
                                         + fileName + "|"
                                         + internalName + "|"
                                         + romMD5 + "||";

                    SETTINGS.setValue("ROMs/cache", newSetting);

                    count++;
                    progress.setValue(count);
                }
            } else {
            QMessageBox::warning(this, "Warning", "No ROMs found.");
            }
        } else {
            QMessageBox::warning(this, "Warning", "Failed to open ROM directory.");
        }
    }

    if (visible.join("") != "")
        romTree->setEnabled(true);
}



void Mupen64PlusQt::addToRomTree(QString fileName, QString romMD5, QString internalName, QStringList visible)
{
    QFile file(romDir.absoluteFilePath(fileName));
    qint64 size = QFileInfo(file).size();

    QString dataPath = SETTINGS.value("Paths/data", "").toString();
    dataDir = QDir(dataPath);
    QString catalogFile = dataDir.absoluteFilePath("mupen64plus.ini");

    bool getGoodName = false;
    if (QFileInfo(catalogFile).exists()) {
        romCatalog = new QSettings(catalogFile, QSettings::IniFormat, this);
        getGoodName = true;
    }

    QString goodName = "Requires data directory";
    QString CRC1 = "";
    QString CRC2 = "";
    QString players = "";
    QString saveType = "";
    QString rumble = "";

    romMD5 = romMD5.toUpper();

    if (getGoodName) {
        //Join GoodName on ", ", otherwise entries with a comma won't show
        QVariant goodNameVariant = romCatalog->value(romMD5+"/GoodName","Unknown ROM");
        goodName = goodNameVariant.toStringList().join(", ");

        QStringList CRC = romCatalog->value(romMD5+"/CRC","").toString().split(" ");

        if (CRC.size() == 2) {
            CRC1 = CRC[0];
            CRC2 = CRC[1];
        }

        QString newMD5 = romCatalog->value(romMD5+"/RefMD5","").toString();
        if (newMD5 == "")
            newMD5 = romMD5;

        players = romCatalog->value(newMD5+"/Players","").toString();
        saveType = romCatalog->value(newMD5+"/SaveType","").toString();
        rumble = romCatalog->value(newMD5+"/Rumble","").toString();
    }

    fileItem = new TreeWidgetItem(romTree);
    fileItem->setText(0, fileName);

    int i = 1;
    foreach (QString current, visible)
    {
        if (current == "GoodName") {
            fileItem->setText(i, goodName);
            if (goodName == "Unknown ROM" || goodName == "Requires data directory")
                fileItem->setForeground(i, QBrush(Qt::gray));
        }
        else if (current == "Filename") {
            fileItem->setText(i, QFileInfo(file).completeBaseName());
        }
        else if (current == "Filename (extension)") {
            fileItem->setText(i, fileName);
        }
        else if (current == "Internal Name") {
            fileItem->setText(i, internalName);
        }
        else if (current == "Size") {
            fileItem->setText(i, tr("%1 MB").arg(int((size + 1023) / 1024 / 1024)));
            fileItem->setData(i, Qt::UserRole, (int)size);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (current == "MD5") {
            fileItem->setText(i, romMD5.toLower());
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        }
        else if (current == "CRC1") {
            fileItem->setText(i, CRC1.toLower());
        }
        else if (current == "CRC2") {
            fileItem->setText(i, CRC2.toLower());
        }
        else if (current == "Players") {
            fileItem->setText(i, players);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (current == "Rumble") {
            fileItem->setText(i, rumble);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else if (current == "Save Type") {
            fileItem->setText(i, saveType);
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
        }
        else //Invalid column name in config file
            fileItem->setText(i, "");

        i++;
    }

    romTree->addTopLevelItem(fileItem);
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


void Mupen64PlusQt::cachedRoms()
{
    romTree->setEnabled(false);
    romTree->clear();
    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    QStringList visible = SETTINGS.value("ROMs/columns", "Filename|Size").toString().split("|");
    resetRomTreeLayout(visible);


    if (visible.join("") != "") { //Otherwise no columns, so don't bother populating

        QString cache = SETTINGS.value("ROMs/cache", "").toString();
        QStringList cachedRoms = cache.split("||");

        foreach (QString current, cachedRoms)
        {
            QStringList romInfo = current.split("|");

            if (romInfo.size() == 3) {
                addToRomTree(romInfo[0], romInfo[2], romInfo[1], visible);
            }
        }

        romTree->setEnabled(true);
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
#ifndef Q_OS_OSX //OSX does not show the quit action so the separator is unneeded
    fileMenu->addSeparator();
#endif
    quitAction = fileMenu->addAction(tr("&Quit"));

    openAction->setIcon(QIcon::fromTheme("document-open"));
    refreshAction->setIcon(QIcon::fromTheme("view-refresh"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));

    menuBar->addMenu(fileMenu);


    emulationMenu = new QMenu(tr("&Emulation"), this);
    startAction = emulationMenu->addAction(tr("&Start"));
    stopAction = emulationMenu->addAction(tr("St&op"));

    startAction->setIcon(QIcon::fromTheme("media-playback-start"));
    stopAction->setIcon(QIcon::fromTheme("media-playback-stop"));

    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    menuBar->addMenu(emulationMenu);


    settingsMenu = new QMenu(tr("&Settings"), this);
    pathsAction = settingsMenu->addAction(tr("&Paths"));
    emulationAction = settingsMenu->addAction(tr("&Emulation"));
    graphicsAction = settingsMenu->addAction(tr("&Graphics"));
    pluginsAction = settingsMenu->addAction(tr("P&lugins"));
    columnsAction = settingsMenu->addAction(tr("&Columns"));

    pathsAction->setIcon(QIcon::fromTheme("preferences-other"));
    emulationAction->setIcon(QIcon::fromTheme("preferences-system"));
    graphicsAction->setIcon(QIcon::fromTheme("video-display"));
    if (QIcon::hasThemeIcon("input-gaming"))
        pluginsAction->setIcon(QIcon::fromTheme("input-gaming"));
    else
        pluginsAction->setIcon(QIcon::fromTheme("preferences-other"));
    columnsAction->setIcon(QIcon::fromTheme("tab-new"));

    menuBar->addMenu(settingsMenu);


    helpMenu = new QMenu(tr("&Help"), this);
    aboutAction = helpMenu->addAction(tr("&About"));
    aboutAction->setIcon(QIcon::fromTheme("help-about"));
    menuBar->addMenu(helpMenu);


    //Create list of actions that are enabled only when Mupen64Plus is not running
    menuEnable << startAction
               << openAction
               << refreshAction
               << emulationAction
               << graphicsAction
               << pathsAction
               << pluginsAction
               << columnsAction
               << quitAction;

    //Create list of actions that are disabled when Mupen64Plus is not running
    menuDisable << stopAction;

    connect(openAction, SIGNAL(triggered()), this, SLOT(openRom()));
    connect(refreshAction, SIGNAL(triggered()), this, SLOT(addRoms()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(startAction, SIGNAL(triggered()), this, SLOT(runEmulatorFromRomTree()));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stopEmulator()));
    connect(pathsAction, SIGNAL(triggered()), this, SLOT(openPaths()));
    connect(emulationAction, SIGNAL(triggered()), this, SLOT(openEmulation()));
    connect(graphicsAction, SIGNAL(triggered()), this, SLOT(openGraphics()));
    connect(pluginsAction, SIGNAL(triggered()), this, SLOT(openPlugins()));
    connect(columnsAction, SIGNAL(triggered()), this, SLOT(openColumns()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(openAbout()));
}


void Mupen64PlusQt::createRomView()
{
    romTree = new QTreeWidget(this);
    romTree->setWordWrap(false);
    romTree->setAllColumnsShowFocus(true);
    romTree->setRootIsDecorated(false);
    romTree->setSortingEnabled(true);
    romTree->setStyleSheet("QTreeView { border: none; } QTreeView::item { height: 25px; }");

    headerView = new QHeaderView(Qt::Horizontal, this);
    romTree->setHeader(headerView);

    cachedRoms();

    connect(romTree, SIGNAL(clicked(QModelIndex)), this, SLOT(enableButtons()));
    connect(romTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(runEmulatorFromRomTree()));
    connect(headerView, SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this, SLOT(saveSortOrder(int,Qt::SortOrder)));
}


void Mupen64PlusQt::enableButtons()
{
    toggleMenus(true);
}


void Mupen64PlusQt::openAbout()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}


void Mupen64PlusQt::openColumns()
{
    openOptions(4);
}


void Mupen64PlusQt::openEmulation()
{
    openOptions(1);
}


void Mupen64PlusQt::openGraphics()
{
    openOptions(2);
}


void Mupen64PlusQt::openOptions(int activeTab) {

    QString columnsBefore = SETTINGS.value("ROMs/columns", "Filename|Size").toString();

    SettingsDialog settingsDialog(this, activeTab);
    settingsDialog.exec();

    QString columnsAfter = SETTINGS.value("ROMs/columns", "Filename|Size").toString();

    //Reset columns widths if user has selected different columns to display
    if (columnsBefore != columnsAfter) {
        SETTINGS.setValue("ROMs/width", "");
        romTree->setColumnCount(1);
        romTree->setHeaderLabels(QStringList(""));
    }

    QString romSave = SETTINGS.value("Paths/roms","").toString();
    if (romPath != romSave) {
        romPath = romSave;
        romDir = QDir(romPath);
        addRoms();
    } else {
        cachedRoms();
    }
}


void Mupen64PlusQt::openPaths()
{
    openOptions(0);
}


void Mupen64PlusQt::openPlugins()
{
    openOptions(3);
}


void Mupen64PlusQt::openRom()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open ROM File"), romPath,
                                                tr("N64 ROMs (*.z64 *.n64 *.v64);;All Files (*)"));
    if (path != "")
        runEmulator(path);
}


void Mupen64PlusQt::resetRomTreeLayout(QStringList visible)
{
    saveColumnWidths();
    QStringList widths = SETTINGS.value("ROMs/width", "").toString().split("|");

    headerLabels.clear();
    headerLabels << "" << visible;
    romTree->setColumnCount(headerLabels.size());
    romTree->setHeaderLabels(headerLabels);
    headerView->setSortIndicatorShown(false);

    QStringList sort = SETTINGS.value("ROMs/sort", "").toString().split("|");
    if (sort.size() == 2) {
        if (sort[1] == "descending")
            headerView->setSortIndicator(visible.indexOf(sort[0]) + 1, Qt::DescendingOrder);
        else
            headerView->setSortIndicator(visible.indexOf(sort[0]) + 1, Qt::AscendingOrder);
    }

    romTree->setColumnHidden(0, true); //Hidden filename for launching emulator


    int i = 1;
    foreach (QString current, visible)
    {
        if (i == 1) {
            if (SETTINGS.value("ROMs/stretchfirstcolumn", "true") == "true") {
#if QT_VERSION >= 0x050000
                romTree->header()->setSectionResizeMode(i, QHeaderView::Stretch);
#else
                romTree->header()->setResizeMode(i, QHeaderView::Stretch);
#endif
            } else {
#if QT_VERSION >= 0x050000
                romTree->header()->setSectionResizeMode(i, QHeaderView::Interactive);
#else
                romTree->header()->setResizeMode(i, QHeaderView::Interactive);
#endif
            }
        }

        if (widths.size() == visible.size()) {
            romTree->setColumnWidth(i, widths[i - 1].toInt());
        } else {
            if (current == "GoodName" || current.left(8) == "Filename")
                romTree->setColumnWidth(i, 300);
            else if (current == "MD5")
                romTree->setColumnWidth(i, 250);
            else if (current == "Internal Name")
                romTree->setColumnWidth(i, 200);
            else if (current == "Save Type")
                romTree->setColumnWidth(i, 100);
            else if (current == "CRC1" || current == "CRC2")
                romTree->setColumnWidth(i, 90);
            else if (current == "Size" || current == "Rumble" || current == "Players")
                romTree->setColumnWidth(i, 75);
        }

        i++;
    }
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

    if(!QFileInfo(mupen64File).exists()) {
        QMessageBox::warning(this, "Warning", "Mupen64Plus executable not found.");
        return;
    }

    if(!QFileInfo(romFile).exists()) {
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
    connect(mupen64proc, SIGNAL(finished(int)), this, SLOT(enableButtons()));
    connect(mupen64proc, SIGNAL(finished(int)), this, SLOT(checkStatus(int)));

    mupen64proc->setWorkingDirectory(QFileInfo(mupen64File).dir().canonicalPath());
    mupen64proc->start(mupen64Path, args);
}


void Mupen64PlusQt::runEmulatorFromRomTree()
{
    QString completeRomFileName = QVariant(romTree->currentItem()->data(0, 0)).toString();
    QString completeRomPath = romDir.absoluteFilePath(completeRomFileName);
    runEmulator(completeRomPath);
}


void Mupen64PlusQt::saveColumnWidths()
{
    QStringList widths;

    for (int i = 1; i < romTree->columnCount(); i++)
    {
        widths << QString::number(romTree->columnWidth(i));
    }

    if (widths.size() > 0)
        SETTINGS.setValue("ROMs/width", widths.join("|"));
}


void Mupen64PlusQt::saveSortOrder(int column, Qt::SortOrder order)
{
    QString columnName = headerLabels.value(column);

    if (order == Qt::DescendingOrder)
        SETTINGS.setValue("ROMs/sort", columnName + "|descending");
    else
        SETTINGS.setValue("ROMs/sort", columnName + "|ascending");
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

    if (romTree->currentItem() == NULL)
        startAction->setEnabled(false);
}
