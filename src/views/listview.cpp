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

#include "listview.h"

#include "../global.h"
#include "../common.h"

#include "widgets/clickablewidget.h"

#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>


ListView::ListView(QWidget *parent) : QScrollArea(parent)
{
    this->parent = parent;

    setObjectName("listView");
    setWidgetResizable(true);
    setHidden(true);

    setListBackground();


    listWidget = new QWidget(this);
    listWidget->setObjectName("listWidget");
    setWidget(listWidget);

    listLayout = new QVBoxLayout(listWidget);
    listLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    listWidget->setLayout(listLayout);

    listCurrent = false;
    currentListRom = 0;
}


void ListView::addToListView(Rom *currentRom, int count, bool ddEnabled)
{
    if (ddEnabled) // Add place for "No Cart" entry
        count++;

    QStringList visible = SETTINGS.value("List/columns", "Filename|Internal Name|Size").toString().split("|");

    if (visible.join("") == "" && SETTINGS.value("List/displaycover", "") != "true")
        //Otherwise no columns, so don't bother populating
        return;

    ClickableWidget *gameListItem = new ClickableWidget(listWidget);
    gameListItem->setContentsMargins(0, 0, 20, 0);
    gameListItem->setContextMenuPolicy(Qt::CustomContextMenu);
    if (SETTINGS.value("List/theme","Light").toString() == "Dark")
        gameListItem->setStyleSheet("color:#EEE;");

    //Assign ROM data to widget for use in click events
    gameListItem->setProperty("fileName", currentRom->fileName);
    gameListItem->setProperty("directory", currentRom->directory);
    if (currentRom->goodName == getTranslation("Unknown ROM") ||
        currentRom->goodName == getTranslation("Requires catalog file"))
        gameListItem->setProperty("search", currentRom->internalName);
    else
        gameListItem->setProperty("search", currentRom->goodName);
    gameListItem->setProperty("romMD5", currentRom->romMD5);
    gameListItem->setProperty("zipFile", currentRom->zipFile);

    QGridLayout *gameListLayout = new QGridLayout(gameListItem);
    gameListLayout->setColumnStretch(3, 1);

    //Add image
    if (SETTINGS.value("List/displaycover", "") == "true") {
        QLabel *listImageLabel = new QLabel(gameListItem);
        listImageLabel->setMinimumHeight(getImageSize("List").height());
        listImageLabel->setMinimumWidth(getImageSize("List").width());

        QPixmap image;

        if (currentRom->imageExists)
            image = currentRom->image.scaled(getImageSize("List"), Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation);
        else {
            if (ddEnabled && count == 0)
                image = QPixmap(":/images/no-cart.png").scaled(getImageSize("List"), Qt::KeepAspectRatio,
                                                                 Qt::SmoothTransformation);
            else
                image = QPixmap(":/images/not-found.png").scaled(getImageSize("List"), Qt::KeepAspectRatio,
                                                                 Qt::SmoothTransformation);
        }

        listImageLabel->setPixmap(image);
        listImageLabel->setAlignment(Qt::AlignCenter);
        gameListLayout->addWidget(listImageLabel, 0, 1);
    }

    //Create text label
    QLabel *listTextLabel = new QLabel("", gameListItem);
    QString listText = "";

    int i = 0;

    foreach (QString current, visible)
    {
        QString addition = "";

        if (i == 0 && SETTINGS.value("List/firstitemheader","true") == "true")
            addition += "<h2 style='line-height:120%;margin:0;padding:0;'>";
        else
            addition += "<div style='line-height:120%;margin:0;padding:0;'><b>"
                     + getTranslation(current) + ":</b> ";

        addition += getRomInfo(current, currentRom, true);

        if (i == 0 && SETTINGS.value("List/firstitemheader","true") == "true")
            addition += "</h2>";
        else
            addition += "</div>";

        if (addition.right(12) != ":</b> </div>")
            listText += addition;

        i++;
    }

    if (ddEnabled && count == 0)
        listText = "<h2>" + tr("No Cart") + "</h2>";

    listTextLabel->setText(listText);
    listTextLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    listTextLabel->setWordWrap(true);
    QFont font = listTextLabel->font();
    font.setPointSize(getTextSize());
    listTextLabel->setFont(font);
    gameListLayout->addWidget(listTextLabel, 0, 3);

    gameListLayout->setColumnMinimumWidth(0, 20);
    gameListLayout->setColumnMinimumWidth(2, 10);
    gameListItem->setLayout(gameListLayout);

    if (count != 0) {
        QFrame *separator = new QFrame();
        separator->setFrameShape(QFrame::HLine);
        separator->setStyleSheet("margin:0;padding:0;");
        QPalette palette = separator->palette();
        if (SETTINGS.value("List/theme","Light").toString() == "Dark")
            palette.setColor(QPalette::Window, Qt::black);
        else
            palette.setColor(QPalette::Window, Qt::gray);
        separator->setPalette(palette);
        listLayout->addWidget(separator);
    }

    listLayout->addWidget(gameListItem);

    connect(gameListItem, SIGNAL(singleClicked(QWidget*)), this, SLOT(highlightListWidget(QWidget*)));
    connect(gameListItem, SIGNAL(doubleClicked(QWidget*)), parent, SLOT(launchRomFromWidget(QWidget*)));
    connect(gameListItem, SIGNAL(arrowPressed(QWidget*, QString)), this, SLOT(selectNextRom(QWidget*, QString)));
    connect(gameListItem, SIGNAL(enterPressed(QWidget*)), parent, SLOT(launchRomFromWidget(QWidget*)));
    connect(gameListItem, SIGNAL(customContextMenuRequested(const QPoint &)), parent, SLOT(showRomMenu(const QPoint &)));
}


