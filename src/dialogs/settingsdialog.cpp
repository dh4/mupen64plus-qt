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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "../global.h"
#include "../common.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QListWidget>
#include <QMouseEvent>
#include <QStandardPaths>
#include <QTranslator>

#include <QDebug>

constexpr const char* CONTROL_BUTTON_EMPTY_TEXT = "Select...";

SettingsDialog::SettingsDialog(QWidget *parent, int activeTab) : QDialog(parent), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(activeTab);

    //Populate Paths tab
    ui->emulatorPath->setText(SETTINGS.value("Paths/mupen64plus", "").toString());
    ui->pluginPath->setText(SETTINGS.value("Paths/plugins", "").toString());
    ui->dataPath->setText(SETTINGS.value("Paths/data", "").toString());
    ui->configPath->setText(SETTINGS.value("Paths/config", "").toString());

    QStringList romDirectories = SETTINGS.value("Paths/roms", "").toString().split("|");
    romDirectories.removeAll("");
    foreach (QString directory, romDirectories)
        ui->romList->addItem(directory);

    connect(ui->emulatorButton, SIGNAL(clicked()), this, SLOT(browseEmulator()));
    connect(ui->pluginButton, SIGNAL(clicked()), this, SLOT(browsePlugin()));
    connect(ui->dataButton, SIGNAL(clicked()), this, SLOT(browseData()));
    connect(ui->configButton, SIGNAL(clicked()), this, SLOT(browseConfig()));
    connect(ui->romAddButton, SIGNAL(clicked()), this, SLOT(addRomDirectory()));
    connect(ui->romRemoveButton, SIGNAL(clicked()), this, SLOT(removeRomDirectory()));


    //Populate Emulation tab
    QString emuMode = SETTINGS.value("Emulation/mode", "").toString();
    if (emuMode == "0")
        ui->pureButton->setChecked(true);
    else if (emuMode == "1")
        ui->cachedButton->setChecked(true);
    else
        ui->dynamicButton->setChecked(true);


    //Populate Graphics tab
    if (SETTINGS.value("Graphics/osd", "true").toString() == "true")
        ui->osdOption->setChecked(true);
    if (SETTINGS.value("Graphics/fullscreen", "").toString() == "true")
        ui->fullscreenOption->setChecked(true);

    QStringList useableModes, modes;
    useableModes << tr("default"); //Allow users to use the screen resolution set in the config file

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
    int audioIndex = audioPlugins.indexOf(SETTINGS.value("Plugins/audio","").toString());
    int inputIndex = inputPlugins.indexOf(SETTINGS.value("Plugins/input","").toString());
    int rspIndex = rspPlugins.indexOf(SETTINGS.value("Plugins/rsp","").toString());

    if (videoIndex >= 0) ui->videoBox->setCurrentIndex(videoIndex);
    if (audioIndex >= 0) ui->audioBox->setCurrentIndex(audioIndex);
    if (inputIndex >= 0) ui->inputBox->setCurrentIndex(inputIndex);
    if (rspIndex >= 0) ui->rspBox->setCurrentIndex(rspIndex);


    //Populate Table tab
    int tableSizeIndex = 0;
    QString currentTableSize = SETTINGS.value("Table/imagesize","Medium").toString();

    QList<QStringList> sizes;
    sizes << (QStringList() << tr("Extra Small") << "Extra Small")
          << (QStringList() << tr("Small")       << "Small")
          << (QStringList() << tr("Medium")      << "Medium")
          << (QStringList() << tr("Large")       << "Large")
          << (QStringList() << tr("Extra Large") << "Extra Large")
          << (QStringList() << tr("Super")       << "Super");


    if (SETTINGS.value("Other/downloadinfo", "").toString() == "true")
        populateTableAndListTab(true);
    else
        populateTableAndListTab(false);

    if (SETTINGS.value("Table/stretchfirstcolumn", "true").toString() == "true")
        ui->tableStretchOption->setChecked(true);

    for (int i = 0; i < sizes.length(); i++)
    {
        ui->tableSizeBox->insertItem(i, sizes.at(i).at(0), sizes.at(i).at(1));
        if (currentTableSize == sizes.at(i).at(1))
            tableSizeIndex = i;
    }
    if (tableSizeIndex >= 0) ui->tableSizeBox->setCurrentIndex(tableSizeIndex);

    connect(ui->tableAddButton, SIGNAL(clicked()), this, SLOT(tableAddColumn()));
    connect(ui->tableRemoveButton, SIGNAL(clicked()), this, SLOT(tableRemoveColumn()));
    connect(ui->tableSortUpButton, SIGNAL(clicked()), this, SLOT(tableSortUp()));
    connect(ui->tableSortDownButton, SIGNAL(clicked()), this, SLOT(tableSortDown()));


    //Populate Grid tab
    int gridSizeIndex = 0, activeIndex = 0, inactiveIndex = 0, labelColorIndex = 0, bgThemeIndex = 0;
    QString currentGridSize = SETTINGS.value("Grid/imagesize","Medium").toString();
    QString currentActiveColor = SETTINGS.value("Grid/activecolor","Cyan").toString();
    QString currentInactiveColor = SETTINGS.value("Grid/inactivecolor","Black").toString();
    QString currentLabelColor = SETTINGS.value("Grid/labelcolor","White").toString();
    QString currentBGTheme = SETTINGS.value("Grid/theme","Normal").toString();

    QList<QStringList> colors;
    colors << (QStringList() << tr("Black")      << "Black")
           << (QStringList() << tr("White")      << "White")
           << (QStringList() << tr("Light Gray") << "Light Gray")
           << (QStringList() << tr("Dark Gray")  << "Dark Gray")
           << (QStringList() << tr("Green")      << "Green")
           << (QStringList() << tr("Cyan")       << "Cyan")
           << (QStringList() << tr("Blue")       << "Blue")
           << (QStringList() << tr("Purple")     << "Purple")
           << (QStringList() << tr("Red")        << "Red")
           << (QStringList() << tr("Pink")       << "Pink")
           << (QStringList() << tr("Orange")     << "Orange")
           << (QStringList() << tr("Yellow")     << "Yellow")
           << (QStringList() << tr("Brown")      << "Brown");

    QList<QStringList> bgThemes;
    bgThemes << (QStringList() << tr("Light")   << "Light")
             << (QStringList() << tr("Normal")  << "Normal")
             << (QStringList() << tr("Dark")    << "Dark");

    for (int i = 0; i < sizes.length(); i++)
    {
        ui->gridSizeBox->insertItem(i, sizes.at(i).at(0), sizes.at(i).at(1));
        if (currentGridSize == sizes.at(i).at(1))
            gridSizeIndex = i;
    }
    if (gridSizeIndex >= 0) ui->gridSizeBox->setCurrentIndex(gridSizeIndex);

    int gridColumnCount = SETTINGS.value("Grid/columncount","4").toInt();
    ui->columnCountBox->setValue(gridColumnCount);

    if (SETTINGS.value("Grid/autocolumns", "true").toString() == "true") {
        toggleGridColumn(true);
        ui->autoColumnOption->setChecked(true);
    } else
        toggleGridColumn(false);

    for (int i = 0; i < colors.length(); i++)
    {
        ui->shadowActiveBox->insertItem(i, colors.at(i).at(0), colors.at(i).at(1));
        if (currentActiveColor == colors.at(i).at(1))
            activeIndex = i;

        ui->shadowInactiveBox->insertItem(i, colors.at(i).at(0), colors.at(i).at(1));
        if (currentInactiveColor == colors.at(i).at(1))
            inactiveIndex = i;

        ui->labelColorBox->insertItem(i, colors.at(i).at(0), colors.at(i).at(1));
        if (currentLabelColor == colors.at(i).at(1))
            labelColorIndex = i;
    }
    if (activeIndex >= 0) ui->shadowActiveBox->setCurrentIndex(activeIndex);
    if (inactiveIndex >= 0) ui->shadowInactiveBox->setCurrentIndex(inactiveIndex);
    if (labelColorIndex >= 0) ui->labelColorBox->setCurrentIndex(labelColorIndex);

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

    for (int i = 0; i < bgThemes.length(); i++)
    {
        ui->bgThemeBox->insertItem(i, bgThemes.at(i).at(0), bgThemes.at(i).at(1));
        if (currentBGTheme == bgThemes.at(i).at(1))
            bgThemeIndex = i;
    }
    if (bgThemeIndex >= 0) ui->bgThemeBox->setCurrentIndex(bgThemeIndex);

    QString imagePath = SETTINGS.value("Grid/background", "").toString();
    ui->backgroundPath->setText(imagePath);
    hideBGTheme(imagePath);

    if (SETTINGS.value("Grid/sortdirection", "ascending").toString() == "descending")
        ui->gridDescendingOption->setChecked(true);

    connect(ui->autoColumnOption, SIGNAL(toggled(bool)), this, SLOT(toggleGridColumn(bool)));
    connect(ui->backgroundPath, SIGNAL(textChanged(QString)), this, SLOT(hideBGTheme(QString)));
    connect(ui->backgroundButton, SIGNAL(clicked()), this, SLOT(browseBackground()));
    connect(ui->labelOption, SIGNAL(toggled(bool)), this, SLOT(toggleLabel(bool)));


    //Populate List tab
    int listSizeIndex = 0, listTextIndex = 0, listThemeIndex = 0;
    QString currentListSize = SETTINGS.value("List/imagesize","Medium").toString();
    QString currentListText = SETTINGS.value("List/textsize","Medium").toString();
    QString currentListTheme = SETTINGS.value("List/theme","Light").toString();

    QList<QStringList> themes;
    themes << (QStringList() << tr("Light") << "Light")
           << (QStringList() << tr("Dark")  << "Dark");

    listCoverEnable << ui->listSizeLabel
                    << ui->listSizeBox;

    if (SETTINGS.value("List/displaycover", "").toString() == "true") {
        toggleListCover(true);
        ui->listCoverOption->setChecked(true);
    } else
        toggleListCover(false);

    if (SETTINGS.value("List/firstitemheader", "true").toString() == "true")
        ui->listHeaderOption->setChecked(true);

    for (int i = 0; i < sizes.length(); i++)
    {
        ui->listSizeBox->insertItem(i, sizes.at(i).at(0), sizes.at(i).at(1));
        if (currentListSize == sizes.at(i).at(1))
            listSizeIndex = i;
        ui->listTextBox->insertItem(i, sizes.at(i).at(0), sizes.at(i).at(1));
        if (currentListText == sizes.at(i).at(1))
            listTextIndex = i;
    }
    if (listSizeIndex >= 0) ui->listSizeBox->setCurrentIndex(listSizeIndex);
    if (listTextIndex >= 0) ui->listTextBox->setCurrentIndex(listTextIndex);

    for (int i = 0; i < themes.length(); i++)
    {
        ui->listThemeBox->insertItem(i, themes.at(i).at(0), themes.at(i).at(1));
        if (currentListTheme == themes.at(i).at(1))
            listThemeIndex = i;
    }
    if (listThemeIndex >= 0) ui->listThemeBox->setCurrentIndex(listThemeIndex);

    if (SETTINGS.value("List/sortdirection", "ascending").toString() == "descending")
        ui->listDescendingOption->setChecked(true);


    connect(ui->listCoverOption, SIGNAL(toggled(bool)), this, SLOT(toggleListCover(bool)));
    connect(ui->listAddButton, SIGNAL(clicked()), this, SLOT(listAddColumn()));
    connect(ui->listRemoveButton, SIGNAL(clicked()), this, SLOT(listRemoveColumn()));
    connect(ui->listSortUpButton, SIGNAL(clicked()), this, SLOT(listSortUp()));
    connect(ui->listSortDownButton, SIGNAL(clicked()), this, SLOT(listSortDown()));


    //Populate Other tab
    int languageIndex = 0;
    QString currentLanguage = SETTINGS.value("language", getDefaultLanguage()).toString();

    QList<QStringList> languages;
    languages << (QStringList() << QString::fromUtf8("English")  << "EN")
              << (QStringList() << QString::fromUtf8("Français") << "FR")
              << (QStringList() << QString::fromUtf8("Русский") << "RU");

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

    ui->parametersLine->setText(SETTINGS.value("Other/parameters", "").toString());

    for (int i = 0; i < languages.length(); i++)
    {
        ui->languageBox->insertItem(i, languages.at(i).at(0), languages.at(i).at(1));
        if (currentLanguage == languages.at(i).at(1))
            languageIndex = i;
    }
    ui->languageBox->setCurrentIndex(languageIndex);

    ui->languageInfoLabel->setHidden(true);

    //Populate Controls tab
    {
        /// 1. Init Maps keys
        mapControlButtonToControlKey = {
            {ui->btnDPadU, "DPad U"}, {ui->btnDPadD, "DPad D"}, {ui->btnDPadL, "DPad L"}, {ui->btnDPadR, "DPad R"},
            {ui->btnStart, "Start"}, {ui->btnLTrig, "L Trig"}, {ui->btnRTrig, "R Trig"}, {ui->btnZTrig, "Z Trig"},
            {ui->btnCBtnU, "C Button U"}, {ui->btnCBtnD, "C Button D"}, {ui->btnCBtnL, "C Button L"}, {ui->btnCBtnR, "C Button R"},
            {ui->btnABtn, "A Button"}, {ui->btnBBtn, "B Button"},
            {ui->btnMempak, "Mempak switch"}, {ui->btnRumblepak, "Rumblepak switch"},
            {ui->btnXAxis, "X Axis"}, {ui->btnYAxis, "Y Axis"},
        };

        /// 2. Load local config to UI upon selection of a Control config
        connect(ui->cboController, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int nbController)
        {
            const QMap<QString, QVariant>& cfg = controlsConfig[nbController];

            ui->chkMouse->setChecked(cfg.value("mouse", false).toBool());
            ui->chkPlugged->setChecked(cfg.value("plugged", false).toBool());

            ui->cboPlugin->setCurrentIndex(cfg.value("plugin", 1).toInt());
            ui->cboDevice->setCurrentText(cfg.value("name", QString()).toString()); // ui->cboDevice->setCurrentIndex(cfg.value("device", 0).toInt()); // The name determines the device..
            ui->cboMode->setCurrentIndex(cfg.value("mode", 0).toInt());            

            QStringList analogDeadzone = cfg.value("AnalogDeadzone", "0,0").toString().split(',');
            ui->nbAnalogDeadzoneX->setValue(analogDeadzone[0].toInt());
            ui->nbAnalogDeadzoneY->setValue(analogDeadzone[1].toInt());

            QStringList analogPeak = cfg.value("AnalogPeak", "32768,32768").toString().split(',');
            ui->nbAnalogPeakX->setValue(analogPeak[0].toInt());
            ui->nbAnalogPeakY->setValue(analogPeak[1].toInt());

            QStringList mouseSensitivity = cfg.value("MouseSensitivity", "2.0,2.0").toString().split(',');
            ui->nbMouseSensitivityX->setValue(mouseSensitivity[0].toInt());
            ui->nbMouseSensitivityY->setValue(mouseSensitivity[1].toInt());

            QMap<QPushButton*, QString>::const_iterator iter = mapControlButtonToControlKey.constBegin();
            while (iter != mapControlButtonToControlKey.constEnd())
            {
                iter.key()->setText(cfg.value(iter.value(), CONTROL_BUTTON_EMPTY_TEXT).toString());
                ++iter;
            }
        });

        /// 3. Update local config upon UI change
        connect(ui->chkMouse, &QCheckBox::toggled, this, [this] (bool mouseEnabled) { controlsConfig[ui->cboController->currentIndex()]["mouse"] = mouseEnabled; });
        connect(ui->chkPlugged, &QCheckBox::toggled, this, [this] (bool plugged) { controlsConfig[ui->cboController->currentIndex()]["plugged"] = plugged; });

        connect(ui->cboPlugin, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] (int plugin) { controlsConfig[ui->cboController->currentIndex()]["plugin"] = plugin; });
        connect(ui->cboDevice, &QComboBox::currentTextChanged, this, [this] (const QString& device)
        {
            if(sdlJoystick)
                SDL_JoystickClose(sdlJoystick);

            sdlJoystick = SDL_JoystickOpen(ui->cboDevice->currentIndex());

            controlsConfig[ui->cboController->currentIndex()]["name"] = device;
        });
        connect(ui->cboMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] (int mode) { controlsConfig[ui->cboController->currentIndex()]["mode"] = mode; });

        connect(ui->nbAnalogDeadzoneX, QOverload<int>::of(&QSpinBox::valueChanged), this, [this] (int analogDeadzoneX) { controlsConfig[ui->cboController->currentIndex()]["AnalogDeadzone"] = QString("%1,%2").arg(analogDeadzoneX).arg(ui->nbAnalogDeadzoneY->value()); });
        connect(ui->nbAnalogDeadzoneY, QOverload<int>::of(&QSpinBox::valueChanged), this, [this] (int analogDeadzoneY) { controlsConfig[ui->cboController->currentIndex()]["AnalogDeadzone"] = QString("%1,%2").arg(ui->nbAnalogDeadzoneX->value()).arg(analogDeadzoneY); });

        connect(ui->nbAnalogPeakX, QOverload<int>::of(&QSpinBox::valueChanged), this, [this] (int analogPeakX) { controlsConfig[ui->cboController->currentIndex()]["AnalogPeak"] = QString("%1,%2").arg(analogPeakX).arg(ui->nbAnalogDeadzoneY->value()); });
        connect(ui->nbAnalogPeakY, QOverload<int>::of(&QSpinBox::valueChanged), this, [this] (int analogPeakY) { controlsConfig[ui->cboController->currentIndex()]["AnalogPeak"] = QString("%1,%2").arg(ui->nbAnalogDeadzoneX->value()).arg(analogPeakY); });

        connect(ui->nbMouseSensitivityX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this] (int mouseSensitivityX) { controlsConfig[ui->cboController->currentIndex()]["MouseSensitivity"] = QString("%1,%2").arg(mouseSensitivityX).arg(ui->nbMouseSensitivityY->value()); });
        connect(ui->nbMouseSensitivityY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this] (int mouseSensitivityY) { controlsConfig[ui->cboController->currentIndex()]["MouseSensitivity"] = QString("%1,%2").arg(ui->nbMouseSensitivityX->value()).arg(mouseSensitivityY); });

        for(QPushButton* btn : {ui->btnDPadU, ui->btnDPadD, ui->btnDPadL, ui->btnDPadR, ui->btnStart, ui->btnLTrig, ui->btnRTrig, ui->btnZTrig, ui->btnCBtnU, ui->btnCBtnD, ui->btnCBtnL, ui->btnCBtnR, ui->btnABtn, ui->btnBBtn, ui->btnMempak, ui->btnRumblepak, ui->btnXAxis, ui->btnYAxis })
        {
            connect(btn, &QPushButton::clicked, this, [this, btn] (bool checked)
            {
                if(checked)
                {
                    if(focusedControlButton)
                        focusedControlButton->setChecked(false);
                    focusedControlButton = btn;
                }
                else
                {
                    focusedControlButton = nullptr;
                }
            });
        }

        /// 4. Browse Shared Data Path
        connect(ui->btnBrowseSharedDataPath, &QPushButton::clicked, this, [this]()
        {
           const QString path = QFileDialog::getExistingDirectory(this, "Choose Shared Data Path", QString());

           if(! path.isNull())
               ui->txtSharedDataPath->setText(path);
        });

        /// 5. Load config from AutoInputCFG.cfg whenever the Shared Data Path changes
        connect(ui->txtSharedDataPath, &QLineEdit::textChanged, this, [this](const QString& sharedDataPath)
        {
            QString configPath = sharedDataPath;

            // Ensure the text is never empty
            if(configPath.isEmpty())
            {
                // Create shared data path if doesn't exists
                configPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
                configPath += "/.config/mupen64plus";
                QDir().mkpath(configPath);

                // Set the text in UI and prevent calling this slot again
                ui->txtSharedDataPath->blockSignals(true);
                ui->txtSharedDataPath->setText(configPath);
                ui->txtSharedDataPath->blockSignals(false);
            }

            // Clear local config
            for(int i = 0; i < 4; i++)
            {
                controlsConfig[i] = {
                    {"mouse", false},
                    {"plugged", false},
                    {"plugin", 1},
                    {"name", QString()},
                    {"mode", 0},
                    {"AnalogDeadzone", "0,0"},
                    {"AnalogPeak", "32768,32768"},
                    {"MouseSensitivity", "2.0,2.0"},
                };

                QMap<QPushButton*, QString>::const_iterator iter = mapControlButtonToControlKey.constBegin();

                while (iter != mapControlButtonToControlKey.constEnd())
                {
                    controlsConfig[i][iter.value()] = CONTROL_BUTTON_EMPTY_TEXT;
                    ++iter;
                }
            }

            // Load from AutoInputCFG.cfg to local config
            QFile configFile(configPath + "/AutoInputCFG.cfg");

            if (configFile.open(QFile::ReadOnly))
            {
                QTextStream stream(&configFile);

                QString line;
                int controlId = 1;
                int nbLine = 0;

                while (stream.readLineInto(&line))
                {
                    line = line.simplified();

                    // [Input-SDL-Control1], [Input-SDL-Control2]...
                    if(line.size() == 20 && line.startsWith('[') && line.endsWith(']'))
                    {
                        controlId = line[18].digitValue();
                    }
                    else
                    {
                        QStringList keyValue = line.split("=");

                        if(keyValue.size() < 2)
                        {
                            qDebug() << "Invalid INI format at line" << nbLine << ":" << line << keyValue;
                            continue;
                        }

                        keyValue[0] = keyValue[0].trimmed();
                        keyValue[1] = keyValue[1].trimmed();

                        if(keyValue[1].isEmpty())
                            keyValue[1] = CONTROL_BUTTON_EMPTY_TEXT;

                        controlsConfig[controlId - 1][keyValue[0]] = keyValue[1];
                    }

                    nbLine++;
                }
            }

            // Load from local config to UI
            emit ui->cboController->currentIndexChanged(0);
        });


        /// 6. Init joystick (device) list
        SDL_Init(SDL_INIT_JOYSTICK);

        for(int i = 0; i < SDL_NumJoysticks(); i++)
        {
            ui->cboDevice->addItem( SDL_JoystickNameForIndex(i) );
        }

        /// 7. Init shared data path UI
        ui->txtSharedDataPath->setText(SETTINGS.value("Controls/sharedDataPath", QString()).toString());

        /// 8. Start joystick events pump
        sdlEventsPumpTimerId = startTimer(10, Qt::VeryCoarseTimer);
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    connect(ui->downloadOption, SIGNAL(toggled(bool)), this, SLOT(toggleDownload(bool)));
    connect(ui->downloadOption, SIGNAL(toggled(bool)), this, SLOT(populateTableAndListTab(bool)));
    connect(ui->languageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateLanguageInfo()));    

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

    if (row >= 0)
        currentList->addItem(availableList->takeItem(row));
}


