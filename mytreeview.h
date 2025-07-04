#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <QTreeView>
#include <QKeyEvent>
#include "mysortfilterproxymodel.h"

class MyTreeView : public QTreeView {
    Q_OBJECT
public:
    MyTreeView(const QString path, const QString position, QWidget *parent);
    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) override;
    void edit(const QModelIndex &index);

    void scrollToFile();
    MySortFilterProxyModel *sortModel;
    // set focus after directory changing
    void forceFocusAfterLayout();
    // set focus after model updating
    void connectModelSignals();
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    // set focus after renaming
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;

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

    int savedRow = -1;
    bool hasFocusNow = false;
};

#endif // MYTREEVIEW_H
