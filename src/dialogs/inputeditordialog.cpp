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

#include "inputeditordialog.h"
#include "ui_inputeditordialog.h"

#include "../global.h"


InputEditorDialog::InputEditorDialog(QString configFile, QWidget *parent): QDialog(parent), ui(new Ui::InputEditorDialog)
{
    config.setFileName(configFile);
    ui->setupUi(this);


    /// Populate plugin drop down
    pluginOptions.insert(1, "None");
    pluginOptions.insert(2, "Mem pak");
    pluginOptions.insert(5, "Rumble pak");

    foreach (const QString &value, pluginOptions)
        ui->cboPlugin->addItem(value);


    /// Init input map
    mapControlButtonToControlKey = {
        {ui->btnDPadU, "DPad U"}, {ui->btnDPadD, "DPad D"}, {ui->btnDPadL, "DPad L"}, {ui->btnDPadR, "DPad R"},
        {ui->btnStart, "Start"}, {ui->btnLTrig, "L Trig"}, {ui->btnRTrig, "R Trig"}, {ui->btnZTrig, "Z Trig"},
        {ui->btnCBtnU, "C Button U"}, {ui->btnCBtnD, "C Button D"}, {ui->btnCBtnL, "C Button L"}, {ui->btnCBtnR, "C Button R"},
        {ui->btnABtn, "A Button"}, {ui->btnBBtn, "B Button"},
        {ui->btnMempak, "Mempak switch"}, {ui->btnRumblepak, "Rumblepak switch"},
        {ui->btnXAxis, "X Axis"}, {ui->btnYAxis, "Y Axis"},
    };


    /// Populate controlsConfig with empty values
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


    /// Grab current configuration from mupen64plus.cfg

    // Populate this and we'll remove them as they're found
    missingControllers << "[Input-SDL-Control1]" << "[Input-SDL-Control2]" << "[Input-SDL-Control3]" << "[Input-SDL-Control4]";

    if (config.open(QFile::ReadOnly)) {
        QTextStream stream(&config);

        QString line;
        int controlId = -1;
        bool controllerSection = false;

        while (stream.readLineInto(&line))
        {
            line = line.simplified();

            // [Input-SDL-Control1], [Input-SDL-Control2]...
            // Grab controller ID and set controllerSection to true
            if (line.trimmed().size() == 20 && line.startsWith("[Input-SDL-Control") && line.endsWith(']')) {
                controllerSection = true;
                missingControllers.removeOne(line.trimmed());
                controlId = line[18].digitValue();
            } else if (line.startsWith('[')) // Set other sections to false
                controllerSection = false;
            else if (controllerSection) {
                // Remove comments and split on the "="
                QStringList keyValue = line.split("#").first().split("=");

                if (keyValue.size() == 2) { // Else, not a config line
                    QString value = keyValue[1].trimmed().replace('"',"");
                    if (value.isEmpty())
                        value = CONTROL_BUTTON_EMPTY_TEXT;

                    controlsConfig[controlId - 1][keyValue[0].trimmed()] = value;
                }
            }
        }
        config.close();
    }

    if (missingControllers.size() == 0) {
        /// Load local config to UI upon selection of a Control config
        connect(ui->cboController, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int nbController) {
            if (!fromAutoChange && unsavedChanges) {
                int answer = QMessageBox::question(this, tr("Unsaved Changes"), tr("There are unsaved changes. Are you sure you want to leave?"),
                                                   QMessageBox::Yes | QMessageBox::No);

                if (answer == QMessageBox::Yes)
                    updateControllerConfig(nbController);
                else { // Revert to old selection, making no changes
                    fromAutoChange = true; // Set to true so the question is ignored on the automatic change
                    ui->cboController->setCurrentIndex(currentController);
                }
            } else if (fromAutoChange)
                fromAutoChange = false;
            else
                updateControllerConfig(nbController);
        });


        /// Update local config upon UI change
        connect(ui->chkMouse, &QCheckBox::toggled, this, [this] (bool mouseEnabled) {
            controlsConfig[ui->cboController->currentIndex()]["mouse"] = mouseEnabled;
            unsavedChanges = true;
        });

        connect(ui->chkPlugged, &QCheckBox::toggled, this, [this] (bool plugged) {
            controlsConfig[ui->cboController->currentIndex()]["plugged"] = plugged;
            unsavedChanges = true;
        });

        connect(ui->cboPlugin, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this, [this] (QString plugin) {
            controlsConfig[ui->cboController->currentIndex()]["plugin"] = pluginOptions.key(plugin);
            unsavedChanges = true;
        });

        connect(ui->cboDevice, &QComboBox::currentTextChanged, this, [this] (const QString& device) {
            controlsConfig[ui->cboController->currentIndex()]["name"] = device;
            unsavedChanges = true;
        });

        connect(ui->cboMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] (int mode) {
            controlsConfig[ui->cboController->currentIndex()]["mode"] = mode;
            unsavedChanges = true;
        });

        connect(ui->nbAnalogDeadzoneX, QOverload<int>::of(&QSpinBox::valueChanged), this, [this] (int analogDeadzoneX) {
            controlsConfig[ui->cboController->currentIndex()]["AnalogDeadzone"] = QString("%1,%2").arg(analogDeadzoneX).arg(ui->nbAnalogDeadzoneY->value());
            unsavedChanges = true;
        });

        connect(ui->nbAnalogDeadzoneY, QOverload<int>::of(&QSpinBox::valueChanged), this, [this] (int analogDeadzoneY) {
            controlsConfig[ui->cboController->currentIndex()]["AnalogDeadzone"] = QString("%1,%2").arg(ui->nbAnalogDeadzoneX->value()).arg(analogDeadzoneY);
            unsavedChanges = true;
        });

        connect(ui->nbAnalogPeakX, QOverload<int>::of(&QSpinBox::valueChanged), this, [this] (int analogPeakX) {
            controlsConfig[ui->cboController->currentIndex()]["AnalogPeak"] = QString("%1,%2").arg(analogPeakX).arg(ui->nbAnalogDeadzoneY->value());
            unsavedChanges = true;
        });

        connect(ui->nbAnalogPeakY, QOverload<int>::of(&QSpinBox::valueChanged), this, [this] (int analogPeakY) {
            controlsConfig[ui->cboController->currentIndex()]["AnalogPeak"] = QString("%1,%2").arg(ui->nbAnalogDeadzoneX->value()).arg(analogPeakY);
            unsavedChanges = true;
        });

        connect(ui->nbMouseSensitivityX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this] (int mouseSensitivityX) {
            controlsConfig[ui->cboController->currentIndex()]["MouseSensitivity"] = QString("%1,%2").arg(mouseSensitivityX).arg(ui->nbMouseSensitivityY->value());
            unsavedChanges = true;
        });

        connect(ui->nbMouseSensitivityY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this] (int mouseSensitivityY) {
            controlsConfig[ui->cboController->currentIndex()]["MouseSensitivity"] = QString("%1,%2").arg(ui->nbMouseSensitivityX->value()).arg(mouseSensitivityY);
            unsavedChanges = true;
        });


        /// Iterate through all the mapped control buttons and connect them
        QList<QPushButton*> btnList;
        for (auto it = mapControlButtonToControlKey.constBegin(); it != mapControlButtonToControlKey.constEnd(); ++it)
            btnList.append(it.key());

        for(QPushButton* btn : btnList)
        {
            connect(btn, &QPushButton::clicked, this, [this, btn] (bool checked) {
                if (checked) {
                    if (focusedControlButton)
                        focusedControlButton->setChecked(false);
                    focusedControlButton = btn;

                    /// Start joystick events timer
                    SDL_Init(SDL_INIT_JOYSTICK);
                    sdlJoystick = SDL_JoystickOpen(ui->cboDevice->currentIndex());
                    sdlEventsPumpTimerId = startTimer(10, Qt::VeryCoarseTimer);
                } else {
                    focusedControlButton = nullptr;
                    if (sdlJoystick)
                        SDL_JoystickClose(sdlJoystick);
                    SDL_Quit();
                }
            });
        }


        /// Load from local config to UI
        unsavedChanges = false;
        fromAutoChange = false;
        currentController = 0;
        updateControllerConfig(currentController);
    }


    /// Init joystick (device) list
    SDL_Init(SDL_INIT_JOYSTICK);

    for(int i = 0; i < SDL_NumJoysticks(); i++)
        ui->cboDevice->addItem( SDL_JoystickNameForIndex(i) );


    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Close"));


    connect(ui->saveBtn, SIGNAL(clicked()), this, SLOT(saveInputSettings()));
    connect(ui->helpButton, SIGNAL(clicked()), this, SLOT(openHelp()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(confirmClose()));
}


