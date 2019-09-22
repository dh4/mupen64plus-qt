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

#include "tableview.h"

#include "../global.h"
#include "../common.h"

#include "widgets/treewidgetitem.h"

#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QScrollBar>


TableView::TableView(QWidget *parent) : QTreeWidget(parent)
{ 
    this->parent = parent;

    setWordWrap(false);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setStyleSheet("QTreeView { border: none; } QTreeView::item { height: 25px; }");

    headerView = new QHeaderView(Qt::Horizontal, this);
    setHeader(headerView);
    setHidden(true);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), parent, SLOT(showRomMenu(const QPoint &)));
    connect(headerView, SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this, SLOT(saveSortOrder(int,Qt::SortOrder)));
}


void TableView::addNoCartRow()
{
    QStringList visible = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");

    fileItem = new TreeWidgetItem(this);

    if (visible.at(0) == "Game Cover") {
        fileItem->setText(6, " " + tr("No Cart"));
        fileItem->setForeground(6, QBrush(Qt::gray));
    } else {
        fileItem->setText(5, " " + tr("No Cart"));
        fileItem->setForeground(5, QBrush(Qt::gray));
    }
    addTopLevelItem(fileItem);
}


void TableView::addToTableView(Rom *currentRom)
{
    QStringList visible = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");

    if (visible.join("") == "") //Otherwise no columns, so don't bother populating
        return;

    fileItem = new TreeWidgetItem(this);

    //Filename for launching ROM
    fileItem->setText(0, currentRom->fileName);

    //Directory ROM is located in
    fileItem->setText(1, currentRom->directory);

    //GoodName or Internal Name for searching
    if (currentRom->goodName == getTranslation("Unknown ROM") ||
        currentRom->goodName == getTranslation("Requires catalog file"))
        fileItem->setText(2, currentRom->internalName);
    else
        fileItem->setText(2, currentRom->goodName);

    //MD5 for cache info
    fileItem->setText(3, currentRom->romMD5.toLower());

    //Zip file
    fileItem->setText(4, currentRom->zipFile);

    int i = 5, c = 0;
    bool addImage = false;

    foreach (QString current, visible)
    {
        QString text = getRomInfo(current, currentRom);
        fileItem->setText(i, text);

        if (current == "GoodName" || current == "Game Title") {
            if (text == getTranslation("Unknown ROM") ||
                text == getTranslation("Requires catalog file") ||
                text == getTranslation("Not found")) {
                fileItem->setForeground(i, QBrush(Qt::gray));
                fileItem->setData(i, Qt::UserRole, "ZZZ"); //end of sorting
            } else
                fileItem->setData(i, Qt::UserRole, text);
        }

        if (current == "Size")
            fileItem->setData(i, Qt::UserRole, currentRom->sortSize);

        if (current == "Release Date")
            fileItem->setData(i, Qt::UserRole, currentRom->sortDate);

        if (current == "Game Cover") {
            c = i;
            addImage = true;
        }

        QStringList center, right;

        center << "MD5" << "CRC1" << "CRC2" << "Rumble" << "ESRB" << "Genre" << "Publisher" << "Developer";
        right << "Size" << "Players" << "Save Type" << "Release Date" << "Rating";

        if (center.contains(current))
            fileItem->setTextAlignment(i, Qt::AlignHCenter | Qt::AlignVCenter);
        else if (right.contains(current))
            fileItem->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);

        i++;
    }

    addTopLevelItem(fileItem);


    if (currentRom->imageExists && addImage) {
        QPixmap image(currentRom->image.scaled(getImageSize("Table"), Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation));

        QWidget *imageContainer = new QWidget(this);
        QGridLayout *imageGrid = new QGridLayout(imageContainer);
        QLabel *imageLabel = new QLabel(imageContainer);

        imageLabel->setPixmap(image);
        imageGrid->addWidget(imageLabel, 1, 1);
        imageGrid->setColumnStretch(0, 1);
        imageGrid->setColumnStretch(2, 1);
        imageGrid->setRowStretch(0, 1);
        imageGrid->setRowStretch(2, 1);
        imageGrid->setContentsMargins(0,0,0,0);

        imageContainer->setLayout(imageGrid);

        setItemWidget(fileItem, c, imageContainer);
    }
}


QString TableView::getCurrentRomInfo(QString infoName)
{
    int index = getTableDataIndexFromName(infoName);
    return QVariant(currentItem()->data(index, 0)).toString();
}


bool TableView::hasSelectedRom()
{
    return currentItem() != nullptr;
}


void TableView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        emit enterPressed();
    else if (event->key() == Qt::Key_Down && selectedItems().count() == 0) {
        setCurrentItem(topLevelItem(0));
        emit tableActive();
    } else if (event->key() == Qt::Key_Down  && selectedItems().count() == 1) {
        int current = indexOfTopLevelItem(selectedItems().at(0));
        if (current < topLevelItemCount() - 1)
            setCurrentItem(topLevelItem(current + 1));
    } else if (event->key() == Qt::Key_Up  && selectedItems().count() == 1) {
        int current = indexOfTopLevelItem(selectedItems().at(0));
        if (current > 0)
            setCurrentItem(topLevelItem(current - 1));
    } else
        QTreeWidget::keyPressEvent(event);
}


