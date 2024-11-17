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

#include "keycodesdialog.h"

#include "../global.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>


KeyCodes::KeyCodes(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("SDL Key Codes"));
    setMinimumSize(300, 400);

    keyCodesLayout = new QGridLayout(this);
    keyCodesLayout->setContentsMargins(5, 10, 5, 10);

    QList<QStringList> codes;
    codes << (QStringList() << "Backspace" << "8")
          << (QStringList() << "Tab" << "9")
          << (QStringList() << "Clear" << "12")
          << (QStringList() << "Enter" << "13")
          << (QStringList() << "Pause" << "19")
          << (QStringList() << "Escape" << "27")
          << (QStringList() << "Space" << "32")
          << (QStringList() << "!" << "33")
          << (QStringList() << "\"" << "34")
          << (QStringList() << "#" << "35")
          << (QStringList() << "$" << "36")
          << (QStringList() << "&" << "38")
          << (QStringList() << "'" << "39")
          << (QStringList() << "(" << "40")
          << (QStringList() << ")" << "41")
          << (QStringList() << "*" << "42")
          << (QStringList() << "+" << "43")
          << (QStringList() << "," << "44")
          << (QStringList() << "-" << "45")
          << (QStringList() << "." << "46")
          << (QStringList() << "/" << "47")
          << (QStringList() << "0" << "48")
          << (QStringList() << "1" << "49")
          << (QStringList() << "2" << "50")
          << (QStringList() << "3" << "51")
          << (QStringList() << "4" << "52")
          << (QStringList() << "5" << "53")
          << (QStringList() << "6" << "54")
          << (QStringList() << "7" << "55")
          << (QStringList() << "8" << "56")
          << (QStringList() << "9" << "57")
          << (QStringList() << ":" << "58")
          << (QStringList() << ";" << "59")
          << (QStringList() << "<" << "60")
          << (QStringList() << "=" << "61")
          << (QStringList() << ">" << "62")
          << (QStringList() << "?" << "63")
          << (QStringList() << "@" << "64")
          << (QStringList() << "[" << "91")
          << (QStringList() << "\\" << "92")
          << (QStringList() << "]" << "93")
          << (QStringList() << "^" << "94")
          << (QStringList() << "_" << "95")
          << (QStringList() << "`" << "96")
          << (QStringList() << "a" << "97")
          << (QStringList() << "b" << "98")
          << (QStringList() << "c" << "99")
          << (QStringList() << "d" << "100")
          << (QStringList() << "e" << "101")
          << (QStringList() << "f" << "102")
          << (QStringList() << "g" << "103")
          << (QStringList() << "h" << "104")
          << (QStringList() << "i" << "105")
          << (QStringList() << "j" << "106")
          << (QStringList() << "k" << "107")
          << (QStringList() << "l" << "108")
          << (QStringList() << "m" << "109")
          << (QStringList() << "n" << "110")
          << (QStringList() << "o" << "111")
          << (QStringList() << "p" << "112")
          << (QStringList() << "q" << "113")
          << (QStringList() << "r" << "114")
          << (QStringList() << "s" << "115")
          << (QStringList() << "t" << "116")
          << (QStringList() << "u" << "117")
          << (QStringList() << "v" << "118")
          << (QStringList() << "w" << "119")
          << (QStringList() << "x" << "120")
          << (QStringList() << "y" << "121")
          << (QStringList() << "z" << "122")
          << (QStringList() << "Delete" << "127")
          << (QStringList() << "Keypad 0" << "256")
          << (QStringList() << "Keypad 1" << "257")
          << (QStringList() << "Keypad 2" << "258")
          << (QStringList() << "Keypad 3" << "259")
          << (QStringList() << "Keypad 4" << "260")
          << (QStringList() << "Keypad 5" << "261")
          << (QStringList() << "Keypad 6" << "262")
          << (QStringList() << "Keypad 7" << "263")
          << (QStringList() << "Keypad 8" << "264")
          << (QStringList() << "Keypad 9" << "265")
          << (QStringList() << "Keypad ." << "266")
          << (QStringList() << "Keypad /" << "267")
          << (QStringList() << "Keypad *" << "268")
          << (QStringList() << "Keypad -" << "269")
          << (QStringList() << "Keypad +" << "270")
          << (QStringList() << "Keypad Enter" << "271")
          << (QStringList() << "Keypad =" << "272")
          << (QStringList() << "Up" << "273")
          << (QStringList() << "Down" << "274")
          << (QStringList() << "Right" << "275")
          << (QStringList() << "Left" << "276")
          << (QStringList() << "Insert" << "277")
          << (QStringList() << "Home" << "278")
          << (QStringList() << "End" << "279")
          << (QStringList() << "Page Up" << "280")
          << (QStringList() << "Page Down" << "281")
          << (QStringList() << "F1" << "282")
          << (QStringList() << "F2" << "283")
          << (QStringList() << "F3" << "284")
          << (QStringList() << "F4" << "285")
          << (QStringList() << "F5" << "286")
          << (QStringList() << "F6" << "287")
          << (QStringList() << "F7" << "288")
          << (QStringList() << "F8" << "289")
          << (QStringList() << "F9" << "290")
          << (QStringList() << "F10" << "291")
          << (QStringList() << "F11" << "292")
          << (QStringList() << "F12" << "293")
          << (QStringList() << "F13" << "294")
          << (QStringList() << "F14" << "295")
          << (QStringList() << "F15" << "296")
          << (QStringList() << "Num Lock" << "300")
          << (QStringList() << "Caps Lock" << "301")
          << (QStringList() << "Scroll Lock" << "302")
          << (QStringList() << "Right Shift" << "303")
          << (QStringList() << "Left Shift" << "304")
          << (QStringList() << "Right Ctrl" << "305")
          << (QStringList() << "Left Ctrl" << "306")
          << (QStringList() << "Right Alt" << "307")
          << (QStringList() << "Left Alt" << "308")
          << (QStringList() << "Right Meta" << "309")
          << (QStringList() << "Left Meta" << "310")
          << (QStringList() << "Left Super" << "311")
          << (QStringList() << "Right Super" << "312")
          << (QStringList() << "Mode" << "313")
          << (QStringList() << "Compose" << "314")
          << (QStringList() << "Help" << "315")
          << (QStringList() << "Print" << "316")
          << (QStringList() << "SysRq" << "317")
          << (QStringList() << "Break" << "318")
          << (QStringList() << "Menu" << "319")
          << (QStringList() << "Power" << "320")
          << (QStringList() << "Euro" << "321")
          << (QStringList() << "Undo" << "322");

    keyCodesTable = new QTableWidget(this);
    keyCodesTable->setColumnCount(2);
    keyCodesTable->setRowCount(codes.length());
    keyCodesTable->setHorizontalHeaderLabels(QStringList() << tr("Key") << tr("SDL Code"));
    keyCodesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    keyCodesTable->verticalHeader()->setVisible(false);
    keyCodesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    keyCodesTable->setSelectionMode(QAbstractItemView::SingleSelection);

    for (int i = 0; i < codes.length(); i++)
    {
        keyCodesTable->setItem(i, 0, new QTableWidgetItem(codes.at(i).at(0)));
        keyCodesTable->setItem(i, 1, new QTableWidgetItem(codes.at(i).at(1)));
    }

    controls = new QWidget(this);
    controlsLayout = new QGridLayout(controls);
    controlsLayout->setContentsMargins(0, 0, 0, 0);

    keyCodesButtonBox = new QDialogButtonBox(Qt::Horizontal, this);
    keyCodesButtonBox->addButton(tr("Close"), QDialogButtonBox::RejectRole);

    helpButtonBox = new QDialogButtonBox(Qt::Horizontal, this);
    QPushButton *helpButton = new QPushButton();
    helpButton->setIcon(QIcon::fromTheme("help-about"));
    helpButtonBox->addButton(helpButton, QDialogButtonBox::HelpRole);

    QString docsLinkString = "<a href=\"https://www.libsdl.org/release/SDL-1.2.15/include/SDL_keysym.h\">"
                             + tr("Full Reference") + "</a>";

    docsLink = new QLabel(docsLinkString, controls);
    docsLink->setOpenExternalLinks(true);

    QSpacerItem *spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);

    controlsLayout->addWidget(helpButtonBox, 0, 0, Qt::AlignLeft);
    controlsLayout->addWidget(docsLink, 0, 1, Qt::AlignLeft);
    controlsLayout->addItem(spacer, 0, 2);
    controlsLayout->addWidget(keyCodesButtonBox, 0, 3, Qt::AlignRight);
    controls->setLayout(controlsLayout);

    keyCodesLayout->addWidget(keyCodesTable, 0, 0);
    keyCodesLayout->addWidget(controls, 1, 0);

    connect(keyCodesButtonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect(helpButton, SIGNAL(clicked()), this, SLOT(openHelp()));

    setLayout(keyCodesLayout);
}


void KeyCodes::openHelp()
{
    QMessageBox::information(this, tr("Key Codes Help"), QString(tr("These codes can be used to map keyboard keys to N64 inputs under the [Input-SDL-Control] sections. ")
                                                                    + tr("To use a code put it inside of key(). For example:") + "<br />"
                                                                    + "A Button = key(97)" + "<br /><br />"
                                                                    + tr("This would set the A button to the a key on your keyboard.") + "<br /><br />"
                                                                    + tr("When modifying keys, set the controller mode to 0 (Fully Manual). Otherwise <ParentName> will overwrite your settings.")
                                                                          .replace("<ParentName>", ParentName)));
}
