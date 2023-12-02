#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <QTreeView>

class MyTreeView : public QTreeView {
    Q_OBJECT
public:
    MyTreeView(QWidget *parent = nullptr);
    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event);
};

#endif // MYTREEVIEW_H
