#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <QTreeView>
#include <QKeyEvent>
#include <QItemSelectionModel>
#include <QDrag>
#include "mysortfilterproxymodel.h"

class MyTreeView : public QTreeView {
    Q_OBJECT
public:
    MyTreeView(const QString path, QWidget *parent);
    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) override;
    void edit(const QModelIndex &index);

    void scrollToFile();
    void updateDirectory(const QString &path);
    MySortFilterProxyModel *sortModel;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    bool dragging;
    QPoint startPos;

    void shiftSelect(const QModelIndex &currentIndex, int offset);
    void moveCursorWithoutSelection(int key);
};

#endif // MYTREEVIEW_H
