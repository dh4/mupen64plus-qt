#ifndef TREEWIDGETITEM_H
#define TREEWIDGETITEM_H

#include <QTreeWidgetItem>


class TreeWidgetItem : public QTreeWidgetItem
{
public:
    TreeWidgetItem(QTreeWidget *parent) : QTreeWidgetItem(parent) {}
    bool operator< (const QTreeWidgetItem &other) const;
};


#endif // TREEWIDGETITEM_H
