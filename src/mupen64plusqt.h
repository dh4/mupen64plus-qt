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
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QLineEdit>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QListWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QStatusBar>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QTextStream>
#include <QTime>
#include <QTimer>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtXml/QDomDocument>

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include "clickablewidget.h"
#include "treewidgetitem.h"


typedef struct {
    QString fileName;
    QString romMD5;
    QString internalName;
    QString zipFile;

    QString baseName;
    QString size;
    int sortSize;

    QString goodName;
    QString CRC1;
    QString CRC2;
    QString players;
    QString saveType;
    QString rumble;

    QString gameTitle;
    QString releaseDate;
    QString sortDate;
    QString overview;
    QString esrb;
    QString genre;
    QString publisher;
    QString developer;
    QString rating;

    QPixmap image;

    int count;
    bool imageExists;
} Rom;

bool romSorter(const Rom &firstRom, const Rom &lastRom);


class Mupen64PlusQt : public QMainWindow
{
    Q_OBJECT

public:
    Mupen64PlusQt(QWidget *parent = 0);
    static QString getRomInfo(QString identifier, const Rom *rom, bool removeWarn = false, bool sort = false);

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
    void downloadGameInfo(QString identifier, QString searchName, QString gameID = "", bool force = false);
    void initializeRom(Rom *currentRom, bool cached);
    void openZipDialog(QStringList zippedFiles);
    void resetLayouts(QStringList tableVisible, bool imageUpdated = false);
    void runEmulator(QString romFileName, QString zipFileName = "");
    void saveColumnWidths();
    void setGridBackground();
    void setupDatabase();
    void setupProgressDialog(int size);
    void toggleMenus(bool active);

    QByteArray byteswap(QByteArray romData);
    QByteArray getUrlContents(QUrl url);
    QColor getColor(QString color, int transparency = 255);
    QGraphicsDropShadowEffect *getShadow(bool active);
    QSize getImageSize(QString view);
    QString getDataLocation();
    QString getCurrentRomInfo(int index);
    Rom addRom(QString fileName, QString zipFile, qint64 size, QSqlQuery query);

    int currentGridRom;
    int currentListRom;
    int getGridSize(QString which);
    int positionx;
    int positiony;
    bool gridCurrent;
    bool listCurrent;

    QDir configDir;
    QDir dataDir;
    QDir pluginDir;
    QDir romDir;
    QDir savesDir;
    QString romPath;
    QStringList headerLabels;

    QAction *aboutAction;
    QAction *configureAction;
    QAction *downloadAction;
    QAction *editorAction;
    QAction *filenameAction;
    QAction *goodnameAction;
    QAction *logAction;
    QAction *openAction;
    QAction *quitAction;
    QAction *refreshAction;
    QAction *saveAction;
    QAction *sizeAction;
    QAction *startAction;
    QAction *stopAction;
    QActionGroup *layoutGroup;
    QByteArray *romData;
    QDialog *downloadDialog;
    QDialog *logDialog;
    QDialog *zipDialog;
    QDialogButtonBox *downloadButtonBox;
    QDialogButtonBox *logButtonBox;
    QDialogButtonBox *zipButtonBox;
    QGridLayout *downloadLayout;
    QGridLayout *emptyLayout;
    QGridLayout *gridLayout;
    QGridLayout *logLayout;
    QGridLayout *zipLayout;
    QHeaderView *headerView;
    QLabel *fileLabel;
    QLabel *gameNameLabel;
    QLabel *gameIDLabel;
    QLabel *icon;
    QLineEdit *gameNameField;
    QLineEdit *gameIDField;
    QList<QAction*> menuEnable;
    QList<QAction*> menuDisable;
    QListWidget *zipList;
    QMenu *emulationMenu;
    QMenu *fileMenu;
    QMenu *helpMenu;
    QMenu *layoutMenu;
    QMenu *settingsMenu;
    QMenuBar *menuBar;
    QProcess *mupen64proc;
    QProgressDialog *progress;
    QScrollArea *emptyView;
    QScrollArea *listView;
    QScrollArea *gridView;
    QSettings *romCatalog;
    QSqlDatabase database;
    QStatusBar *statusBar;
    QString lastOutput;
    QString openPath;
    QTextEdit *logArea;
    QTreeWidget *romTree;
    TreeWidgetItem *headerItem;
    TreeWidgetItem *fileItem;
    QVBoxLayout *layout;
    QVBoxLayout *listLayout;
    QWidget *listWidget;
    QWidget *gridContainer;
    QWidget *gridWidget;
    QWidget *widget;

private slots:
    void addRoms();
    void checkStatus(int status);
    void cleanTemp();
    void enableButtons();
    void highlightGridWidget(QWidget *current);
    void highlightListWidget(QWidget *current);
    void openAbout();
    void openDownloader();
    void openEditor();
    void openLog();
    void openOptions();
    void openRom();
    void readMupen64PlusOutput();
    void runDownloader();
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

#endif // MUPEN64PLUSQT_H
