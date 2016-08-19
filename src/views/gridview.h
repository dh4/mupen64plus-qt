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

#ifndef GRIDVIEW_H
#define GRIDVIEW_H

#include <QScrollArea>

class QGridLayout;
struct Rom;


class GridView : public QScrollArea
{
    Q_OBJECT

public:
    explicit GridView(QWidget *parent = 0);
    void addToGridView(Rom *currentRom, int count, bool ddEnabled);
    int getCurrentRom();
    QString getCurrentRomInfo(QString infoName);
    QWidget *getCurrentRomWidget();
    bool hasSelectedRom();
    void resetView();
    void saveGridPosition();
    void setGridBackground();

protected:
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);

signals:
    void gridItemSelected(bool active);

private:
    void updateGridColumns(int width);

    int autoColumnCount;
    int currentGridRom;
    bool gridCurrent;
    int savedGridRom;
    QString savedGridRomFilename;
    int positionx;
    int positiony;

    QGridLayout *gridLayout;
    QWidget *gridWidget;
    QWidget *parent;

private slots:
    void highlightGridWidget(QWidget *current);
    void selectNextRom(QWidget *current, QString keypress);
    void setGridPosition();
};

#endif // GRIDVIEW_H
