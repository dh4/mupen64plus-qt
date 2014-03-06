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


SettingsDialog::SettingsDialog(QWidget *parent, int activeTab) : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    setMinimumWidth(500);

    layout = new QGridLayout(this);
    tabWidget = new QTabWidget(this);


    //Paths tab
    pathsWidget = new QWidget(this);
    pathsLayout = new QGridLayout(pathsWidget);

    mupen64PathLabel = new QLabel(tr("Mupen64Plus executable:"), this);
    romPathLabel = new QLabel(tr("ROM directory:"), this);
    pluginPathLabel = new QLabel(tr("Plugins directory:"), this);
    dataPathLabel = new QLabel(tr("Data directory:"), this);
    configPathLabel = new QLabel(tr("Config directory:"), this);

    mupen64Path = new QLineEdit(SETTINGS.value("Paths/mupen64plus", "").toString(), this);
    romPath = new QLineEdit(SETTINGS.value("Paths/roms", "").toString(), this);
    pluginPath = new QLineEdit(SETTINGS.value("Paths/plugins", "").toString(), this);
    dataPath = new QLineEdit(SETTINGS.value("Paths/data", "").toString(), this);
    configPath = new QLineEdit(SETTINGS.value("Paths/config", "").toString(), this);

    mupen64Button = new QPushButton(tr("Browse..."), this);
    romButton = new QPushButton(tr("Browse..."), this);
    pluginButton = new QPushButton(tr("Browse..."), this);
    dataButton = new QPushButton(tr("Browse..."), this);
    configButton = new QPushButton(tr("Browse..."), this);

    pathsLayout->addWidget(mupen64PathLabel, 0, 0);
    pathsLayout->addWidget(romPathLabel, 1, 0);
    pathsLayout->addWidget(pluginPathLabel, 2, 0);
    pathsLayout->addWidget(dataPathLabel, 3, 0);
    pathsLayout->addWidget(configPathLabel, 4, 0);

    pathsLayout->addWidget(mupen64Path, 0, 1);
    pathsLayout->addWidget(romPath, 1, 1);
    pathsLayout->addWidget(pluginPath, 2, 1);
    pathsLayout->addWidget(dataPath, 3, 1);
    pathsLayout->addWidget(configPath, 4, 1);

    pathsLayout->addWidget(mupen64Button, 0, 2);
    pathsLayout->addWidget(romButton, 1, 2);
    pathsLayout->addWidget(pluginButton, 2, 2);
    pathsLayout->addWidget(dataButton, 3, 2);
    pathsLayout->addWidget(configButton, 4, 2);

    pathsLayout->setColumnStretch(1, 1);
    pathsLayout->setRowStretch(5, 1);
    pathsWidget->setLayout(pathsLayout);


    //Emulation tab
    emulationWidget = new QWidget(this);
    emulationLayout = new QGridLayout(emulationWidget);

    emulationGroup = new QButtonGroup(this);

    pureButton = new QRadioButton(tr("Pure Interpreter"), this);
    cachedButton = new QRadioButton(tr("Cached Interpreter"), this);
    dynamicButton = new QRadioButton(tr("Dynamic Recompiler"), this);

    emulationGroup->addButton(pureButton);
    emulationGroup->addButton(cachedButton);
    emulationGroup->addButton(dynamicButton);

    QString emuMode = SETTINGS.value("Emulation/mode", "").toString();
    if (emuMode == "0")
        pureButton->setChecked(true);
    else if (emuMode == "1")
        cachedButton->setChecked(true);
    else
        dynamicButton->setChecked(true);

    emulationLayout->addWidget(pureButton, 1, 0);
    emulationLayout->addWidget(cachedButton, 2, 0);
    emulationLayout->addWidget(dynamicButton, 3, 0);

    emulationLayout->setRowMinimumHeight(0, 10);
    emulationLayout->setColumnStretch(1, 1);
    emulationLayout->setRowStretch(4, 1);
    emulationWidget->setLayout(emulationLayout);


    //Graphics tab
    graphicsWidget = new QWidget(this);
    graphicsLayout = new QGridLayout(graphicsWidget);

    osdLabel = new QLabel(tr("On Screen Display:"), this);
    fullscreenLabel = new QLabel(tr("Fullscreen:"), this);
    resolutionLabel = new QLabel(tr("Resolution:"), this);

    osdOption = new QCheckBox(this);
    fullscreenOption = new QCheckBox(this);

    if (SETTINGS.value("Graphics/osd", "").toString() == "true")
        osdOption->setChecked(true);
    if (SETTINGS.value("Graphics/fullscreen", "").toString() == "true")
        fullscreenOption->setChecked(true);

    resolutionBox = new QComboBox(this);

    //Allow users to use the screen resolution set in the config file
    useableModes << "default";

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

    for(QStringList::Iterator it = modes.begin(); it != modes.end(); ++it)
    {
        QString mode = *it;
        QStringList values = mode.split("x");

        if (values.value(0).toInt() <= screenWidth && values.value(1).toInt() <= screenHeight)
            useableModes << mode;
    }

    resolutionBox->insertItems(0, useableModes);
    int resIndex = useableModes.indexOf(SETTINGS.value("Graphics/resolution","").toString());
    if (resIndex >= 0) resolutionBox->setCurrentIndex(resIndex);
    resolutionBox->setMaximumWidth(100);

    graphicsLayout->addWidget(osdLabel, 0, 0);
    graphicsLayout->addWidget(fullscreenLabel, 1, 0);
    graphicsLayout->addWidget(resolutionLabel, 2, 0);

    graphicsLayout->addWidget(osdOption, 0, 1);
    graphicsLayout->addWidget(fullscreenOption, 1, 1);
    graphicsLayout->addWidget(resolutionBox, 2, 1);

    graphicsLayout->setColumnMinimumWidth(0, 150);
    graphicsLayout->setColumnStretch(2, 1);
    graphicsLayout->setRowStretch(3, 1);
    graphicsWidget->setLayout(graphicsLayout);


    //Plugins tab
    pluginsWidget = new QWidget(this);
    pluginsLayout = new QGridLayout(pluginsWidget);

    pluginsDir = QDir(SETTINGS.value("Paths/plugins", "").toString());

    if (pluginsDir.exists()) {
        QStringList files = pluginsDir.entryList(QStringList() << "*", QDir::Files);

        if (files.size() > 0) {
            for(QStringList::Iterator it = files.begin(); it != files.end(); ++it)
            {
                QFile file(pluginsDir.absoluteFilePath(*it));
                QString fileName = QFileInfo(file).fileName();

                if (fileName.contains("-audio-"))
                    audioPlugins << fileName;
                else if (fileName.contains("-input-"))
                    inputPlugins << fileName;
                else if (fileName.contains("-rsp-"))
                    rspPlugins << fileName;
                else if (fileName.contains("-video-"))
                    videoPlugins << fileName;
            }
        }
    }

    videoLabel = new QLabel(tr("Video Plugin:"), this);
    audioLabel = new QLabel(tr("Audio Plugin:"), this);
    inputLabel = new QLabel(tr("Input Plugin:"), this);
    rspLabel = new QLabel(tr("RSP Plugin:"), this);

    videoBox = new QComboBox(this);
    audioBox = new QComboBox(this);
    inputBox = new QComboBox(this);
    rspBox = new QComboBox(this);

    videoBox->insertItems(0, videoPlugins);
    audioBox->insertItems(0, audioPlugins);
    inputBox->insertItems(0, inputPlugins);
    rspBox->insertItems(0, rspPlugins);

    int videoIndex = videoPlugins.indexOf(SETTINGS.value("Plugins/video","").toString());
    int audioIndex = videoPlugins.indexOf(SETTINGS.value("Plugins/audio","").toString());
    int inputIndex = videoPlugins.indexOf(SETTINGS.value("Plugins/input","").toString());
    int rspIndex = videoPlugins.indexOf(SETTINGS.value("Plugins/rsp","").toString());

    if (videoIndex >= 0) videoBox->setCurrentIndex(videoIndex);
    if (audioIndex >= 0) audioBox->setCurrentIndex(audioIndex);
    if (inputIndex >= 0) inputBox->setCurrentIndex(inputIndex);
    if (rspIndex >= 0) rspBox->setCurrentIndex(rspIndex);

    pluginsLayout->addWidget(videoLabel, 0, 0);
    pluginsLayout->addWidget(audioLabel, 1, 0);
    pluginsLayout->addWidget(inputLabel, 2, 0);
    pluginsLayout->addWidget(rspLabel, 3, 0);

    pluginsLayout->addWidget(videoBox, 0, 1);
    pluginsLayout->addWidget(audioBox, 1, 1);
    pluginsLayout->addWidget(inputBox, 2, 1);
    pluginsLayout->addWidget(rspBox, 3, 1);

    pluginsLayout->setColumnMinimumWidth(0, 100);
    pluginsLayout->setColumnStretch(1, 1);
    pluginsLayout->setRowStretch(4, 1);
    pluginsWidget->setLayout(pluginsLayout);


    tabWidget->addTab(pathsWidget, "Paths");
    tabWidget->addTab(emulationWidget, "Emulation");
    tabWidget->addTab(graphicsWidget, "Graphics");
    tabWidget->addTab(pluginsWidget, "Plugins");

    tabWidget->setCurrentIndex(activeTab);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                     Qt::Horizontal, this);

    layout->addWidget(tabWidget);
    layout->addWidget(buttonBox);

    connect(mupen64Button, SIGNAL(clicked()), this, SLOT(browseMupen64()));
    connect(pluginButton, SIGNAL(clicked()), this, SLOT(browsePlugin()));
    connect(dataButton, SIGNAL(clicked()), this, SLOT(browseData()));
    connect(configButton, SIGNAL(clicked()), this, SLOT(browseConfig()));
    connect(romButton, SIGNAL(clicked()), this, SLOT(browseROM()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(editSettings()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void SettingsDialog::browseMupen64()
{
    QString path = QFileDialog::getOpenFileName(this);
    if (path != "")
        mupen64Path->setText(path);

}

void SettingsDialog::browsePlugin()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "")
        pluginPath->setText(path);

}

void SettingsDialog::browseData()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "")
        dataPath->setText(path);
}

