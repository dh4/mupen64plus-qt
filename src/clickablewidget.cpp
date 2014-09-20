#include "clickablewidget.h"


ClickableWidget::ClickableWidget(QWidget *parent) : QWidget(parent)
{
}


void ClickableWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit singleClicked(this);
}


void ClickableWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit doubleClicked(this);
}