void TableView::resetView(bool imageUpdated)
{
    QStringList tableVisible = SETTINGS.value("Table/columns", "Filename|Size").toString().split("|");

    QStringList translations;
    foreach (QString header, tableVisible) translations << getTranslation(header);

    int hidden = 5;

    saveColumnWidths();
    QStringList widths = SETTINGS.value("Table/width", "").toString().split("|");

    headerLabels.clear();
    headerLabels << "" << "" << "" << "" << "" << translations; //First 5 blank for hidden columns

    //Remove Game Cover title for aesthetics
    for (int i = 0; i < headerLabels.size(); i++)
        if (headerLabels.at(i) == getTranslation("Game Cover")) headerLabels.replace(i, "");

    setColumnCount(headerLabels.size());
    setHeaderLabels(headerLabels);
    headerView->setSortIndicatorShown(false);

    double height = 0, width = 0;
    if (tableVisible.contains("Game Cover")) {
        //Get optimal height/width for cover column
        height = getImageSize("Table").height() * 1.1;
        width = getImageSize("Table").width() * 1.2;

        setStyleSheet("QTreeView { border: none; } QTreeView::item { height: "
                               + QString::number(static_cast<int>(std::round(height))) + "px; }");
    } else
        setStyleSheet("QTreeView { border: none; } QTreeView::item { height: 25px; }");

    QStringList sort = SETTINGS.value("Table/sort", "").toString().split("|");
    if (sort.size() == 2) {
        if (sort[1] == "descending")
            headerView->setSortIndicator(tableVisible.indexOf(sort[0]) + hidden, Qt::DescendingOrder);
        else
            headerView->setSortIndicator(tableVisible.indexOf(sort[0]) + hidden, Qt::AscendingOrder);
    }

    setColumnHidden(0, true); //Hidden filename for launching emulator
    setColumnHidden(1, true); //Hidden directory of ROM location
    setColumnHidden(2, true); //Hidden goodname for searching
    setColumnHidden(3, true); //Hidden md5 for cache info
    setColumnHidden(4, true); //Hidden column for zip file

    int i = hidden;
    foreach (QString current, tableVisible)
    {
        if (i == hidden) {
            int c = i;
            if (current == "Game Cover") c++; //If first column is game cover, use next column

            if (SETTINGS.value("Table/stretchfirstcolumn", "true") == "true")
                header()->setSectionResizeMode(c, QHeaderView::Stretch);
            else
                header()->setSectionResizeMode(c, QHeaderView::Interactive);
        }

        if (widths.size() == tableVisible.size())
            setColumnWidth(i, widths[i - hidden].toInt());
        else
            setColumnWidth(i, getDefaultWidth(current, static_cast<int>(std::round(width))));

        //Overwrite saved value if switching image sizes
        if (imageUpdated && current == "Game Cover")
            setColumnWidth(i, static_cast<int>(std::round(width)));

        i++;
    }
}


void TableView::saveColumnWidths()
{
    QStringList widths;

    for (int i = 5; i < columnCount(); i++)
    {
        widths << QString::number(columnWidth(i));
    }

    if (widths.size() > 0)
        SETTINGS.setValue("Table/width", widths.join("|"));
}


void TableView::saveSortOrder(int column, Qt::SortOrder order)
{
    QString columnName = headerLabels.value(column);

    if (order == Qt::DescendingOrder)
        SETTINGS.setValue("Table/sort", columnName + "|descending");
    else
        SETTINGS.setValue("Table/sort", columnName + "|ascending");
}


void TableView::saveTablePosition()
{
    positionx = horizontalScrollBar()->value();
    positiony = verticalScrollBar()->value();

    if (selectedItems().count() > 0) {
        int index = getTableDataIndexFromName("fileName");
        savedTableRom = indexOfTopLevelItem(selectedItems().at(0));
        savedTableRomFilename = QVariant(topLevelItem(savedTableRom)->data(index, 0)).toString();
    } else {
        savedTableRom = -1;
        savedTableRomFilename = "";
    }
}


void TableView::setTablePosition()
{
    horizontalScrollBar()->setValue(positionx);
    verticalScrollBar()->setValue(positiony);

    //Restore selected ROM if it is in the same position
    if (savedTableRom >= 0) {
        int index = getTableDataIndexFromName("fileName");
        QString checkFilename = QVariant(topLevelItem(savedTableRom)->data(index, 0)).toString();
        if (savedTableRomFilename == checkFilename) {
            setCurrentItem(topLevelItem(savedTableRom));
            emit tableActive();
        }
    }
}
