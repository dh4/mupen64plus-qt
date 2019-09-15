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

#include "gamesettingsdialog.h"
#include "ui_gamesettingsdialog.h"

#include "../global.h"

#include <QFileDialog>


GameSettingsDialog::GameSettingsDialog(QString fileName, QWidget *parent)
    : QDialog(parent), ui(new Ui::GameSettingsDialog)
{
    this->fileName = fileName;
    ui->setupUi(this);

    QString labelText = ui->gameLabel->text();
    labelText.append(" <b>"+fileName+"</b>");
    ui->gameLabel->setText(labelText);


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

    ui->videoBox->insertItems(1, videoPlugins);
    ui->audioBox->insertItems(1, audioPlugins);
    ui->inputBox->insertItems(1, inputPlugins);
    ui->rspBox->insertItems(1, rspPlugins);

    int videoIndex = videoPlugins.indexOf(SETTINGS.value(fileName+"/video","").toString());
    int audioIndex = audioPlugins.indexOf(SETTINGS.value(fileName+"/audio","").toString());
    int inputIndex = inputPlugins.indexOf(SETTINGS.value(fileName+"/input","").toString());
    int rspIndex = rspPlugins.indexOf(SETTINGS.value(fileName+"/rsp","").toString());

    if (videoIndex >= 0) ui->videoBox->setCurrentIndex(videoIndex + 1);
    if (audioIndex >= 0) ui->audioBox->setCurrentIndex(audioIndex + 1);
    if (inputIndex >= 0) ui->inputBox->setCurrentIndex(inputIndex + 1);
    if (rspIndex >= 0) ui->rspBox->setCurrentIndex(rspIndex + 1);

    ui->configPath->setText(SETTINGS.value(fileName+"/config", "").toString());
    ui->parameters->setText(SETTINGS.value(fileName+"/parameters", "").toString());

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    connect(ui->configButton, SIGNAL(clicked()), this, SLOT(browseConfig()));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(editGameSettings()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


GameSettingsDialog::~GameSettingsDialog()
{
    delete ui;
}


void GameSettingsDialog::browseConfig()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Config Directory"));
    if (path != "")
        ui->configPath->setText(path);
}


void GameSettingsDialog::editGameSettings()
{
    if (ui->videoBox->currentIndex() > 0)
        SETTINGS.setValue(fileName+"/video", ui->videoBox->currentText());
    else
        SETTINGS.setValue(fileName+"/video", "");

    if (ui->audioBox->currentIndex() > 0)
        SETTINGS.setValue(fileName+"/audio", ui->audioBox->currentText());
    else
        SETTINGS.setValue(fileName+"/audio", "");

    if (ui->inputBox->currentIndex() > 0)
        SETTINGS.setValue(fileName+"/input", ui->inputBox->currentText());
    else
        SETTINGS.setValue(fileName+"/input", "");

    if (ui->rspBox->currentIndex() > 0)
        SETTINGS.setValue(fileName+"/rsp", ui->rspBox->currentText());
    else
        SETTINGS.setValue(fileName+"/rsp", "");

    SETTINGS.setValue(fileName+"/config", ui->configPath->text());
    SETTINGS.setValue(fileName+"/parameters", ui->parameters->text());


    // Clean up game settings if they are empty
    bool emptyCheck = true;
    foreach (QString key, SETTINGS.allKeys())
        if (key.startsWith(fileName) && SETTINGS.value(key, "") != "")
            emptyCheck = false;

    if (emptyCheck)
        SETTINGS.remove(fileName);
}