int ListView::getCurrentRom()
{
    return currentListRom;
}


QString ListView::getCurrentRomInfo(QString infoName)
{
    const char *property = infoName.toUtf8().constData();

    if (listLayout->count() > currentListRom)
        return listLayout->itemAt(currentListRom)->widget()->property(property).toString();
    return "";
}


QWidget *ListView::getCurrentRomWidget()
{
    return listLayout->itemAt(currentListRom)->widget();
}


bool ListView::hasSelectedRom()
{
    return listCurrent;
}


void ListView::highlightListWidget(QWidget *current)
{
    current->setFocus();

    QLayoutItem *listItem;
    for (int item = 0; (listItem = listLayout->itemAt(item)) != nullptr; item++)
    {
        if (listItem->widget() == current)
            currentListRom = item;
    }

    //Give current left margin to stand out
    //Delay with QTimer so right-click menu appears correctly
    QTimer::singleShot(5, this, SLOT(highlightListWidgetSetMargin()));

    listCurrent = true;
    emit listItemSelected(true);
}


void ListView::highlightListWidgetSetMargin()
{
    //Reset all margins
    QLayoutItem *listItem;
    for (int item = 0; (listItem = listLayout->itemAt(item)) != nullptr; item++)
        listItem->widget()->setContentsMargins(0, 0, 20, 0);

    getCurrentRomWidget()->setContentsMargins(20, 0, 0, 0);
}


void ListView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Down && listLayout->count() > 0) {
        highlightListWidget(listLayout->itemAt(0)->widget());
        ensureWidgetVisible(listLayout->itemAt(0)->widget());
    } else
        QScrollArea::keyPressEvent(event);
}


void ListView::resetView()
{
    QLayoutItem *listItem;
    while ((listItem = listLayout->takeAt(0)) != nullptr)
    {
        delete listItem->widget();
        delete listItem;
    }

    listCurrent = false;
}


void ListView::saveListPosition()
{
    positionx = horizontalScrollBar()->value();
    positiony = verticalScrollBar()->value();

    if (listCurrent)
        savedListRom = currentListRom;
    else
        savedListRom = -1;
    savedListRomFilename = getCurrentRomInfo("fileName");
}


void ListView::selectNextRom(QWidget* current, QString keypress)
{
    int offset = 0;
    if (keypress == "UP" || keypress == "LEFT")
        offset = -2;
    else if (keypress == "DOWN" || keypress == "RIGHT")
        offset = 2;

    QLayoutItem *listItem;
    for (int item = 0; (listItem = listLayout->itemAt(item)) != nullptr; item++)
    {
        if (listItem->widget() == current && item + offset >= 0 && listLayout->itemAt(item + offset) != nullptr) {
            ensureWidgetVisible(listLayout->itemAt(item + offset)->widget());
            highlightListWidget(listLayout->itemAt(item + offset)->widget());
        }
    }
}


void ListView::setListBackground()
{
    if (SETTINGS.value("List/theme","Light").toString() == "Dark")
        setStyleSheet("#listView { border: none; background: #222; } #listWidget { background: transparent; }");
    else
        setStyleSheet("#listView { border: none; background: #FFF; } #listWidget { background: transparent; }");
}


void ListView::setListPosition()
{
    horizontalScrollBar()->setValue(positionx);
    verticalScrollBar()->setValue(positiony);

    //Restore selected ROM if it is in the same position
    if (savedListRom != -1 && listLayout->count() > savedListRom) {
        QWidget *checkWidget = listLayout->itemAt(savedListRom)->widget();
        if (checkWidget->property("fileName").toString() == savedListRomFilename)
            highlightListWidget(checkWidget);
    }
}
