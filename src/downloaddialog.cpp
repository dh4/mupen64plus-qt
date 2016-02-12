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

#include "downloaddialog.h"
#include "common.h"
#include "thegamesdbscrapper.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>


DownloadDialog::DownloadDialog(QString fileText, QString defaultText, QString romMD5, QWidget *parent)
    : QDialog(parent)
{
    this->romMD5 = romMD5;
    this->parent = parent;

    setWindowTitle(tr("Search Game Information"));

    downloadLayout = new QGridLayout(this);

    fileLabel = new QLabel("<b>" + tr("File") + ":</b> " + fileText, this);

    gameNameLabel = new QLabel(tr("Name of Game:"), this);
    gameIDLabel = new QLabel(tr("or Game ID:"), this);

    defaultText.remove(QRegExp("\\W*(\\(|\\[).+(\\)|\\])\\W*"));
    gameNameField = new QLineEdit(defaultText, this);
    gameIDField = new QLineEdit(this);

    gameIDField->setToolTip(tr("From thegamesdb.net URL of game"));

    downloadButtonBox = new QDialogButtonBox(Qt::Horizontal, this);
    downloadButtonBox->addButton(tr("Search"), QDialogButtonBox::AcceptRole);
    downloadButtonBox->addButton(QDialogButtonBox::Cancel);

    downloadLayout->addWidget(fileLabel, 0, 0, 1, 2);
    downloadLayout->addWidget(gameNameLabel, 1, 0);
    downloadLayout->addWidget(gameIDLabel, 2, 0);
    downloadLayout->addWidget(gameNameField, 1, 1);
    downloadLayout->addWidget(gameIDField, 2, 1);
    downloadLayout->addWidget(downloadButtonBox, 4, 0, 1, 3);
    downloadLayout->setRowStretch(3,1);
    downloadLayout->setColumnStretch(1,1);

    downloadLayout->setColumnMinimumWidth(1, 300);
    downloadLayout->setRowMinimumHeight(0, 20);
    downloadLayout->setRowMinimumHeight(3, 20);

    connect(downloadButtonBox, SIGNAL(accepted()), this, SLOT(runDownloader()));
    connect(downloadButtonBox, SIGNAL(rejected()), this, SLOT(close()));

    setLayout(downloadLayout);
}


void DownloadDialog::runDownloader()
{
    close();

    scrapper = new TheGamesDBScrapper(parent, true);
    scrapper->downloadGameInfo(romMD5, gameNameField->text(), gameIDField->text());
}
