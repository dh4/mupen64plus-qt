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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "global.h"


SettingsDialog::SettingsDialog(QWidget *parent, int activeTab) : QDialog(parent), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(activeTab);


    //Populate Paths tab
    ui->mupen64Path->setText(SETTINGS.value("Paths/mupen64plus", "").toString());
    ui->romPath->setText(SETTINGS.value("Paths/roms", "").toString());
    ui->pluginPath->setText(SETTINGS.value("Paths/plugins", "").toString());
    ui->dataPath->setText(SETTINGS.value("Paths/data", "").toString());
    ui->configPath->setText(SETTINGS.value("Paths/config", "").toString());

    connect(ui->mupen64Button, SIGNAL(clicked()), this, SLOT(browseMupen64()));
    connect(ui->pluginButton, SIGNAL(clicked()), this, SLOT(browsePlugin()));
    connect(ui->dataButton, SIGNAL(clicked()), this, SLOT(browseData()));
    connect(ui->configButton, SIGNAL(clicked()), this, SLOT(browseConfig()));
    connect(ui->romButton, SIGNAL(clicked()), this, SLOT(browseROM()));


    //Populate Emulation tab
    QString emuMode = SETTINGS.value("Emulation/mode", "").toString();
    if (emuMode == "0")
        ui->pureButton->setChecked(true);
    else if (emuMode == "1")
        ui->cachedButton->setChecked(true);
    else
        ui->dynamicButton->setChecked(true);


    //Populate Graphics tab
    if (SETTINGS.value("Graphics/osd", "").toString() == "true")
        ui->osdOption->setChecked(true);
    if (SETTINGS.value("Graphics/fullscreen", "").toString() == "true")
        ui->fullscreenOption->setChecked(true);

    QStringList useableModes, modes;
    useableModes << "default"; //Allow users to use the screen resolution set in the config file

    modes << "2560x1600"
          << "2560x1440"
          << "2048x1152"
          << "1920x1200"
          << "1920x1080"
          << "1680x1050"
          << "1600x1200"
          << "1600x900"
          << "1440x900"
          << "1400x1050"
          << "1366x768"
          << "1360x768"
          << "1280x1024"
          << "1280x960"
          << "1280x800"
          << "1280x768"
          << "1280x720"
          << "1152x864"
          << "1024x768"
          << "1024x600"
          << "800x600"
          << "640x480";


    desktop = new QDesktopWidget;
    int screenWidth = desktop->width();
    int screenHeight = desktop->height();

    foreach (QString mode, modes)
    {
        QStringList values = mode.split("x");

        if (values.value(0).toInt() <= screenWidth && values.value(1).toInt() <= screenHeight)
            useableModes << mode;
    }

    ui->resolutionBox->insertItems(0, useableModes);
    int resIndex = useableModes.indexOf(SETTINGS.value("Graphics/resolution","").toString());
    if (resIndex >= 0) ui->resolutionBox->setCurrentIndex(resIndex);


    //Populate Plugins tab
    QStringList audioPlugins, inputPlugins, rspPlugins, videoPlugins;
    pluginsDir = QDir(SETTINGS.value("Paths/plugins", "").toString());

    if (pluginsDir.exists()) {
        QStringList files = pluginsDir.entryList(QStringList() << "*", QDir::Files);

        if (files.size() > 0) {
            foreach (QString fileName, files)
            {
                QString baseName = QFileInfo(fileName).completeBaseName();

                if (fileName.contains("-audio-"))
                    audioPlugins << baseName;
                else if (fileName.contains("-input-"))
                    inputPlugins << baseName;
                else if (fileName.contains("-rsp-"))
                    rspPlugins << baseName;
                else if (fileName.contains("-video-"))
                    videoPlugins << baseName;
            }
        }
    }

    ui->videoBox->insertItems(0, videoPlugins);
    ui->audioBox->insertItems(0, audioPlugins);
    ui->inputBox->insertItems(0, inputPlugins);
    ui->rspBox->insertItems(0, rspPlugins);

    //Set Rice as default
    QString videoDefault = "";
    if (videoPlugins.contains("mupen64plus-video-rice"))
        videoDefault = "mupen64plus-video-rice";

    int videoIndex = videoPlugins.indexOf(SETTINGS.value("Plugins/video",videoDefault).toString());
    int audioIndex = videoPlugins.indexOf(SETTINGS.value("Plugins/audio","").toString());
    int inputIndex = videoPlugins.indexOf(SETTINGS.value("Plugins/input","").toString());
    int rspIndex = videoPlugins.indexOf(SETTINGS.value("Plugins/rsp","").toString());

    if (videoIndex >= 0) ui->videoBox->setCurrentIndex(videoIndex);
    if (audioIndex >= 0) ui->audioBox->setCurrentIndex(audioIndex);
    if (inputIndex >= 0) ui->inputBox->setCurrentIndex(inputIndex);
    if (rspIndex >= 0) ui->rspBox->setCurrentIndex(rspIndex);


    //Populate Table tab
    QStringList sizes;
    sizes << "Extra Small"
          << "Small"
          << "Medium"
          << "Large"
          << "Extra Large";

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true")
        populateTableAndListTab(true);
    else
        populateTableAndListTab(false);

    if (SETTINGS.value("Table/stretchfirstcolumn", "true").toString() == "true")
        ui->tableStretchOption->setChecked(true);

    ui->tableSizeBox->insertItems(0, sizes);
    int tableSizeIndex = sizes.indexOf(SETTINGS.value("Table/imagesize","Medium").toString());
    if (tableSizeIndex >= 0) ui->tableSizeBox->setCurrentIndex(tableSizeIndex);

    connect(ui->tableAddButton, SIGNAL(clicked()), this, SLOT(tableAddColumn()));
    connect(ui->tableRemoveButton, SIGNAL(clicked()), this, SLOT(tableRemoveColumn()));
    connect(ui->tableSortUpButton, SIGNAL(clicked()), this, SLOT(tableSortUp()));
    connect(ui->tableSortDownButton, SIGNAL(clicked()), this, SLOT(tableSortDown()));


    //Populate Grid tab
    QStringList colors;
    colors << "Black"
           << "White"
           << "Light Gray"
           << "Dark Gray"
           << "Green"
           << "Cyan"
           << "Blue"
           << "Purple"
           << "Red"
           << "Pink"
           << "Orange"
           << "Yellow"
           << "Brown";

    ui->gridSizeBox->insertItems(0, sizes);
    int gridSizeIndex = sizes.indexOf(SETTINGS.value("Grid/imagesize","Medium").toString());
    if (gridSizeIndex >= 0) ui->gridSizeBox->setCurrentIndex(gridSizeIndex);

    int gridColumnCount = SETTINGS.value("Grid/columncount","4").toInt();
    ui->columnCountBox->setValue(gridColumnCount);

    ui->shadowActiveBox->insertItems(0, colors);
    int activeIndex = colors.indexOf(SETTINGS.value("Grid/activecolor","Cyan").toString());
    if (activeIndex >= 0) ui->shadowActiveBox->setCurrentIndex(activeIndex);

    ui->shadowInactiveBox->insertItems(0, colors);
    int inactiveIndex = colors.indexOf(SETTINGS.value("Grid/inactivecolor","Black").toString());
    if (inactiveIndex >= 0) ui->shadowInactiveBox->setCurrentIndex(inactiveIndex);

    //Widgets to enable when label active
    labelEnable << ui->labelTextLabel
                << ui->labelTextBox
                << ui->labelColorLabel
                << ui->labelColorBox;

    if (SETTINGS.value("Grid/label", "true").toString() == "true") {
        toggleLabel(true);
        ui->labelOption->setChecked(true);
    } else
        toggleLabel(false);

    ui->labelColorBox->insertItems(0, colors);
    int labelColorIndex = colors.indexOf(SETTINGS.value("Grid/labelcolor","White").toString());
    if (labelColorIndex >= 0) ui->labelColorBox->setCurrentIndex(labelColorIndex);

    ui->backgroundPath->setText(SETTINGS.value("Grid/background", "").toString());

    if (SETTINGS.value("Grid/sortdirection", "ascending").toString() == "descending")
        ui->gridDescendingOption->setChecked(true);

    connect(ui->backgroundButton, SIGNAL(clicked()), this, SLOT(browseBackground()));
    connect(ui->labelOption, SIGNAL(toggled(bool)), this, SLOT(toggleLabel(bool)));


    //Populate List tab
    listCoverEnable << ui->listSizeLabel
                    << ui->listSizeBox;

    if (SETTINGS.value("List/displaycover", "").toString() == "true") {
        toggleListCover(true);
        ui->listCoverOption->setChecked(true);
    } else
        toggleListCover(false);

    if (SETTINGS.value("List/firstitemheader", "true").toString() == "true")
        ui->listHeaderOption->setChecked(true);

    ui->listSizeBox->insertItems(0, sizes);
    int listSizeIndex = sizes.indexOf(SETTINGS.value("List/imagesize","Medium").toString());
    if (listSizeIndex >= 0) ui->listSizeBox->setCurrentIndex(listSizeIndex);

    if (SETTINGS.value("List/sortdirection", "ascending").toString() == "descending")
        ui->listDescendingOption->setChecked(true);


    connect(ui->listCoverOption, SIGNAL(toggled(bool)), this, SLOT(toggleListCover(bool)));
    connect(ui->listAddButton, SIGNAL(clicked()), this, SLOT(listAddColumn()));
    connect(ui->listRemoveButton, SIGNAL(clicked()), this, SLOT(listRemoveColumn()));
    connect(ui->listSortUpButton, SIGNAL(clicked()), this, SLOT(listSortUp()));
    connect(ui->listSortDownButton, SIGNAL(clicked()), this, SLOT(listSortDown()));


    //Populate Other tab
    downloadEnable << ui->tableSizeLabel
                   << ui->tableSizeBox
                   << ui->listCoverOption
                   << ui->listSizeLabel
                   << ui->listSizeBox;

    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true") {
        toggleDownload(true);
        ui->downloadOption->setChecked(true);
    } else
        toggleDownload(false);

    if (SETTINGS.value("saveoptions", "").toString() == "true")
        ui->saveOption->setChecked(true);

    connect(ui->downloadOption, SIGNAL(toggled(bool)), this, SLOT(toggleDownload(bool)));
    connect(ui->downloadOption, SIGNAL(toggled(bool)), this, SLOT(populateTableAndListTab(bool)));


    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(editSettings()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


SettingsDialog::~SettingsDialog()
{
    delete ui;
}


void SettingsDialog::addColumn(QListWidget *currentList, QListWidget *availableList)
{
    int row = availableList->currentRow();

    if (row >= 0) {
        currentList->addItem(availableList->currentItem()->text());
        delete availableList->takeItem(row);
    }
}


void SettingsDialog::browseBackground()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Background Image"));
    if (path != "")
        ui->backgroundPath->setText(path);
}


