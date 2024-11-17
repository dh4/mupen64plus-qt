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

#ifndef INPUTEDITORDIALOG_H
#define INPUTEDITORDIALOG_H

#include <QDialog>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QMessageBox>
#include <QMouseEvent>

#include <SDL2/SDL.h>

class QDesktopWidget;


namespace Ui {
    class InputEditorDialog;
}

class InputEditorDialog : public QDialog
{
    Q_OBJECT

    
public:
    explicit InputEditorDialog(QString configFile, QWidget *parent = 0);
    ~InputEditorDialog();

    
private:
    Ui::InputEditorDialog *ui;

    QDesktopWidget *desktop;
    QFile config;
    QStringList missingControllers;

    bool fromAutoChange;
    bool unsavedChanges;
    int currentController;
    int initialMode;
    bool initialPlugged;

    const QString CONTROL_BUTTON_EMPTY_TEXT = "Select...";

    int sdlEventsPumpTimerId{0};
    QMap<QString, QVariant> controlsConfig[4];
    QMap<int, QString> pluginOptions;
    QMap<QPushButton*, QString> mapControlButtonToControlKey;
    QPushButton* focusedControlButton{nullptr};
    SDL_Joystick* sdlJoystick{NULL};
    struct { QString name; QString param; } controlAxisFirstEvent;
    

private slots:
    void checkErrors();
    void confirmClose();
    void openHelp();
    void resetInputSettings();
    void saveInputSettings();
    void setUnsavedChanges(bool changes, bool modeChange = false);
    void updateControllerConfig(int controller);


protected:
    void inputEvent(const QString& eventType, const QString& eventData);
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void timerEvent(QTimerEvent *e) override;
};

#endif // INPUTEDITORDIALOG_H
