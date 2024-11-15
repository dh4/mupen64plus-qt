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


    /// 1. Init joystick (device) list
    SDL_Init(SDL_INIT_JOYSTICK);

    for(int i = 0; i < SDL_NumJoysticks(); i++)
        ui->cboDevice->addItem( SDL_JoystickNameForIndex(i) );


    /// 2. Populate controlsConfig with empty values
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


    /// 3. Grab current configuration from mupen64plus.cfg
    if (config.open(QFile::ReadOnly)) {
        QTextStream stream(&config);

        QString line;
        int controlId = -1;
        int nbLine = 0;
        bool controllerSection = false;

        while (stream.readLineInto(&line))
        {
            line = line.simplified();

            // [Input-SDL-Control1], [Input-SDL-Control2]...
            // Grab controller ID and set controllerSection to true
            if (line.trimmed().size() == 20 && line.startsWith("[Input-SDL-Control") && line.endsWith(']')) {
                controllerSection = true;
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

            nbLine++;
        }
        config.close();
    }


    /// 4. Init Maps keys
    mapControlButtonToControlKey = {
        {ui->btnDPadU, "DPad U"}, {ui->btnDPadD, "DPad D"}, {ui->btnDPadL, "DPad L"}, {ui->btnDPadR, "DPad R"},
        {ui->btnStart, "Start"}, {ui->btnLTrig, "L Trig"}, {ui->btnRTrig, "R Trig"}, {ui->btnZTrig, "Z Trig"},
        {ui->btnCBtnU, "C Button U"}, {ui->btnCBtnD, "C Button D"}, {ui->btnCBtnL, "C Button L"}, {ui->btnCBtnR, "C Button R"},
        {ui->btnABtn, "A Button"}, {ui->btnBBtn, "B Button"},
        {ui->btnMempak, "Mempak switch"}, {ui->btnRumblepak, "Rumblepak switch"},
        {ui->btnXAxis, "X Axis"}, {ui->btnYAxis, "Y Axis"},
    };

    /// 5. Load local config to UI upon selection of a Control config
    connect(ui->cboController, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int nbController)
            {
                const QMap<QString, QVariant>& cfg = controlsConfig[nbController];

                ui->chkMouse->setChecked(cfg.value("mouse", false).toBool());
                ui->chkPlugged->setChecked(cfg.value("plugged", false).toBool());

                ui->cboPlugin->setCurrentIndex(cfg.value("plugin", 1).toInt());
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
            });


    /// 6. Update local config upon UI change
    connect(ui->chkMouse, &QCheckBox::toggled, this, [this] (bool mouseEnabled) { controlsConfig[ui->cboController->currentIndex()]["mouse"] = mouseEnabled; });
    connect(ui->chkPlugged, &QCheckBox::toggled, this, [this] (bool plugged) { controlsConfig[ui->cboController->currentIndex()]["plugged"] = plugged; });

    connect(ui->cboPlugin, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] (int plugin) { controlsConfig[ui->cboController->currentIndex()]["plugin"] = plugin; });
    connect(ui->cboDevice, &QComboBox::currentTextChanged, this, [this] (const QString& device)
            {
                if (sdlJoystick)
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
                    if (checked) {
                        if (focusedControlButton)
                            focusedControlButton->setChecked(false);
                        focusedControlButton = btn;
                    } else
                        focusedControlButton = nullptr;
                });
    }


    /// 7. Start joystick events pump
    sdlEventsPumpTimerId = startTimer(10, Qt::VeryCoarseTimer);


    /// 8. Load from local config to UI
    emit ui->cboController->currentIndexChanged(0);


    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Close"));


    connect(ui->saveBtn, SIGNAL(clicked()), this, SLOT(saveInputSettings()));
    connect(ui->helpButton, SIGNAL(clicked()), this, SLOT(openHelp()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


InputEditorDialog::~InputEditorDialog()
{
    delete ui;
}


void InputEditorDialog::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);

    // Override showEvent so we can check for SDL inputs after the dialog has rendered
    // This is so users can at least see the dialog even without a controller connected
    QMetaObject::invokeMethod(this, "checkSDLInputs", Qt::ConnectionType::QueuedConnection);
}


void InputEditorDialog::checkSDLInputs() {
    if (SDL_NumJoysticks() == 0) {
        setEnabled(false);
        QMessageBox::critical(this, tr("No SDL inputs found"), QString(tr("Could not find a connected controller. This editor only works with controller/gamepad input and not with keyboard input. ")
                                                                     + tr("Check to make sure your controller is connected to your PC.") + "<br /><br />"
                                                                     + tr("If you just connected your controller, try restarting <AppName>.").replace("<AppName>", AppName)));
        close();
    }
}