InputEditorDialog::~InputEditorDialog()
{
    delete ui;
}


void InputEditorDialog::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    // Override showEvent so we can check for SDL inputs after the dialog has rendered
    // This is so users can at least see the dialog even without a controller connected
    QMetaObject::invokeMethod(this, "checkErrors", Qt::ConnectionType::QueuedConnection);
}


void InputEditorDialog::checkErrors()
{
    if (SDL_NumJoysticks() == 0) {
        setEnabled(false);
        QMessageBox::critical(this, tr("No SDL inputs found"), QString(tr("Could not find a connected controller. This editor only works with controller/gamepad input and not with keyboard input. ")
                                                                     + tr("Check to make sure your controller is connected to your PC.") + "<br /><br />"
                                                                     + tr("If you just connected your controller, try restarting <AppName>.").replace("<AppName>", AppName)));
        close();
    }

    if (missingControllers.size() != 0) {
        setEnabled(false);
        QMessageBox::critical(this, missingControllers.join(", ") + tr("Not Found"),
                              QString(tr("Input configuration sections for ") + missingControllers.join(", ")+ tr(" not found in your mupen64plus.cfg. Please fix this and try again.")));
        close();
    }
}

void InputEditorDialog::confirmClose()
{
    if(unsavedChanges) {
        int answer = QMessageBox::question(this, tr("Unsaved Changes"), tr("There are unsaved changes. Are you sure you want to leave?"),
                                           QMessageBox::Yes | QMessageBox::No);

        if (answer == QMessageBox::Yes)
            reject();
    } else
        reject();
}


