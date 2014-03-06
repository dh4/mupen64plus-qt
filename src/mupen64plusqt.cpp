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
#include <QDebug>


Mupen64PlusQt::Mupen64PlusQt(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("Mupen64Plus-Qt"));
    setWindowIcon(QIcon(":/images/mupen64.png"));

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
    romTree->clear();
    startAction->setEnabled(false);
    stopAction->setEnabled(false);

    if (romPath != "") {
        if (romDir.exists()) {
            QStringList files = romDir.entryList(QStringList() << "*.z64" << "*.n64" << "*.v64",
                                                 QDir::Files | QDir::NoSymLinks);

            if (files.size() > 0) {
                for(QStringList::Iterator it = files.begin(); it != files.end(); ++it)
                {
                    QFile file(romDir.absoluteFilePath(*it));
                    qint64 size = QFileInfo(file).size();

                    fileItem = new QTreeWidgetItem;
                    fileItem->setText(0, QFileInfo(file).fileName());
                    fileItem->setText(1, tr("%1 MB").arg(int((size + 1023) / 1024 / 1024)));
                    fileItem->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);

                    romTree->addTopLevelItem(fileItem);
                }
            } else {
            QMessageBox::warning(this, "Warning", "No ROMs found.");
            }
        } else {
            QMessageBox::warning(this, "Warning", "Failed to open ROM directory.");
        }
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
    SETTINGS.setValue("Geometry/windowx", x());
    SETTINGS.setValue("Geometry/windowy", y());
    SETTINGS.setValue("Geometry/width", width());
    SETTINGS.setValue("Geometry/height", height());
    if (isMaximized())
        SETTINGS.setValue("Geometry/maximized", true);
    else
        SETTINGS.setValue("Geometry/maximized", "");
    event->accept();
}


void Mupen64PlusQt::createMenu()
{
    menuBar = new QMenuBar(this);


    fileMenu = new QMenu(tr("&File"), this);
    openAction = fileMenu->addAction(tr("&Open ROM..."));
    fileMenu->addSeparator();
    refreshAction = fileMenu->addAction(tr("&Refresh List"));
    fileMenu->addSeparator();
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
    graphicsAction = settingsMenu->addAction(tr("&Graphics"));
    pluginsAction = settingsMenu->addAction(tr("&Plugins"));

    pathsAction->setIcon(QIcon::fromTheme("preferences-other"));
    graphicsAction->setIcon(QIcon::fromTheme("video-display"));
    pluginsAction->setIcon(QIcon::fromTheme("input-gaming"));

    menuBar->addMenu(settingsMenu);


    helpMenu = new QMenu(tr("&Help"), this);
    aboutAction = helpMenu->addAction(tr("&About"));
    aboutAction->setIcon(QIcon::fromTheme("help-about"));
    menuBar->addMenu(helpMenu);


    //Create list of actions that are enabled only when Mupen64 is not running
    menuEnable << startAction
               << openAction
               << refreshAction
               << graphicsAction
               << pathsAction
               << pluginsAction;

    //Create list of actions that are disabled when Mupen64 is not running
    menuDisable << stopAction;

    connect(openAction, SIGNAL(triggered()), this, SLOT(openRom()));
    connect(refreshAction, SIGNAL(triggered()), this, SLOT(addRoms()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(startAction, SIGNAL(triggered()), this, SLOT(runEmulatorFromRomTree()));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stopEmulator()));
    connect(pathsAction, SIGNAL(triggered()), this, SLOT(openPaths()));
    connect(graphicsAction, SIGNAL(triggered()), this, SLOT(openGraphics()));
    connect(pluginsAction, SIGNAL(triggered()), this, SLOT(openPlugins()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(openAbout()));
}


void Mupen64PlusQt::createRomView()
{
    romTree = new QTreeWidget(this);
    romTree->setColumnCount(2);
    romTree->setWordWrap(false);
    romTree->setAllColumnsShowFocus(true);
    romTree->setRootIsDecorated(false);
    romTree->setStyleSheet("QTreeView { border: none; } QTreeView::item { height: 25px; }");

    headerItem = new QTreeWidgetItem;
    headerItem->setText(0, tr("ROM"));
    headerItem->setText(1, tr("Size"));
    headerItem->setTextAlignment(0, Qt::AlignCenter | Qt::AlignVCenter);
    headerItem->setTextAlignment(1, Qt::AlignCenter | Qt::AlignVCenter);
    romTree->setHeaderItem(headerItem);

    romTree->header()->setStretchLastSection(false);
#if QT_VERSION >= 0x050000
    romTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
    romTree->header()->setResizeMode(0, QHeaderView::Stretch);
#endif
    romTree->setColumnWidth(1, 100);

    addRoms();

    connect(romTree, SIGNAL(clicked(QModelIndex)), this, SLOT(enableButtons()));
    connect(romTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(runEmulatorFromRomTree()));
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


void Mupen64PlusQt::openGraphics()
{
    openOptions(1);
}


void Mupen64PlusQt::openOptions(int activeTab) {
    SettingsDialog settingsDialog(this, activeTab);
    settingsDialog.exec();

    if (romPath != SETTINGS.value("Paths/roms","").toString()) {
        romPath = SETTINGS.value("Paths/roms","").toString();
        romDir = QDir(romPath);
        addRoms();
    }
}


void Mupen64PlusQt::openPaths()
{
    openOptions(0);
}


void Mupen64PlusQt::openPlugins()
{
    openOptions(2);
}


void Mupen64PlusQt::openRom()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open ROM File"), romPath,
                                                tr("N64 ROMs (*.z64 *.n64 *.v64);;All Files (*)"));
    if (path != "")
        runEmulator(path);
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
    args << "--nosaveoptions";

    if (dataPath != "" && dataDir.exists())
        args << "--datadir" << dataPath;
    if (configPath != "" && configDir.exists())
        args << "--configdir" << configPath;
    if (pluginPath != "" && pluginDir.exists())
        args << "--plugindir" << pluginPath;

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

    mupen64proc->start(mupen64Path, args);
}


void Mupen64PlusQt::runEmulatorFromRomTree()
{
    QString completeRomFileName = QVariant(romTree->currentItem()->data(0, 0)).toString();
    QString completeRomPath = romDir.absoluteFilePath(completeRomFileName);
    runEmulator(completeRomPath);
}


void Mupen64PlusQt::stopEmulator()
{
    mupen64proc->terminate();
}


void Mupen64PlusQt::toggleMenus(bool active)
{
    QListIterator<QAction*> enableIter(menuEnable);
    while(enableIter.hasNext())
        enableIter.next()->setEnabled(active);

    QListIterator<QAction*> disableIter(menuDisable);
    while(disableIter.hasNext())
        disableIter.next()->setEnabled(!active);

    romTree->setEnabled(active);

    if (romTree->currentItem() == NULL)
        startAction->setEnabled(false);
}

