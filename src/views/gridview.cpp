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

#include "gridview.h"

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


GridView::GridView(QWidget *parent) : QScrollArea(parent)
{
    this->parent = parent;

    setObjectName("gridView");
    setStyleSheet("#gridView { border: none; }");
    setBackgroundRole(QPalette::Dark);
    setAlignment(Qt::AlignHCenter);
    setHidden(true);

    setGridBackground();


    gridWidget = new QWidget(this);
    gridWidget->setObjectName("gridWidget");
    gridWidget->setStyleSheet("#gridWidget { background: transparent; }");
    setWidget(gridWidget);

    gridLayout = new QGridLayout(gridWidget);
    gridLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    gridLayout->setRowMinimumHeight(0, 10);

    gridWidget->setLayout(gridLayout);

    gridCurrent = false;
    currentGridRom = 0;
}


void GridView::addToGridView(Rom *currentRom, int count, bool ddEnabled)
{
    if (ddEnabled) // Add place for "No Cart" entry
        count++;

    ClickableWidget *gameGridItem = new ClickableWidget(gridWidget);
    gameGridItem->setMinimumWidth(getGridSize("width"));
    gameGridItem->setMaximumWidth(getGridSize("width"));
    gameGridItem->setGraphicsEffect(getShadow(false));
    gameGridItem->setContextMenuPolicy(Qt::CustomContextMenu);

    //Assign ROM data to widget for use in click events
    gameGridItem->setProperty("fileName", currentRom->fileName);
    gameGridItem->setProperty("directory", currentRom->directory);
    if (currentRom->goodName == getTranslation("Unknown ROM") ||
        currentRom->goodName == getTranslation("Requires catalog file"))
        gameGridItem->setProperty("search", currentRom->internalName);
    else
        gameGridItem->setProperty("search", currentRom->goodName);
    gameGridItem->setProperty("romMD5", currentRom->romMD5);
    gameGridItem->setProperty("zipFile", currentRom->zipFile);

    QGridLayout *gameGridLayout = new QGridLayout(gameGridItem);
    gameGridLayout->setColumnStretch(0, 1);
    gameGridLayout->setColumnStretch(3, 1);
    gameGridLayout->setRowMinimumHeight(1, getImageSize("Grid").height());

    QLabel *gridImageLabel = new QLabel(gameGridItem);
    gridImageLabel->setMinimumHeight(getImageSize("Grid").height());
    gridImageLabel->setMinimumWidth(getImageSize("Grid").width());
    QPixmap image;

    if (currentRom->imageExists) {
        //Use uniform aspect ratio to account for fluctuations in TheGamesDB box art
        Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio;

        //Don't warp aspect ratio though if image is too far away from standard size (JP box art)
        double aspectRatio = double(currentRom->image.width()) / currentRom->image.height();

        if (aspectRatio < 1.1 || aspectRatio > 1.8)
            aspectRatioMode = Qt::KeepAspectRatio;

        image = currentRom->image.scaled(getImageSize("Grid"), aspectRatioMode, Qt::SmoothTransformation);
    } else {
        if (ddEnabled && count == 0)
            image = QPixmap(":/images/no-cart.png").scaled(getImageSize("Grid"), Qt::IgnoreAspectRatio,
                                                             Qt::SmoothTransformation);
        else
            image = QPixmap(":/images/not-found.png").scaled(getImageSize("Grid"), Qt::IgnoreAspectRatio,
                                                             Qt::SmoothTransformation);
    }

    gridImageLabel->setPixmap(image);
    gridImageLabel->setAlignment(Qt::AlignCenter);
    gameGridLayout->addWidget(gridImageLabel, 1, 1);

    if (SETTINGS.value("Grid/label","true") == "true") {
        QLabel *gridTextLabel = new QLabel(gameGridItem);

        //Don't allow label to be wider than image
        gridTextLabel->setMaximumWidth(getImageSize("Grid").width());

        QString text = "";
        QString labelText = SETTINGS.value("Grid/labeltext","Filename").toString();

        text = getRomInfo(labelText, currentRom);

        if (ddEnabled && count == 0)
            text = tr("No Cart");

        gridTextLabel->setText(text);

        QString textHex = getColor(SETTINGS.value("Grid/labelcolor","White").toString()).name();
        int fontSize = getGridSize("font");

        gridTextLabel->setStyleSheet("QLabel { font-weight: bold; color: " + textHex + "; font-size: "
                                     + QString::number(fontSize) + "px; }");
        gridTextLabel->setWordWrap(true);
        gridTextLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

        gameGridLayout->addWidget(gridTextLabel, 2, 1);
    }

    gameGridItem->setLayout(gameGridLayout);

    gameGridItem->setMinimumHeight(gameGridItem->sizeHint().height());

    int columnCount;
    if (SETTINGS.value("Grid/autocolumns","true").toString() == "true")
        columnCount = viewport()->width() / (getGridSize("width") + 10);
    else
        columnCount = SETTINGS.value("Grid/columncount", "4").toInt();

    if (columnCount == 0) columnCount = 1;

    gridLayout->addWidget(gameGridItem, count / columnCount + 1, count % columnCount + 1);
    gridWidget->adjustSize();

    connect(gameGridItem, SIGNAL(singleClicked(QWidget*)), this, SLOT(highlightGridWidget(QWidget*)));
    connect(gameGridItem, SIGNAL(doubleClicked(QWidget*)), parent, SLOT(launchRomFromWidget(QWidget*)));
    connect(gameGridItem, SIGNAL(arrowPressed(QWidget*, QString)), this, SLOT(selectNextRom(QWidget*, QString)));
    connect(gameGridItem, SIGNAL(enterPressed(QWidget*)), parent, SLOT(launchRomFromWidget(QWidget*)));
    connect(gameGridItem, SIGNAL(customContextMenuRequested(const QPoint &)), parent, SLOT(showRomMenu(const QPoint &)));
}