void SettingsDialog::browseMupen64()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Mupen64Plus Executable"));
    if (path != "")
        ui->mupen64Path->setText(path);

#ifdef Q_OS_OSX
    //Allow OSX users to just select the .app directory and auto-populate for them
    if (path.right(15) == "mupen64plus.app") {
        ui->mupen64Path->setText(path+"/Contents/MacOS/mupen64plus");
        ui->pluginPath->setText(path+"/Contents/MacOS");
        ui->dataPath->setText(path+"/Contents/Resources");
    }
#endif
}


void SettingsDialog::browsePlugin()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Plugin Directory"));
    if (path != "")
        ui->pluginPath->setText(path);

}


void SettingsDialog::browseData()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Data Directory"));
    if (path != "")
        ui->dataPath->setText(path);
}


void SettingsDialog::browseConfig()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Config Directory"));
    if (path != "")
        ui->configPath->setText(path);
}


void SettingsDialog::browseROM()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("ROM Directory"));
    if (path != "")
        ui->romPath->setText(path);
}


void SettingsDialog::editSettings()
{
    //Set download option first
    if (ui->downloadOption->isChecked()) {
        SETTINGS.setValue("Other/downloadinfo", true);
        populateAvailable(true); //This removes thegamesdb.net options if user unselects download
    } else {
        SETTINGS.setValue("Other/downloadinfo", "");
        populateAvailable(false);
    }


    //Paths tab
    SETTINGS.setValue("Paths/mupen64plus", ui->mupen64Path->text());
    SETTINGS.setValue("Paths/roms", ui->romPath->text());
    SETTINGS.setValue("Paths/plugins", ui->pluginPath->text());
    SETTINGS.setValue("Paths/data", ui->dataPath->text());
    SETTINGS.setValue("Paths/config", ui->configPath->text());


    //Emulation tab
    if (ui->pureButton->isChecked())
        SETTINGS.setValue("Emulation/mode", "0");
    else if (ui->cachedButton->isChecked())
        SETTINGS.setValue("Emulation/mode", "1");
    else
        SETTINGS.setValue("Emulation/mode", "2");


    //Graphics tab
    if (ui->osdOption->isChecked())
        SETTINGS.setValue("Graphics/osd", true);
    else
        SETTINGS.setValue("Graphics/osd", "");

    if (ui->fullscreenOption->isChecked())
        SETTINGS.setValue("Graphics/fullscreen", true);
    else
        SETTINGS.setValue("Graphics/fullscreen", "");

    if (ui->resolutionBox->currentText() != "default")
        SETTINGS.setValue("Graphics/resolution", ui->resolutionBox->currentText());
    else
        SETTINGS.setValue("Graphics/resolution", "");


    //Plugins tab
    SETTINGS.setValue("Plugins/video", ui->videoBox->currentText());
    SETTINGS.setValue("Plugins/audio", ui->audioBox->currentText());
    SETTINGS.setValue("Plugins/input", ui->inputBox->currentText());
    SETTINGS.setValue("Plugins/rsp", ui->rspBox->currentText());


    //Table tab
    QStringList tableVisibleItems;
    foreach (QListWidgetItem *item, ui->tableCurrentList->findItems("*", Qt::MatchWildcard))
        if (available.contains(item->text()))
            tableVisibleItems << item->text();

    SETTINGS.setValue("Table/columns", tableVisibleItems.join("|"));

    if (ui->tableStretchOption->isChecked())
        SETTINGS.setValue("Table/stretchfirstcolumn", true);
    else
        SETTINGS.setValue("Table/stretchfirstcolumn", "");

    SETTINGS.setValue("Table/imagesize", ui->tableSizeBox->currentText());


    //Grid tab
    SETTINGS.setValue("Grid/imagesize", ui->gridSizeBox->currentText());
    SETTINGS.setValue("Grid/columncount", ui->columnCountBox->value());
    SETTINGS.setValue("Grid/inactivecolor", ui->shadowInactiveBox->currentText());
    SETTINGS.setValue("Grid/activecolor", ui->shadowActiveBox->currentText());
    SETTINGS.setValue("Grid/background", ui->backgroundPath->text());

    if (ui->labelOption->isChecked())
        SETTINGS.setValue("Grid/label", true);
    else
        SETTINGS.setValue("Grid/label", "");

    SETTINGS.setValue("Grid/labeltext", ui->labelTextBox->currentText());
    SETTINGS.setValue("Grid/labelcolor", ui->labelColorBox->currentText());
    SETTINGS.setValue("Grid/sort", ui->gridSortBox->currentText());

    if (ui->gridDescendingOption->isChecked())
        SETTINGS.setValue("Grid/sortdirection", "descending");
    else
        SETTINGS.setValue("Grid/sortdirection", "ascending");


    //List tab
    QStringList listVisibleItems;
    foreach (QListWidgetItem *item, ui->listCurrentList->findItems("*", Qt::MatchWildcard))
        if (available.contains(item->text()))
            listVisibleItems << item->text();

    SETTINGS.setValue("List/columns", listVisibleItems.join("|"));

    if (ui->listHeaderOption->isChecked())
        SETTINGS.setValue("List/firstitemheader", true);
    else
        SETTINGS.setValue("List/firstitemheader", "");

    if (ui->listCoverOption->isChecked() && ui->downloadOption->isChecked())
        SETTINGS.setValue("List/displaycover", true);
    else
        SETTINGS.setValue("List/displaycover", "");

    SETTINGS.setValue("List/imagesize", ui->listSizeBox->currentText());
    SETTINGS.setValue("List/sort", ui->listSortBox->currentText());

    if (ui->listDescendingOption->isChecked())
        SETTINGS.setValue("List/sortdirection", "descending");
    else
        SETTINGS.setValue("List/sortdirection", "ascending");


    //Other tab
    if (ui->saveOption->isChecked())
        SETTINGS.setValue("saveoptions", true);
    else
        SETTINGS.setValue("saveoptions", "");

    close();
}