void SettingsDialog::addRomDirectory()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("ROM Directory"));
    if (path != "") {
        //check for duplicates
        bool found = false;
        for (int i = 0; i < ui->romList->count(); i++)
            if (path == ui->romList->item(i)->text())
                found = true;

        if (!found)
            ui->romList->addItem(path);
    }
}


void SettingsDialog::browseBackground()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Background Image"));
    if (path != "")
        ui->backgroundPath->setText(path);
}


void SettingsDialog::browseEmulator()
{
    QString path = QFileDialog::getOpenFileName(this, tr("<ParentName> Executable")
                                                .replace("<ParentName>",ParentName));
    if (path != "")
        ui->emulatorPath->setText(path);

#ifdef Q_OS_OSX
    //Allow OSX users to just select the .app directory and auto-populate for them
    if (path.right(15) == "mupen64plus.app") {
        ui->emulatorPath->setText(path+"/Contents/MacOS/mupen64plus");
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
    SETTINGS.setValue("Paths/mupen64plus", ui->emulatorPath->text());
    SETTINGS.setValue("Paths/plugins", ui->pluginPath->text());
    SETTINGS.setValue("Paths/data", ui->dataPath->text());
    SETTINGS.setValue("Paths/config", ui->configPath->text());

    QStringList romDirectories;
    for (int i = 0; i < ui->romList->count(); i++)
        romDirectories << ui->romList->item(i)->text();

    SETTINGS.setValue("Paths/roms", romDirectories.join("|"));


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
    for (int i = 0; i < ui->tableCurrentList->count(); i++)
        if (available.contains(ui->tableCurrentList->item(i)->data(Qt::UserRole).toString()))
            tableVisibleItems << ui->tableCurrentList->item(i)->data(Qt::UserRole).toString();

    SETTINGS.setValue("Table/columns", tableVisibleItems.join("|"));

    if (ui->tableStretchOption->isChecked())
        SETTINGS.setValue("Table/stretchfirstcolumn", true);
    else
        SETTINGS.setValue("Table/stretchfirstcolumn", "");

    SETTINGS.setValue("Table/imagesize", ui->tableSizeBox->itemData(ui->tableSizeBox->currentIndex()));


    //Grid tab
    SETTINGS.setValue("Grid/imagesize", ui->gridSizeBox->itemData(ui->gridSizeBox->currentIndex()));
    SETTINGS.setValue("Grid/columncount", ui->columnCountBox->value());

    if (ui->autoColumnOption->isChecked())
        SETTINGS.setValue("Grid/autocolumns", true);
    else
        SETTINGS.setValue("Grid/autocolumns", "");

    SETTINGS.setValue("Grid/inactivecolor", ui->shadowInactiveBox->itemData(ui->shadowInactiveBox->currentIndex()));
    SETTINGS.setValue("Grid/activecolor", ui->shadowActiveBox->itemData(ui->shadowActiveBox->currentIndex()));
    SETTINGS.setValue("Grid/theme", ui->bgThemeBox->itemData(ui->bgThemeBox->currentIndex()));
    SETTINGS.setValue("Grid/background", ui->backgroundPath->text());

    if (ui->labelOption->isChecked())
        SETTINGS.setValue("Grid/label", true);
    else
        SETTINGS.setValue("Grid/label", "");

    SETTINGS.setValue("Grid/labeltext", ui->labelTextBox->itemData(ui->labelTextBox->currentIndex()));
    SETTINGS.setValue("Grid/labelcolor", ui->labelColorBox->itemData(ui->labelColorBox->currentIndex()));
    SETTINGS.setValue("Grid/sort", ui->gridSortBox->itemData(ui->gridSortBox->currentIndex()));

    if (ui->gridDescendingOption->isChecked())
        SETTINGS.setValue("Grid/sortdirection", "descending");
    else
        SETTINGS.setValue("Grid/sortdirection", "ascending");


    //List tab
    QStringList listVisibleItems;
    for (int i = 0; i <  ui->listCurrentList->count(); i++)
        if (available.contains(ui->listCurrentList->item(i)->data(Qt::UserRole).toString()))
            listVisibleItems << ui->listCurrentList->item(i)->data(Qt::UserRole).toString();

    SETTINGS.setValue("List/columns", listVisibleItems.join("|"));

    if (ui->listHeaderOption->isChecked())
        SETTINGS.setValue("List/firstitemheader", true);
    else
        SETTINGS.setValue("List/firstitemheader", "");

    if (ui->listCoverOption->isChecked() && ui->downloadOption->isChecked())
        SETTINGS.setValue("List/displaycover", true);
    else
        SETTINGS.setValue("List/displaycover", "");

    SETTINGS.setValue("List/imagesize", ui->listSizeBox->itemData(ui->listSizeBox->currentIndex()));
    SETTINGS.setValue("List/textsize", ui->listTextBox->itemData(ui->listTextBox->currentIndex()));
    SETTINGS.setValue("List/theme", ui->listThemeBox->itemData(ui->listThemeBox->currentIndex()));
    SETTINGS.setValue("List/sort", ui->listSortBox->itemData(ui->listSortBox->currentIndex()));

    if (ui->listDescendingOption->isChecked())
        SETTINGS.setValue("List/sortdirection", "descending");
    else
        SETTINGS.setValue("List/sortdirection", "ascending");


    //Other tab
    if (ui->saveOption->isChecked())
        SETTINGS.setValue("saveoptions", true);
    else
        SETTINGS.setValue("saveoptions", "");

    SETTINGS.setValue("Other/parameters", ui->parametersLine->text());
    SETTINGS.setValue("language", ui->languageBox->itemData(ui->languageBox->currentIndex()));

    //Controls tab
    {
        killTimer(sdlEventsPumpTimerId);
        SDL_JoystickClose(sdlJoystick);
        SDL_Quit();

        SETTINGS.setValue("Controls/sharedDataPath", ui->txtSharedDataPath->text());

        QFile configFile(ui->txtSharedDataPath->text() + "/AutoInputCFG.cfg");

        if (configFile.open(QFile::WriteOnly | QFile::Truncate))
        {
            QTextStream stream(&configFile);

            for(int i = 0; i < 4; i++)
            {
                if(controlsConfig[i].isEmpty())
                    continue;

                stream << "[Input-SDL-Control" << QString::number(i + 1) << "]\n";

                QMap<QString, QVariant>::const_iterator iter = controlsConfig[i].constBegin();
                while (iter != controlsConfig[i].constEnd())
                {
                    QString value = iter.value().toString();

                    if(iter.value().type() == QVariant::Bool)
                    {
                        value[0] = value[0].toUpper(); // false -> False, true -> True
                    }
                    else if(value == CONTROL_BUTTON_EMPTY_TEXT)
                    {
                        value.clear();
                    }

                    stream << iter.key() << " = " << value << "\n";
                    ++iter;
                }
            }
        }
    }


    close();
}


void SettingsDialog::hideBGTheme(QString imagePath)
{
    if (imagePath == "") {
        ui->bgThemeLabel->setEnabled(true);
        ui->bgThemeBox->setEnabled(true);
    } else {
        ui->bgThemeLabel->setEnabled(false);
        ui->bgThemeBox->setEnabled(false);
    }
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
              << "Zip File"
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
                  << "Game Cover";

        labelOptions << "Game Title"
                     << "Release Date"
                     << "Genre";

        sortOptions << "Game Title"
                    << "Release Date"
                    << "ESRB"
                    << "Genre"
                    << "Publisher"
                    << "Developer";
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
    foreach (QString listItem, tableAvailable)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(getTranslation(listItem));
        item->setData(Qt::UserRole, listItem);

        ui->tableAvailableList->addItem(item);
    }
    ui->tableAvailableList->sortItems();

    ui->tableCurrentList->clear();
    foreach (QString listItem, tableCurrent)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(getTranslation(listItem));
        item->setData(Qt::UserRole, listItem);

        ui->tableCurrentList->addItem(item);
    }


    //Grid sort field and label text
    int labelTextIndex = 0, gridSortIndex = 0;
    QString currentLabelText = SETTINGS.value("Grid/labeltext","Filename").toString();
    QString currentGridSort = SETTINGS.value("Grid/sort","Filename").toString();

    ui->labelTextBox->clear();
    for (int i = 0; i < labelOptions.length(); i++)
    {
        ui->labelTextBox->insertItem(i, getTranslation(labelOptions.at(i)), labelOptions.at(i));
        if (currentLabelText == labelOptions.at(i))
            labelTextIndex = i;
    }
    if (labelTextIndex >= 0) ui->labelTextBox->setCurrentIndex(labelTextIndex);

    ui->gridSortBox->clear();
    for (int i = 0; i < sortOptions.length(); i++)
    {
        ui->gridSortBox->insertItem(i, getTranslation(sortOptions.at(i)), sortOptions.at(i));
        if (currentGridSort == sortOptions.at(i))
            gridSortIndex = i;
    }
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
    foreach (QString listItem, listAvailable)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(getTranslation(listItem));
        item->setData(Qt::UserRole, listItem);

        ui->listAvailableList->addItem(item);
    }
    ui->listAvailableList->sortItems();

    ui->listCurrentList->clear();
    foreach (QString listItem, listCurrent)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(getTranslation(listItem));
        item->setData(Qt::UserRole, listItem);

        ui->listCurrentList->addItem(item);
    }

    int listSortIndex = 0;
    QString currentListSort = SETTINGS.value("List/sort","Filename").toString();

    ui->listSortBox->clear();
    for (int i = 0; i < sortOptions.length(); i++)
    {
        ui->listSortBox->insertItem(i, getTranslation(sortOptions.at(i)), sortOptions.at(i));
        if (currentListSort == sortOptions.at(i))
            listSortIndex = i;
    }
    if (listSortIndex >= 0) ui->listSortBox->setCurrentIndex(listSortIndex);
}