int GridView::getCurrentRom()
{
    return currentGridRom;
}


QString GridView::getCurrentRomInfo(QString infoName)
{
    const char *property = infoName.toUtf8().constData();

    if (gridLayout->count() > currentGridRom)
        return gridLayout->itemAt(currentGridRom)->widget()->property(property).toString();
    return "";
}


QWidget *GridView::getCurrentRomWidget()
{
    return gridLayout->itemAt(currentGridRom)->widget();
}


bool GridView::hasSelectedRom()
{
    return gridCurrent;
}


void GridView::highlightGridWidget(QWidget *current)
{
    current->setFocus();

    //Set all to inactive shadow
    QLayoutItem *gridItem;
    for (int item = 0; (gridItem = gridLayout->itemAt(item)) != nullptr; item++)
    {
        gridItem->widget()->setGraphicsEffect(getShadow(false));

        if (gridItem->widget() == current)
            currentGridRom = item;
    }

    //Set current to active shadow
    current->setGraphicsEffect(getShadow(true));

    gridCurrent = true;
    emit gridItemSelected(true);
}


void GridView::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Down || event->key() == Qt::Key_Right) && gridLayout->count() > 0) {
        highlightGridWidget(gridLayout->itemAt(0)->widget());
        ensureWidgetVisible(gridLayout->itemAt(0)->widget());
    } else
        QScrollArea::keyPressEvent(event);
}


void GridView::resetView()
{
    QLayoutItem *gridItem;
    while ((gridItem = gridLayout->takeAt(0)) != nullptr)
    {
        delete gridItem->widget();
        delete gridItem;
    }

    gridCurrent = false;
}


void GridView::resizeEvent(QResizeEvent *event)
{
    int check = event->size().width() / (getGridSize("width") + 10);
    bool autoAdjustColumns = SETTINGS.value("Grid/autocolumns","true").toString() == "true";

    if (autoAdjustColumns && check != autoColumnCount && check != 0) {
        autoColumnCount = check;
        updateGridColumns(event->size().width());
    } else
        QScrollArea::resizeEvent(event);
}


void GridView::saveGridPosition()
{
    positionx = horizontalScrollBar()->value();
    positiony = verticalScrollBar()->value();

    if (gridCurrent)
        savedGridRom = currentGridRom;
    else
        savedGridRom = -1;
    savedGridRomFilename = getCurrentRomInfo("fileName");
}


void GridView::selectNextRom(QWidget* current, QString keypress)
{
    int columnCount;
    if (SETTINGS.value("Grid/autocolumns","true").toString() == "true")
        columnCount = autoColumnCount;
    else
        columnCount = SETTINGS.value("Grid/columncount", "4").toInt();

    int offset = 0;
    if (keypress == "UP")
        offset = columnCount * -1;
    else if (keypress == "DOWN")
        offset = columnCount;
    else if (keypress == "RIGHT")
        offset = 1;
    else if (keypress == "LEFT")
        offset = -1;

    QLayoutItem *gridItem;
    for (int item = 0; (gridItem = gridLayout->itemAt(item)) != nullptr; item++)
    {
        if (gridItem->widget() == current && item + offset >= 0 && gridLayout->itemAt(item + offset) != nullptr) {
            ensureWidgetVisible(gridLayout->itemAt(item + offset)->widget());
            highlightGridWidget(gridLayout->itemAt(item + offset)->widget());
        }
    }
}


void GridView::setGridBackground()
{
    QString theme = SETTINGS.value("Grid/theme", "Normal").toString();
    if (theme == "Light")
        setStyleSheet("#gridView { border: none; background: #FFF; } #gridWidget { background: transparent; }");
    else if (theme == "Dark")
        setStyleSheet("#gridView { border: none; background: #222; } #gridWidget { background: transparent; }");
    else
        setStyleSheet("#gridView { border: none; }");

    QString background = SETTINGS.value("Grid/background", "").toString();
    if (background != "") {
        QFile backgroundFile(background);

        if (backgroundFile.exists() && !QFileInfo(backgroundFile).isDir())
            setStyleSheet(QString()
                + "#gridView { "
                    + "border: none; "
                    + "background: url(" + background + "); "
                    + "background-attachment: fixed; "
                    + "background-position: top center; "
                + "} #gridWidget { background: transparent; } "
            );
    }
}


void GridView::setGridPosition()
{
    horizontalScrollBar()->setValue(positionx);
    verticalScrollBar()->setValue(positiony);

    //Restore selected ROM if it is in the same position
    if (savedGridRom != -1 && gridLayout->count() > savedGridRom) {
        QWidget *checkWidget = gridLayout->itemAt(savedGridRom)->widget();
        if (checkWidget->property("fileName").toString() == savedGridRomFilename)
            highlightGridWidget(checkWidget);
    }

    if (SETTINGS.value("Grid/autocolumns","true").toString() == "true")
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    else
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}


void GridView::updateGridColumns(int width)
{
    int columnCount = width / (getGridSize("width") + 10);

    int gridCount = gridLayout->count();
    QList<QWidget*> gridItems;
    for (int count = 0; count < gridCount; count++)
        gridItems << gridLayout->takeAt(0)->widget();

    int count = 0;
    foreach(QWidget *gridItem, gridItems)
    {
        gridLayout->addWidget(gridItem, count / columnCount + 1, count % columnCount + 1);
        count++;
    }

    gridWidget->adjustSize();
}

