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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDesktopWidget>
#include <QFileDialog>
#include <QListWidget>

#include "global.h"


namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0, int activeTab = 0);
    ~SettingsDialog();

private:
    void populateAvailable(bool downloadItems);

    Ui::SettingsDialog *ui;

    QDesktopWidget *desktop;
    QDir pluginsDir;
    QList<QWidget*> downloadEnable;
    QList<QWidget*> labelEnable;
    QList<QWidget*> listCoverEnable;
    QStringList available;
    QStringList labelOptions;
    QStringList sortOptions;

private slots:
    void addColumn(QListWidget *currentList, QListWidget *availableList);
    void browseBackground();
    void browseMupen64();
    void browsePlugin();
    void browseData();
    void browseConfig();
    void browseROM();
    void editSettings();
    void listAddColumn();
    void listRemoveColumn();
    void listSortDown();
    void listSortUp();
    void removeColumn(QListWidget *currentList, QListWidget *availableList);
    void populateTableAndListTab(bool downloadItems);
    void sortDown(QListWidget *currentList);
    void sortUp(QListWidget *currentList);
    void tableAddColumn();
    void tableRemoveColumn();
    void tableSortDown();
    void tableSortUp();
    void toggleDownload(bool active);
    void toggleLabel(bool active);
    void toggleListCover(bool active);
};

#endif // SETTINGSDIALOG_H
