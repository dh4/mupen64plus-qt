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

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDesktopWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>


class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0, int activeTab = 0);

private:
    QDir pluginsDir;
    QStringList audioPlugins;
    QStringList available;
    QStringList current;
    QStringList inputPlugins;
    QStringList modes;
    QStringList rspPlugins;
    QStringList useableModes;
    QStringList videoPlugins;

    QButtonGroup *emulationGroup;
    QCheckBox *fullscreenOption;
    QCheckBox *osdOption;
    QCheckBox *saveOption;
    QComboBox *audioBox;
    QComboBox *inputBox;
    QComboBox *resolutionBox;
    QComboBox *rspBox;
    QComboBox *videoBox;
    QDesktopWidget *desktop;
    QDialogButtonBox *buttonBox;
    QGridLayout *columnsLayout;
    QGridLayout *emulationLayout;
    QGridLayout *graphicsLayout;
    QGridLayout *layout;
    QGridLayout *otherLayout;
    QGridLayout *pathsLayout;
    QGridLayout *pluginsLayout;
    QLabel *audioLabel;
    QLabel *availableLabel;
    QLabel *configPathLabel;
    QLabel *currentLabel;
    QLabel *dataPathLabel;
    QLabel *fullscreenLabel;
    QLabel *inputLabel;
    QLabel *mupen64PathLabel;
    QLabel *osdLabel;
    QLabel *pluginPathLabel;
    QLabel *resolutionLabel;
    QLabel *romPathLabel;
    QLabel *rspLabel;
    QLabel *saveLabel;
    QLabel *videoLabel;
    QLineEdit *mupen64Path;
    QLineEdit *pluginPath;
    QLineEdit *dataPath;
    QLineEdit *configPath;
    QLineEdit *romPath;
    QListWidget *availableList;
    QListWidget *currentList;
    QPushButton *configButton;
    QPushButton *dataButton;
    QPushButton *mupen64Button;
    QPushButton *pluginButton;
    QPushButton *romButton;
    QRadioButton *cachedButton;
    QRadioButton *dynamicButton;
    QRadioButton *pureButton;
    QTabWidget *tabWidget;
    QToolButton *addButton;
    QToolButton *removeButton;
    QToolButton *sortDownButton;
    QToolButton *sortUpButton;
    QVBoxLayout *sortLayout;
    QVBoxLayout *toggleLayout;
    QWidget *columnsWidget;
    QWidget *emulationWidget;
    QWidget *graphicsWidget;
    QWidget *otherWidget;
    QWidget *pathsWidget;
    QWidget *pluginsWidget;
    QWidget *sortWidget;
    QWidget *toggleWidget;

private slots:
    void addColumn();
    void browseMupen64();
    void browsePlugin();
    void browseData();
    void browseConfig();
    void browseROM();
    void editSettings();
    void removeColumn();
    void sortDown();
    void sortUp();
};

#endif // SETTINGSDIALOG_H