void InputEditorDialog::inputEvent(const QString &eventType, const QString &eventData)
{
    if (!focusedControlButton)
        return;

    bool forAxis = focusedControlButton == ui->btnXAxis || focusedControlButton == ui->btnYAxis;

    if (forAxis && eventType == "mouse")
        return; // Axis don't support mouse events

    QString inputString;

    if (forAxis) {
        if(controlAxisFirstEvent.name == eventType) {
            inputString = QString("%1(%2,%3)").arg(eventType, controlAxisFirstEvent.param, eventData);
            controlAxisFirstEvent.name.clear();
        } else {
            controlAxisFirstEvent.name = eventType;
            controlAxisFirstEvent.param = eventData;
        }
    } else
        inputString = QString("%1(%2)").arg(eventType, eventData);

    if (!inputString.isNull() && !focusedControlButton->text().contains(inputString)) {
        if (focusedControlButton->text() == CONTROL_BUTTON_EMPTY_TEXT)
            focusedControlButton->setText(inputString);
        else
            focusedControlButton->setText(focusedControlButton->text() + " " + inputString);

        const QString controlKey = mapControlButtonToControlKey[focusedControlButton];

        focusedControlButton->setToolTip(controlKey + " = " + focusedControlButton->text());
        controlsConfig[ui->cboController->currentIndex()][controlKey] = focusedControlButton->text();
        unsavedChanges = true;

        focusedControlButton->click();
    }
}


void InputEditorDialog::mousePressEvent(QMouseEvent *event)
{
    if (focusedControlButton)
        inputEvent("mouse", QString::number(event->button() == Qt::MiddleButton ? 2 : event->button() == Qt::RightButton ? 3 : 1));
}


void InputEditorDialog::keyPressEvent(QKeyEvent *event)
{
    if (!focusedControlButton)
        return;

    if (event->key() == Qt::Key_Escape) // Abandon current selection
        focusedControlButton->click();
    else if (event->key() == Qt::Key_Backspace) { // Empty the current input selection
        focusedControlButton->setText(CONTROL_BUTTON_EMPTY_TEXT);
        focusedControlButton->setToolTip(mapControlButtonToControlKey[focusedControlButton]);
        focusedControlButton->click();
    }
}


void InputEditorDialog::openHelp()
{
    QMessageBox::information(this, tr("Input Editor Help"), QString(tr("To map your controller inputs, left click an input to start listening and then press ")
                                                                  + tr("the corresponding button or axis you want it mapped to on your controller. ")
                                                                  + tr("For the X and Y axis, press both directions while listening.") + "<br /><br />"
                                                                  + tr("You have the following shortcuts while listening:") + "<br /><b>Esc</b>: "
                                                                  + tr("Abandon this input") + "<br /><b>Backspace</b>: " + tr("Clear the current input selection") + "<br /><br /><br />"
                                                                  + tr("See <link>here<linkend> for a full description of all the input options.")
                                                                        .replace("<link>", "<a href=\"https://mupen64plus.org/wiki/index.php/Mupen64Plus_Plugin_Parameters#Input-SDL\">")
                                                                        .replace("<linkend>", "</a>")));
}