void SettingsDialog::listAddColumn()
{
    addColumn(ui->listCurrentList, ui->listAvailableList);
}


void SettingsDialog::listRemoveColumn()
{
    removeColumn(ui->listCurrentList, ui->listAvailableList);
}


void SettingsDialog::listSortDown()
{
    sortDown(ui->listCurrentList);
}


void SettingsDialog::listSortUp()
{
    sortUp(ui->listCurrentList);
}


void SettingsDialog::populateAvailable(bool downloadItems) {
    available.clear();
    labelOptions.clear();
    sortOptions.clear();

    available << "Filename"
              << "Filename (extension)"
              << "GoodName"
              << "Internal Name"
              << "Size"
              << "MD5"
              << "CRC1"
              << "CRC2"
              << "Players"
              << "Rumble"
              << "Save Type";

    labelOptions << "Filename"
                 << "Filename (extension)"
                 << "GoodName"
                 << "Internal Name";

    sortOptions << "Filename"
                << "GoodName"
                << "Internal Name"
                << "Size";

    if (downloadItems) {
        available << "Game Title"
                  << "Release Date"
                  << "Overview"
                  << "ESRB"
                  << "Genre"
                  << "Publisher"
                  << "Developer"
                  << "Rating"
                  << "Game Cover";

        labelOptions << "Game Title"
                     << "Release Date"
                     << "Genre";

        sortOptions << "Game Title"
                    << "Release Date"
                    << "ESRB"
                    << "Genre"
                    << "Publisher"
                    << "Developer"
                    << "Rating";
    }

    available.sort();
    labelOptions.sort();
    sortOptions.sort();
}


