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

#ifndef MUPEN64PLUSQT_H
#define MUPEN64PLUSQT_H

#include <QCloseEvent>
#include <QCryptographicHash>
#include <QDir>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QStatusBar>
#include <QTableWidgetItem>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "aboutdialog.h"
#include "global.h"
#include "settingsdialog.h"


class Mupen64PlusQt : public QMainWindow
{
    Q_OBJECT

public:
    Mupen64PlusQt(QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void createMenu();
    void createRomView();
    void openOptions(int activeTab);
    void runEmulator(QString completeRomPath);
    void toggleMenus(bool active);

    QDir configDir;
    QDir dataDir;
    QDir pluginDir;
    QDir romDir;
    QDir savesDir;
    QString romPath;

    QAction *aboutAction;
    QAction *convertAction;
    QAction *graphicsAction;
    QAction *openAction;
    QAction *outputAction;
    QAction *pathsAction;
    QAction *pluginsAction;
    QAction *quitAction;
    QAction *refreshAction;
    QAction *startAction;
    QAction *statusBarAction;
    QAction *stopAction;
    QActionGroup *inputGroup;
    QByteArray *romData;
    QList<QAction*> menuEnable;
    QList<QAction*> menuDisable;
    QMenu *emulationMenu;
    QMenu *fileMenu;
    QMenu *helpMenu;
    QMenu *inputMenu;
    QMenu *settingsMenu;
    QMenu *viewMenu;
    QMenuBar *menuBar;
    QProcess *mupen64proc;
    QStatusBar *statusBar;
    QTreeWidget *romTree;
    QTreeWidgetItem *headerItem;
    QTreeWidgetItem *fileItem;
    QVBoxLayout *layout;
    QWidget *widget;

private slots:
    void addRoms();
    void checkStatus(int status);
    void enableButtons();
    void openAbout();
    void openGraphics();
    void openPaths();
    void openPlugins();
    void openRom();
    void runEmulatorFromRomTree();
    void stopEmulator();

};

#endif // MUPEN64PLUSQT_H
