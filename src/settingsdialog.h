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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDesktopWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QTabWidget>

#include "global.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0, int activeTab = 0);

private:
    QDir pluginsDir;
    QStringList audioPlugins;
    QStringList inputPlugins;
    QStringList modes;
    QStringList rspPlugins;
    QStringList useableModes;
    QStringList videoPlugins;

    QCheckBox *fullscreenOption;
    QCheckBox *osdOption;
    QComboBox *audioBox;
    QComboBox *inputBox;
    QComboBox *resolutionBox;
    QComboBox *rspBox;
    QComboBox *videoBox;
    QDesktopWidget *desktop;
    QDialogButtonBox *buttonBox;
    QGridLayout *emulatorLayout;
    QGridLayout *filesLayout;
    QGridLayout *graphicsLayout;
    QGridLayout *layout;
    QGridLayout *pathsLayout;
    QGridLayout *pluginsLayout;
    QGroupBox *graphics;
    QGroupBox *paths;
    QGroupBox *plugins;
    QLabel *audioLabel;
    QLabel *configPathLabel;
    QLabel *dataPathLabel;
    QLabel *fullscreenLabel;
    QLabel *inputLabel;
    QLabel *mupen64PathLabel;
    QLabel *osdLabel;
    QLabel *pluginPathLabel;
    QLabel *resolutionLabel;
    QLabel *romPathLabel;
    QLabel *rspLabel;
    QLabel *videoLabel;
    QLineEdit *mupen64Path;
    QLineEdit *pluginPath;
    QLineEdit *dataPath;
    QLineEdit *configPath;
    QLineEdit *romPath;
    QPushButton *mupen64Button;
    QPushButton *pluginButton;
    QPushButton *dataButton;
    QPushButton *configButton;
    QPushButton *romButton;
    QTabWidget *tabWidget;
    QWidget *graphicsWidget;
    QWidget *pathsWidget;
    QWidget *pluginsWidget;

private slots:
    void browseMupen64();
    void browsePlugin();
    void browseData();
    void browseConfig();
    void browseROM();
    void editSettings();
};

#endif // SETTINGSDIALOG_H