void SettingsDialog::populateTableAndListTab(bool downloadItems)
{
    populateAvailable(downloadItems);

    QStringList tableCurrent, tableAvailable;
    tableCurrent = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");
    tableAvailable = available;

    foreach (QString cur, tableCurrent)
    {
        if (tableAvailable.contains(cur))
            tableAvailable.removeOne(cur);
        else //Someone added an invalid item
            tableCurrent.removeOne(cur);
    }

    ui->tableAvailableList->clear();
    ui->tableAvailableList->addItems(tableAvailable);
    ui->tableAvailableList->sortItems();

    ui->tableCurrentList->clear();
    ui->tableCurrentList->addItems(tableCurrent);


    //Grid sort field and label text
    ui->labelTextBox->clear();
    ui->labelTextBox->insertItems(0, labelOptions);
    int labelTextIndex = labelOptions.indexOf(SETTINGS.value("Grid/labeltext","Filename").toString());
    if (labelTextIndex >= 0) ui->labelTextBox->setCurrentIndex(labelTextIndex);

    ui->gridSortBox->clear();
    ui->gridSortBox->insertItems(0, sortOptions);
    int gridSortIndex = sortOptions.indexOf(SETTINGS.value("Grid/sort","Filename").toString());
    if (gridSortIndex >= 0) ui->gridSortBox->setCurrentIndex(gridSortIndex);


    //List items and sort field
    QStringList listCurrent, listAvailable;
    listCurrent = SETTINGS.value("List/columns", "Filename|Internal Name|Size").toString().split("|");
    listAvailable = available;
    listAvailable.removeOne("Game Cover"); //Game Cover handled separately

    foreach (QString cur, listCurrent)
    {
        if (listAvailable.contains(cur))
            listAvailable.removeOne(cur);
        else //Someone added an invalid item
            listCurrent.removeOne(cur);
    }

    ui->listAvailableList->clear();
    ui->listAvailableList->addItems(listAvailable);
    ui->listAvailableList->sortItems();

    ui->listCurrentList->clear();
    ui->listCurrentList->addItems(listCurrent);

    ui->listSortBox->clear();
    ui->listSortBox->insertItems(0, sortOptions);
    int listSortIndex = sortOptions.indexOf(SETTINGS.value("List/sort","Filename").toString());
    if (listSortIndex >= 0) ui->listSortBox->setCurrentIndex(listSortIndex);
}


