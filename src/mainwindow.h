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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>

class QActionGroup;
class QDialogButtonBox;
class QHeaderView;
class QGridLayout;
class QLabel;
class QListWidget;
class QMenuBar;
class QScrollArea;
class QTreeWidget;
class QVBoxLayout;
class EmulatorHandler;
class GridView;
class ListView;
class RomCollection;
class TableView;
class TheGamesDBScraper;
class TreeWidgetItem;
struct Rom;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject*, QEvent *event);

private:
    void autoloadSettings();
    void createMenu();
    void createRomView();
    void openZipDialog(QStringList zippedFiles);
    void resetLayouts(bool imageUpdated = false);
    void showActiveView();

    QString getCurrentRomInfoFromView(QString infoName);
    QString openPath;

    QAction *aboutAction;
    QAction *configureAction;
    QAction *configureGameAction;
    QAction *controlInfoAction;
    QAction *deleteAction;
    QAction *downloadAction;
    QAction *editorAction;
    QAction *fullScreenAction;
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
    QGridLayout *zipLayout;
    QLabel *emptyIcon;
    QLabel *disabledLabel;
    QList<QAction*> menuEnable;
    QList<QAction*> menuDisable;
    QList<QAction*> menuRomSelected;
    QListWidget *zipList;
    QMenu *emulationMenu;
    QMenu *fileMenu;
    QMenu *helpMenu;
    QMenu *layoutMenu;
    QMenu *settingsMenu;
    QMenu *viewMenu;
    QMenuBar *menuBar;
    QScrollArea *emptyView;
    QVBoxLayout *disabledLayout;
    QVBoxLayout *mainLayout;
    QWidget *disabledView;
    QWidget *mainWidget;

    EmulatorHandler *emulation;
    GridView *gridView;
    ListView *listView;
    RomCollection *romCollection;
    TableView *tableView;
    TheGamesDBScraper *scraper;
    TreeWidgetItem *fileItem;

private slots:
    void addToView(Rom *currentRom, int count);
    void disableButtons();
    void disableViews(bool imageUpdated);
    void enableButtons();
    void enableViews(int romCount, bool cached);
    void launchRomFromMenu();
    void launchRomFromTable();
    void launchRomFromWidget(QWidget *current);
    void launchRomFromZip();
    void openAbout();
    void openDefaultControls();
    void openDeleteDialog();
    void openDownloader();
    void openEditor();
    void openGameSettings();
    void openLog();
    void openSettings();
    void openRom();
    void showMenuBar(bool mouseAtTop);
    void showRomMenu(const QPoint &);
    void stopEmulator();
    void toggleMenus(bool active);
    void updateFullScreenMode();
    void updateLayoutSetting();

};

#endif // MAINWINDOW_H