void SettingsDialog::removeColumn(QListWidget *currentList, QListWidget *availableList)
{
    int row = currentList->currentRow();

    if (row >= 0) {
        availableList->addItem(currentList->takeItem(row));
        availableList->sortItems();
    }
}


void SettingsDialog::removeRomDirectory()
{
    int row = ui->romList->currentRow();

    if (row >= 0)
        delete ui->romList->takeItem(row);
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


void SettingsDialog::toggleGridColumn(bool active)
{
    if (active)
        ui->columnCountBox->setEnabled(false);
    else
        ui->columnCountBox->setEnabled(true);
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


void SettingsDialog::updateLanguageInfo()
{
    ui->languageInfoLabel->setHidden(false);

    const char *sourceText = "<b>Note:</b> Language changes will not take place until application restart.";

    QTranslator translator;
    QString language = ui->languageBox->itemData(ui->languageBox->currentIndex()).toString().toLower();
    QString resource = ":/locale/"+AppNameLower+"_"+language+".qm";
    if (QFileInfo(resource).exists()) {
        translator.load(resource);
        ui->languageInfoLabel->setText(translator.translate("SettingsDialog", sourceText));
    } else
        ui->languageInfoLabel->setText(sourceText);
}

void SettingsDialog::inputEvent(const QString &eventType, const QString &eventData)
{
    bool forAxis = focusedControlButton == ui->btnXAxis || focusedControlButton == ui->btnYAxis;

    if(forAxis && eventType == "mouse")
        return; // Axis don't support mouse events

    QString inputString;

    if(forAxis)
    {
        if(controlAxisFirstEvent.name == eventType)
        {
            inputString = QString("%1(%2,%3)").arg(eventType, controlAxisFirstEvent.param, eventData);
            controlAxisFirstEvent.name.clear();
        }
        else
        {
            controlAxisFirstEvent.name = eventType;
            controlAxisFirstEvent.param = eventData;
        }
    }
    else
    {
        inputString = QString("%1(%2)").arg(eventType, eventData);
    }

    if(!inputString.isNull() && !focusedControlButton->text().contains(inputString))
    {
        if(focusedControlButton->text() == CONTROL_BUTTON_EMPTY_TEXT)
            focusedControlButton->setText(inputString);
        else
            focusedControlButton->setText(focusedControlButton->text() + " " + inputString);

        const QString controlKey = mapControlButtonToControlKey[focusedControlButton];

        focusedControlButton->setToolTip(controlKey + " = " + focusedControlButton->text());
        controlsConfig[ui->cboController->currentIndex()][controlKey] = focusedControlButton->text();

        focusedControlButton->click();
    }
}

void SettingsDialog::mousePressEvent(QMouseEvent *event)
{
    if(focusedControlButton)
    {
        inputEvent("mouse", QString::number(event->button() == Qt::MiddleButton ? 2 : event->button() == Qt::RightButton ? 3 : 1));
    }
}

void SettingsDialog::keyPressEvent(QKeyEvent *event)
{
    if(!focusedControlButton)
        return;

    if(event->key() == Qt::Key_Escape) //  == SDLK_ESCAPE
        focusedControlButton->click();
    else if (event->key() == Qt::Key_Backspace)
    {
        focusedControlButton->setText(CONTROL_BUTTON_EMPTY_TEXT);
        focusedControlButton->setToolTip(mapControlButtonToControlKey[focusedControlButton]);
        focusedControlButton->click();
    }
    else
    {
        //inputEvent("key", SDL_GetScancodeName(event->nativeScanCode()));
    }
}

void SettingsDialog::timerEvent(QTimerEvent *e)
{
    if(e->timerId() != sdlEventsPumpTimerId || focusedControlButton == nullptr)
        return;

    SDL_Event event;

    while(SDL_PollEvent(&event) == 1)
    {
        QString eventType, eventData;

        if (event.type == SDL_JOYAXISMOTION && abs(event.jaxis.value) >= 32767)
        {
            eventType = "axis";
            eventData = QString::number(event.jaxis.axis) + (event.jaxis.value > 0 ? "+" : "-");
        }
        else if (event.type == SDL_JOYHATMOTION)
        {

            eventType = "hat";
            eventData = QString::number(event.jhat.hat) + event.jhat.value == SDL_HAT_UP ? " Up" :
                                                           event.jhat.value == SDL_HAT_DOWN ? " Down" :
                                                           event.jhat.value == SDL_HAT_LEFT ? " Left" : " Right";
        }
        else if (event.type == SDL_JOYBUTTONDOWN)
        {
            eventType = "button";
            eventData = QString::number(event.jbutton.button);
        }

        if(! eventType.isNull())
            inputEvent(eventType, eventData);
    }
        SDL_Delay(20);
}
#include <iostream>
#include <fstream>
#include <algorithm>
void SettingsDialog::on_saveBtn_clicked()
{
if(1){
//https://www.walletfox.com/course/parseconfigfile.php
    std::ifstream cFile ("/home/netpipe/.config/mupen64plus/InputAutoCfg.ini");
   // /home/netpipe/.config/mupen64plus/InputAutoCfg.ini
    if (cFile.is_open())
    {
        std::string line;
       QString tstring;
        while(getline(cFile, line)){
            line.erase(std::remove_if(line.begin(), line.end(), isspace),
                                 line.end());
            if(line[0] == '#' || line.empty())
                continue;
            auto delimiterPos = line.find("=");
            auto name = line.substr(0, delimiterPos);
            auto value = line.substr(delimiterPos + 1);
            //std::cout << name << " " << value << '\n';
             std::string test;
            test = name;
             std::string test2 = value;
          //  test += value;
         //   qDebug() << name << " " << value;
             if ("DPadU" == name){
            tstring +=  ui->btnDPadU->text().toLatin1() + "\n";
         //    qDebug() << test.c_str() << " " << test2.c_str();
            }
             if ("DPadD" == name){
                 tstring +=  ui->btnDPadD->text().toLatin1() + "\n";
            }
             if ("DPadL" == name){
                 tstring +=  ui->btnDPadL->text().toLatin1() + "\n";

            }
             if ("DPadR" == name){
                 tstring +=  ui->btnDPadR->text().toLatin1() + "\n";
            }
             if ("CButtonU" == name){
                 tstring +=  ui->btnCBtnU->text().toLatin1() + "\n";

            }
             if ("CButtonD" == name){
                 tstring +=  ui->btnCBtnD->text().toLatin1() + "\n";
            }
             if ("CButtonR" == name){
                 tstring +=  ui->btnCBtnR->text().toLatin1() + "\n";

            }
             if ("CButtonL" == name){
                 tstring +=  ui->btnCBtnL->text().toLatin1() + "\n";
            }
             if ("AButton" == name){
                 tstring +=  ui->btnABtn->text().toLatin1() + "\n";

            }
             if ("BButton" == name){
                 tstring +=  ui->btnBBtn->text().toLatin1() + "\n";
            }
             if ("Start" == name){
                 tstring +=  ui->btnStart->text().toLatin1() + "\n";

            }
             if ("ZTrig" == name){
                 tstring +=  ui->btnZTrig->text().toLatin1() + "\n";
            }
             if ("LTrig" == name){
                 tstring +=  ui->btnLTrig->text().toLatin1() + "\n";

            }
             if ("RTrig" == name){
                 tstring +=  ui->btnRTrig->text().toLatin1() + "\n";
            }

        }
 qDebug() << tstring;
       // if (){

       // }
        // for each joystick found thats connected search these values

    }
    else {
        std::cerr << "Couldn't open config file for reading.\n";
    }

 //  qDebug() << ui->btnDPadU->text().toLatin1();
//    ui->btnDPadD->text()
//    ui->btnDPadL->text()
//    ui->btnDPadR->text()

//    ui->btnYAxis->text()
//    ui->btnXAxis->text()

//    ui->btnCBtnU->text()
//    ui->btnCBtnD->text()
//    ui->btnCBtnL->text()
//    ui->btnCBtnR->text()

//    ui->btnStart->text()
//    ui->btnZTrig->text()

//    ui->btnBBtn->text()
//    ui->btnABtn->text()
}
}



void SettingsDialog::on_loadCCFGBtn_2_clicked()
{
    std::ifstream cFile ("/home/netpipe/.config/mupen64plus/InputAutoCfg.ini");
   // /home/netpipe/.config/mupen64plus/InputAutoCfg.ini
    if (cFile.is_open())
    {
        std::string line;
       QString tstring;
        while(getline(cFile, line)){
            line.erase(std::remove_if(line.begin(), line.end(), isspace),
                                 line.end());
            if(line[0] == '#' || line.empty())
                continue;
            auto delimiterPos = line.find("=");
            auto name = line.substr(0, delimiterPos);
            auto value = line.substr(delimiterPos + 1);
            //std::cout << name << " " << value << '\n';
             std::string test;
            test = name;
             std::string test2 = value;
    if ("DPadU" == name){
   ui->btnDPadU->setText(test2.c_str());
//    qDebug() << test.c_str() << " " << test2.c_str();
   }
    if ("DPadD" == name){
       ui->btnDPadD->setText(test2.c_str());
   }
    if ("DPadL" == name){
        ui->btnDPadL->setText(test2.c_str());

   }
    if ("DPadR" == name){
       ui->btnDPadR->setText(test2.c_str());
   }
    if ("CButtonU" == name){
       ui->btnCBtnU->setText(test2.c_str());

   }
    if ("CButtonD" == name){
          ui->btnCBtnD->setText(test2.c_str());
   }
    if ("CButtonR" == name){
       ui->btnCBtnR->setText(test2.c_str());

   }
    if ("CButtonL" == name){
     ui->btnCBtnL->setText(test2.c_str());
   }
    if ("AButton" == name){
       ui->btnABtn->setText(test2.c_str());

   }
    if ("BButton" == name){
        ui->btnBBtn->setText(test2.c_str());
   }
    if ("Start" == name){
      ui->btnStart->setText(test2.c_str());

   }
    if ("ZTrig" == name){
          ui->btnZTrig->setText(test2.c_str());
   }
    if ("LTrig" == name){
      ui->btnLTrig->setText(test2.c_str());

   }
    if ("RTrig" == name){
      ui->btnRTrig->setText(test2.c_str());
   }
}
}
}