void SettingsDialog::removeColumn(QListWidget *currentList, QListWidget *availableList)
{
    int row = currentList->currentRow();

    if (row >= 0) {
        availableList->addItem(currentList->currentItem()->text());
        delete currentList->takeItem(row);

        availableList->sortItems();
    }
}


void SettingsDialog::sortDown(QListWidget *currentList)
{
    int row = currentList->currentRow();

    if (row > 0) {
        QListWidgetItem *item = currentList->takeItem(row);
        currentList->insertItem(row - 1, item);
        currentList->setCurrentRow(row - 1);
    }
}


void SettingsDialog::sortUp(QListWidget *currentList)
{
    int row = currentList->currentRow();

    if (row >= 0 && row < currentList->count() - 1) {
        QListWidgetItem *item = currentList->takeItem(row);
        currentList->insertItem(row + 1, item);
        currentList->setCurrentRow(row + 1);
    }
}


void SettingsDialog::tableAddColumn()
{
    addColumn(ui->tableCurrentList, ui->tableAvailableList);
}


void SettingsDialog::tableRemoveColumn()
{
    removeColumn(ui->tableCurrentList, ui->tableAvailableList);
}


void SettingsDialog::tableSortDown()
{
    sortDown(ui->tableCurrentList);
}


void SettingsDialog::tableSortUp()
{
    sortUp(ui->tableCurrentList);
}


void SettingsDialog::toggleDownload(bool active)
{
    foreach (QWidget *next, downloadEnable)
        next->setEnabled(active);

    if (active)
        toggleListCover(ui->listCoverOption->isChecked());
}


void SettingsDialog::toggleLabel(bool active)
{
    foreach (QWidget *next, labelEnable)
        next->setEnabled(active);
}


void SettingsDialog::toggleListCover(bool active)
{
    foreach (QWidget *next, listCoverEnable)
        next->setEnabled(active);
}
