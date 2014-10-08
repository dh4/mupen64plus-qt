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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCryptographicHash>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenuBar>
#include <QProcess>
#include <QProgressDialog>
#include <QScrollArea>
#include <QScrollBar>
#include <QStatusBar>
#include <QTimer>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include "aboutdialog.h"
#include "clickablewidget.h"
#include "common.h"
#include "configeditor.h"
#include "downloaddialog.h"
#include "emulatorhandler.h"
#include "global.h"
#include "logdialog.h"
#include "settingsdialog.h"
#include "treewidgetitem.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    void toggleMenus(bool active);
    QString getCurrentRomInfo(int index);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void addToGridView(Rom *currentRom, int count);
    void addToListView(Rom *currentRom, int count);
    void addToTableView(Rom *currentRom);
    void autoloadSettings();
    void cachedRoms(bool imageUpdated = false);
    void createMenu();
    void createRomView();
    void openZipDialog(QStringList zippedFiles);
    void resetLayouts(QStringList tableVisible, bool imageUpdated = false);
    void runEmulator(QString romFileName, QString zipFileName = "");
    void saveColumnWidths();
    void setGridBackground();
    void setupDatabase();
    void setupProgressDialog(int size);

    Rom addRom(QByteArray *romData, QString fileName, QString zipFile, QSqlQuery query);

    int currentGridRom;
    int currentListRom;
    int positionx;
    int positiony;
    bool gridCurrent;
    bool listCurrent;

    QDir romDir;
    QString romPath;
    QStringList headerLabels;

    QAction *aboutAction;
    QAction *configureAction;
    QAction *downloadAction;
    QAction *editorAction;
    QAction *logAction;
    QAction *openAction;
    QAction *quitAction;
    QAction *refreshAction;
    QAction *startAction;
    QAction *stopAction;
    QActionGroup *layoutGroup;
    QDialog *zipDialog;
    QDialogButtonBox *zipButtonBox;
    QGridLayout *emptyLayout;
    QGridLayout *gridLayout;
    QGridLayout *zipLayout;
    QHeaderView *headerView;
    QLabel *icon;
    QList<QAction*> menuEnable;
    QList<QAction*> menuDisable;
    QListWidget *zipList;
    QMenu *emulationMenu;
    QMenu *fileMenu;
    QMenu *helpMenu;
    QMenu *layoutMenu;
    QMenu *settingsMenu;
    QMenuBar *menuBar;
    QProgressDialog *progress;
    QScrollArea *emptyView;
    QScrollArea *listView;
    QScrollArea *gridView;
    QSqlDatabase database;
    QString openPath;
    QTreeWidget *romTree;
    QVBoxLayout *layout;
    QVBoxLayout *listLayout;
    QWidget *listWidget;
    QWidget *gridContainer;
    QWidget *gridWidget;
    QWidget *widget;

    EmulatorHandler *emulation;
    TreeWidgetItem *fileItem;

private slots:
    void addRoms();
    void disableButtons();
    void enableButtons();
    void highlightGridWidget(QWidget *current);
    void highlightListWidget(QWidget *current);
    void openAbout();
    void openDownloader();
    void openEditor();
    void openLog();
    void openSettings();
    void openRom();
    void runEmulatorFromMenu();
    void runEmulatorFromRomTree();
    void runEmulatorFromWidget(QWidget *current);
    void runEmulatorFromZip();
    void saveSortOrder(int column, Qt::SortOrder order);
    void setGridPosition();
    void setListPosition();
    void setTablePosition();
    void stopEmulator();
    void updateLayoutSetting();

};

#endif // MAINWINDOW_H
