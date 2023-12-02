#include <QDragMoveEvent>
#include <QtGui>

#include "basetsd.h"

#include "mytreeview.h"
#include "styletweaks.h"


MyTreeView::MyTreeView(QWidget *parent) {
    Q_UNUSED(parent);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRootIsDecorated(false);
    setAllColumnsShowFocus(true);
    setSortingEnabled(true);
    setExpandsOnDoubleClick(false);
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::CopyAction);
    setDropIndicatorShown(true);
    dropIndicatorPosition();
    setAcceptDrops(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    // change selection cursor to solid rectangle
    setStyle(new StyleTweaks);
}

// Disable rename of TreeView element on double click
bool MyTreeView::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) {
    Q_UNUSED(index);
    Q_UNUSED(trigger);
    Q_UNUSED(event);
    if(trigger == QAbstractItemView::DoubleClicked)
        return false;

    return QTreeView::edit(index, trigger, event);
}