void InputEditorDialog::inputEvent(const QString &eventType, const QString &eventData)
{
    bool forAxis = focusedControlButton == ui->btnXAxis || focusedControlButton == ui->btnYAxis;

    if(forAxis && eventType == "mouse")
        return; // Axis don't support mouse events

    QString inputString;

    if(forAxis) {
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

    if (event->key() == Qt::Key_Escape) //  == SDLK_ESCAPE
        focusedControlButton->click();
    else if (event->key() == Qt::Key_Backspace) {
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


void InputEditorDialog::timerEvent(QTimerEvent *e)
{
    if(e->timerId() != sdlEventsPumpTimerId || focusedControlButton == nullptr)
        return;

    SDL_Event event;

    while(SDL_PollEvent(&event) == 1)
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

        if(! eventType.isNull())
            inputEvent(eventType, eventData);
    }

    SDL_Delay(20);
}


void InputEditorDialog::saveInputSettings()
{
    // Check if the file can be opened for reading
    if (!config.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(this, tr("mupen64plus.cfg was not found"), QString(tr("Make sure you've set the config path to a directory with mupen64plus.cfg.")));
        return;
    }

    bool controllerFound = false;

    // Read all lines from the config file
    QStringList lines;
    QTextStream in(&config);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Check for the header sections and set controllerFound to true if we're in the right one
        if (line.startsWith("[")) {
            if (line.startsWith("[" + ui->cboController->currentText() + "]"))
                controllerFound = true;
            else
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
                    line = configValue.first().trimmed() + " = " + QString::number(ui->cboPlugin->currentIndex());
                if (configValue.first().trimmed() == "mouse")
                    line = configValue.first().trimmed() + " = " + (ui->chkMouse->isChecked() ? "True" : "False");
                if (configValue.first().trimmed() == "MouseSensitivity")
                    line = configValue.first().trimmed() + " = " + ui->nbMouseSensitivityX->text().toLatin1() + "," + ui->nbMouseSensitivityY->text().toLatin1();
                if (configValue.first().trimmed() == "AnalogDeadzone")
                    line = configValue.first().trimmed() + " = " + ui->nbAnalogDeadzoneX->text().toLatin1() + "," + ui->nbAnalogDeadzoneY->text().toLatin1();
                if (configValue.first().trimmed() == "AnalogPeak")
                    line = configValue.first().trimmed() + " = " + ui->nbAnalogPeakX->text().toLatin1() + "," + ui->nbAnalogPeakY->text().toLatin1();
                if (configValue.first().trimmed() == "DPad R")
                    line = configValue.first().trimmed() + " = " + ui->btnDPadR->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "DPad L")
                    line = configValue.first().trimmed() + " = " + ui->btnDPadL->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "DPad D")
                    line = configValue.first().trimmed() + " = " + ui->btnDPadD->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "DPad U")
                    line = configValue.first().trimmed() + " = " + ui->btnDPadU->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "Start")
                    line = configValue.first().trimmed() + " = " + ui->btnStart->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "Z Trig")
                    line = configValue.first().trimmed() + " = " + ui->btnZTrig->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "B Button")
                    line = configValue.first().trimmed() + " = " + ui->btnBBtn->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "A Button")
                    line = configValue.first().trimmed() + " = " + ui->btnABtn->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "C Button R")
                    line = configValue.first().trimmed() + " = " + ui->btnCBtnR->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "C Button L")
                    line = configValue.first().trimmed() + " = " + ui->btnCBtnL->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "C Button D")
                    line = configValue.first().trimmed() + " = " + ui->btnCBtnD->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "C Button U")
                    line = configValue.first().trimmed() + " = " + ui->btnCBtnU->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "R Trig")
                    line = configValue.first().trimmed() + " = " + ui->btnRTrig->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "L Trig")
                    line = configValue.first().trimmed() + " = " + ui->btnLTrig->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "Mempak switch")
                    line = configValue.first().trimmed() + " = " + ui->btnMempak->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "Rumblepak switch")
                    line = configValue.first().trimmed() + " = " + ui->btnRumblepak->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "X Axis")
                    line = configValue.first().trimmed() + " = " + ui->btnXAxis->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
                if (configValue.first().trimmed() == "Y Axis")
                    line = configValue.first().trimmed() + " = " + ui->btnYAxis->text().replace(CONTROL_BUTTON_EMPTY_TEXT,"").toLatin1();
            }
        }


        lines << line;
    }
    config.close();


    // Write the modified lines back to the config
    if (!config.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::information(this, tr("Could not open mupen64plus.cfg for writing"), QString(tr("Check the write permissions in your config directory.")));
        return;
    }

    QTextStream out(&config);
    for (const QString &line : std::as_const(lines))
        out << line << "\n";

    config.close();

    QMessageBox::information(this, tr("Save successful"), QString(tr("Input configuration for ") + ui->cboController->currentText() + tr(" successfully saved to mupen64plus.cfg.")));
}
