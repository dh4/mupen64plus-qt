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

#include "controlinfodialog.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>


ControlInfo::ControlInfo(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Mupen64Plus Default Controls"));
    setMinimumSize(600, 600);

    controlInfoLayout = new QGridLayout(this);
    controlInfoLayout->setContentsMargins(5, 10, 5, 10);

    QString description = tr("These are the default controls for Mupen64Plus. ")
                        + tr("They can be modified using Settings->Edit mupen64plus.cfg...")
                        + "<br /><br />"
                        + tr("For more information see the <link>Mupen64Plus Documentation<linkend>. ")
                          .replace("<link>", "<a href=\"https://mupen64plus.org/docs/\">")
                          .replace("<linkend>", "</a>")
                        + tr("For setting up a gamepad controller see <link>this page<linkend>. ")
                          .replace("<link>", "<a href=\"https://mupen64plus.org/wiki/index.php?title=ControllerSetup\">")
                          .replace("<linkend>", "</a>")
                        + tr("Note that Mupen64Plus output can be viewed with Emulation->View Log.")
                        + "<br />";

    descriptionLabel = new QLabel(description, this);
    descriptionLabel->setOpenExternalLinks(true);
    descriptionLabel->setWordWrap(true);

    controlInfoTabs = new QTabWidget(this);


    //Controller Input Tab
    inputTab = new QWidget(controlInfoTabs);

    inputLayout = new QGridLayout(inputTab);
    inputLayout->setContentsMargins(5, 10, 5, 10);

    QList<QStringList> inputs;
    inputs << (QStringList() << "Control Stick" << "Arrow Keys")
           << (QStringList() << "C Up/Left/Down/Right " << "I, J, K, L")
           << (QStringList() << "DPad Up/Left/Down/Right " << "W, A, S, D")
           << (QStringList() << "Z Trigger" << "Z")
           << (QStringList() << "L Trigger" << "X")
           << (QStringList() << "R Trigger" << "C")
           << (QStringList() << "Start" << "Enter")
           << (QStringList() << "A Button" << "Left Shift")
           << (QStringList() << "B Button" << "Left Ctrl")
           << (QStringList() << "Select Mem Pak" << ",")
           << (QStringList() << "Select Rumblep Pak" << ".");

    inputTable = new QTableWidget(this);
    inputTable->setColumnCount(2);
    inputTable->setRowCount(inputs.length());
    inputTable->setHorizontalHeaderLabels(QStringList() << tr("N64 Input") << tr("Key"));
    inputTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    inputTable->verticalHeader()->setVisible(false);
    inputTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    inputTable->setSelectionMode(QAbstractItemView::NoSelection);
    inputTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    for (int i = 0; i < inputs.length(); i++)
    {
        inputTable->setItem(i, 0, new QTableWidgetItem(inputs.at(i).at(0)));
        inputTable->setItem(i, 1, new QTableWidgetItem(inputs.at(i).at(1)));
    }

    keyboard = new QPixmap(":/images/keyboard.png");
    keyboardImage = new QLabel(this);
    keyboardImage->setAlignment(Qt::AlignCenter);
    keyboardImage->setPixmap(*keyboard);

    inputLayout->addWidget(inputTable, 0, 0);
    inputLayout->addWidget(keyboardImage, 1, 0);
    inputTab->setLayout(inputLayout);


    //Hotkeys Tab
    hotkeysTab = new QWidget(controlInfoTabs);

    hotkeysLayout = new QGridLayout(hotkeysTab);
    hotkeysLayout->setContentsMargins(5, 10, 5, 10);

    QList<QStringList> hotkeys;
    hotkeys << (QStringList() << "Esc" << "Quit the emulator")
            << (QStringList() << "0 through 9" << "Select virtual 'slot' for save/load state commands")
            << (QStringList() << "F5" << "Save emulator state")
            << (QStringList() << "F7" << "Load emulator state")
            << (QStringList() << "F9" << "Reset emulator")
            << (QStringList() << "F10" << "Slow down emulator by 5%")
            << (QStringList() << "F11" << "Speed up emulator by 5%")
            << (QStringList() << "F12" << "Take screenshot")
            << (QStringList() << "Alt+Enter" << "Toggle between windowed and fullscreen")
            << (QStringList() << "P" << "Pause on/off")
            << (QStringList() << "M" << "Mute/unmute sound")
            << (QStringList() << "G" << "Press \"Game Shark\" button (only if cheats are enabled)")
            << (QStringList() << "/" << "Single frame advance while paused")
            << (QStringList() << "F" << "Fast forward (playback at 250% normal speed while pressed)")
            << (QStringList() << "[" << "Decrease volume")
            << (QStringList() << "]" << "Increase volume");

    hotkeysTable = new QTableWidget(this);
    hotkeysTable->setColumnCount(2);
    hotkeysTable->setRowCount(hotkeys.length());
    hotkeysTable->setHorizontalHeaderLabels(QStringList() << tr("Key") << tr("Action"));
    hotkeysTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    hotkeysTable->horizontalHeader()->setStretchLastSection(true);
    hotkeysTable->verticalHeader()->setVisible(false);
    hotkeysTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    hotkeysTable->setSelectionMode(QAbstractItemView::NoSelection);

    for (int i = 0; i < hotkeys.length(); i++)
    {
        hotkeysTable->setItem(i, 0, new QTableWidgetItem(hotkeys.at(i).at(0)));
        hotkeysTable->setItem(i, 1, new QTableWidgetItem(hotkeys.at(i).at(1)));
    }

    hotkeysLayout->addWidget(hotkeysTable, 0, 0);
    hotkeysTab->setLayout(hotkeysLayout);


    controlInfoButtonBox = new QDialogButtonBox(Qt::Horizontal, this);
    controlInfoButtonBox->addButton(tr("Close"), QDialogButtonBox::RejectRole);

    connect(controlInfoButtonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect(controlInfoTabs, SIGNAL(currentChanged(int)), this, SLOT(resizeImageSlot(int)));

    controlInfoTabs->addTab(inputTab, tr("Controller Input"));
    controlInfoTabs->addTab(hotkeysTab, tr("Hotkeys"));

    controlInfoLayout->addWidget(descriptionLabel, 0, 0);
    controlInfoLayout->addWidget(controlInfoTabs, 1, 0);
    controlInfoLayout->addWidget(controlInfoButtonBox, 2, 0);

    setLayout(controlInfoLayout);
}


void ControlInfo::resizeEvent(QResizeEvent*)
{
    resizeImage();
}

void ControlInfo::resizeImage()
{
    int width = keyboardImage->width();
    if (width > 300 && width < 910)
        keyboardImage->setPixmap(keyboard->scaledToWidth(width,Qt::SmoothTransformation));
    else if (width >= 910)
        keyboardImage->setPixmap(keyboard->scaledToWidth(910,Qt::SmoothTransformation));
}

void ControlInfo::resizeImageSlot(int)
{
    resizeImage();
}