void SettingsDialog::browseConfig()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "")
        configPath->setText(path);
}

void SettingsDialog::browseROM()
{
    QString path = QFileDialog::getExistingDirectory(this);
    if (path != "")
        romPath->setText(path);
}

void SettingsDialog::editSettings()
{
    SETTINGS.setValue("Paths/mupen64plus", mupen64Path->text());
    SETTINGS.setValue("Paths/roms", romPath->text());
    SETTINGS.setValue("Paths/plugins", pluginPath->text());
    SETTINGS.setValue("Paths/data", dataPath->text());
    SETTINGS.setValue("Paths/config", configPath->text());

    if (pureButton->isChecked())
        SETTINGS.setValue("Emulation/mode", "0");
    else if (cachedButton->isChecked())
        SETTINGS.setValue("Emulation/mode", "1");
    else
        SETTINGS.setValue("Emulation/mode", "2");

    if (osdOption->isChecked())
        SETTINGS.setValue("Graphics/osd", true);
    else
        SETTINGS.setValue("Graphics/osd", "");

    if (fullscreenOption->isChecked())
        SETTINGS.setValue("Graphics/fullscreen", true);
    else
        SETTINGS.setValue("Graphics/fullscreen", "");

    if (resolutionBox->currentText() != "default")
        SETTINGS.setValue("Graphics/resolution", resolutionBox->currentText());
    else
        SETTINGS.setValue("Graphics/resolution", "");

    SETTINGS.setValue("Plugins/video", videoBox->currentText());
    SETTINGS.setValue("Plugins/audio", audioBox->currentText());
    SETTINGS.setValue("Plugins/input", inputBox->currentText());
    SETTINGS.setValue("Plugins/rsp", rspBox->currentText());

    close();
}
