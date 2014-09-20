#ifndef CLICKABLEWIDGET_H
#define CLICKABLEWIDGET_H

#include <QMouseEvent>
#include <QWidget>


class ClickableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClickableWidget(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

signals:
    void singleClicked(QWidget *current);
    void doubleClicked(QWidget *current);

public slots:

};

#endif // CLICKABLEWIDGET_H