void InputEditorDialog::saveInputSettings()
{
    // Check if the file can be opened for reading
    if (!config.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(this, tr("mupen64plus.cfg was not found"), QString(tr("Make sure you've set the config path to a directory with mupen64plus.cfg.")));
        return;
    }

    bool controllerFound = false;
    bool missing = true;

    // Read all lines from the config file
    QStringList lines;
    QTextStream in(&config);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Check for the header sections and set controllerFound to true if we're in the right one
        if (line.startsWith("[")) {
            if (line.startsWith("[" + ui->cboController->currentText() + "]")) {
                controllerFound = true;
                missing = false;
            } else
                controllerFound = false;
        } else if (controllerFound) {
            QStringList configValue = line.split("=");

            if (!configValue.isEmpty()) {
                if (configValue.first().trimmed() == "device")
                    line = configValue.first().trimmed() + " = " + QString::number(ui->cboDevice->currentIndex());
                if (configValue.first().trimmed() == "name")
                    line = configValue.first().trimmed() + " = " + ui->cboDevice->currentText().toLatin1();
                if (configValue.first().trimmed() == "plugged")
                    line = configValue.first().trimmed() + " = " + (ui->chkPlugged->isChecked() ? "True" : "False");
                if (configValue.first().trimmed() == "plugin")
                    line = configValue.first().trimmed() + " = " + QString::number(pluginOptions.key(ui->cboPlugin->currentText()));
                if (configValue.first().trimmed() == "mouse")
                    line = configValue.first().trimmed() + " = " + (ui->chkMouse->isChecked() ? "True" : "False");
                if (configValue.first().trimmed() == "MouseSensitivity")
                    line = configValue.first().trimmed() + " = " + ui->nbMouseSensitivityX->text().toLatin1() + "," + ui->nbMouseSensitivityY->text().toLatin1();
                if (configValue.first().trimmed() == "AnalogDeadzone")
                    line = configValue.first().trimmed() + " = " + ui->nbAnalogDeadzoneX->text().toLatin1() + "," + ui->nbAnalogDeadzoneY->text().toLatin1();
                if (configValue.first().trimmed() == "AnalogPeak")
                    line = configValue.first().trimmed() + " = " + ui->nbAnalogPeakX->text().toLatin1() + "," + ui->nbAnalogPeakY->text().toLatin1();

                for (auto it = mapControlButtonToControlKey.constBegin(); it != mapControlButtonToControlKey.constEnd(); ++it)
                    if (configValue.first().trimmed() == it.value())
                        line = configValue.first().trimmed() + " = " + it.key()->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
            }
        }


        lines << line;
    }
    config.close();


    if (!missing) {
        // Write the modified lines back to the config
        if (!config.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::information(this, tr("Could not open mupen64plus.cfg for writing"), QString(tr("Check the write permissions in your config directory.")));
            return;
        }

        QTextStream out(&config);
        for (const QString &line : std::as_const(lines))
            out << line << "\n";

        config.close();


        unsavedChanges = false;
        QMessageBox::information(this, tr("Save successful"), QString(tr("Input configuration for [") + ui->cboController->currentText() + tr("] successfully saved to mupen64plus.cfg.")));
    } else
        QMessageBox::critical(this, "[" + ui->cboController->currentText() + "] " + tr("Not Found"),
                              QString(tr("Input configuration section for [") + ui->cboController->currentText() + tr("] not found in your mupen64plus.cfg. Please fix this and try again.")));
}


void InputEditorDialog::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != sdlEventsPumpTimerId || focusedControlButton == nullptr)
        return;

    SDL_Event event;

    while (SDL_PollEvent(&event) == 1)
    {
        QString eventType, eventData;

        if (event.type == SDL_JOYAXISMOTION && abs(event.jaxis.value) >= 32767) {
            eventType = "axis";
            eventData = QString::number(event.jaxis.axis) + (event.jaxis.value > 0 ? "+" : "-");
        } else if (event.type == SDL_JOYHATMOTION) {
            eventType = "hat";
            eventData = QString::number(event.jhat.hat) + QChar(event.jhat.value) == QChar(SDL_HAT_UP) ? " Up" :
                            event.jhat.value == SDL_HAT_DOWN ? " Down" :
                            event.jhat.value == SDL_HAT_LEFT ? " Left" : " Right";
        } else if (event.type == SDL_JOYBUTTONDOWN) {
            eventType = "button";
            eventData = QString::number(event.jbutton.button);
        }

        if (!eventType.isNull())
            inputEvent(eventType, eventData);
    }

    SDL_Delay(20);
}


void InputEditorDialog::updateControllerConfig(int controller)
{
    const QMap<QString, QVariant>& cfg = controlsConfig[controller];

    ui->chkMouse->setChecked(cfg.value("mouse", false).toBool());
    ui->chkPlugged->setChecked(cfg.value("plugged", false).toBool());

    ui->cboPlugin->setCurrentText(pluginOptions.value(cfg.value("plugin", 1).toInt()));
    ui->cboDevice->setCurrentText(cfg.value("name", QString()).toString());
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

    unsavedChanges = false;
    currentController = controller;
}
