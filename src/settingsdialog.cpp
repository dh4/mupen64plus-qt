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


    //Populate Columns tab
    QStringList available, current;

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

    current = SETTINGS.value("ROMs/columns", "Filename|Size").toString().split("|");

    foreach (QString cur, current)
    {
        if (available.contains(cur))
            available.removeOne(cur);
        else //Someone added an invalid item
            current.removeOne(cur);
    }

    ui->availableList->addItems(available);
    ui->availableList->sortItems();

    ui->currentList->addItems(current);

    if (SETTINGS.value("ROMs/stretchfirstcolumn", "true").toString() == "true")
        ui->stretchOption->setChecked(true);

    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addColumn()));
    connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(removeColumn()));
    connect(ui->sortUpButton, SIGNAL(clicked()), this, SLOT(sortUp()));
    connect(ui->sortDownButton, SIGNAL(clicked()), this, SLOT(sortDown()));


    //Populate Other tab
    if (SETTINGS.value("saveoptions", "").toString() == "true")
        ui->saveOption->setChecked(true);


    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(editSettings()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


SettingsDialog::~SettingsDialog()
{
    delete ui;
}


void SettingsDialog::addColumn()
{
    int row = ui->availableList->currentRow();

    if (row >= 0) {
        ui->currentList->addItem(ui->availableList->currentItem()->text());
        delete ui->availableList->takeItem(row);
    }
}

void SettingsDialog::browseMupen64()
{
    QString path = QFileDialog::getOpenFileName(this);
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
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "")
        ui->pluginPath->setText(path);

}


void SettingsDialog::browseData()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "")
        ui->dataPath->setText(path);
}


void SettingsDialog::browseConfig()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "")
        ui->configPath->setText(path);
}


void SettingsDialog::browseROM()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "")
        ui->romPath->setText(path);
}


void SettingsDialog::editSettings()
{
    SETTINGS.setValue("Paths/mupen64plus", ui->mupen64Path->text());
    SETTINGS.setValue("Paths/roms", ui->romPath->text());
    SETTINGS.setValue("Paths/plugins", ui->pluginPath->text());
    SETTINGS.setValue("Paths/data", ui->dataPath->text());
    SETTINGS.setValue("Paths/config", ui->configPath->text());

    if (ui->pureButton->isChecked())
        SETTINGS.setValue("Emulation/mode", "0");
    else if (ui->cachedButton->isChecked())
        SETTINGS.setValue("Emulation/mode", "1");
    else
        SETTINGS.setValue("Emulation/mode", "2");

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

    SETTINGS.setValue("Plugins/video", ui->videoBox->currentText());
    SETTINGS.setValue("Plugins/audio", ui->audioBox->currentText());
    SETTINGS.setValue("Plugins/input", ui->inputBox->currentText());
    SETTINGS.setValue("Plugins/rsp", ui->rspBox->currentText());

    QStringList visibleItems;
    foreach (QListWidgetItem *item, ui->currentList->findItems("*", Qt::MatchWildcard))
        visibleItems << item->text();

    SETTINGS.setValue("ROMs/columns", visibleItems.join("|"));

    if (ui->stretchOption->isChecked())
        SETTINGS.setValue("ROMs/stretchfirstcolumn", true);
    else
        SETTINGS.setValue("ROMs/stretchfirstcolumn", "");

    if (ui->saveOption->isChecked())
        SETTINGS.setValue("saveoptions", true);
    else
        SETTINGS.setValue("saveoptions", "");

    close();
}


void SettingsDialog::removeColumn()
{
    int row = ui->currentList->currentRow();

    if (row >= 0) {
        ui->availableList->addItem(ui->currentList->currentItem()->text());
        delete ui->currentList->takeItem(row);

        ui->availableList->sortItems();
    }
}


void SettingsDialog::sortDown()
{
    int row = ui->currentList->currentRow();

    if (row > 0) {
        QListWidgetItem *item = ui->currentList->takeItem(row);
        ui->currentList->insertItem(row - 1, item);
        ui->currentList->setCurrentRow(row - 1);
    }
}


void SettingsDialog::sortUp()
{
    int row = ui->currentList->currentRow();

    if (row >= 0 && row < ui->currentList->count() - 1) {
        QListWidgetItem *item = ui->currentList->takeItem(row);
        ui->currentList->insertItem(row + 1, item);
        ui->currentList->setCurrentRow(row + 1);
    }
}
