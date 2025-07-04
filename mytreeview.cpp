#include "mytreeview.h"
#include "styletweaks.h"

#include <QApplication>
#include <QItemSelectionModel>
#include <QDrag>
#include <QHeaderView>
#include <QTimer>
#include <QPointer>

MyTreeView::MyTreeView(const QString path, const QString position, QWidget *parent)
{
    Q_UNUSED(parent);

    sortModel = new MySortFilterProxyModel(path, position, this);
    setModel(sortModel);
    setRootIndex(sortModel->mapFromSource(sortModel->fsModel->index(path)));
    setUniformRowHeights(true);

    setEditTriggers(QAbstractItemView::NoEditTriggers);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRootIsDecorated(false);
    setAllColumnsShowFocus(true);
    setSortingEnabled(true);
    setExpandsOnDoubleClick(false);
    setItemsExpandable(false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setFocusPolicy(Qt::StrongFocus);

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::CopyAction);
    setDropIndicatorShown(true);
    setAcceptDrops(true);

    // change selection cursor to solid rectangle
    setStyle(new StyleTweaks);
    setItemDelegate(new EditRectangleDelegate);

    setColumnWidth(1, 70);
    setColumnWidth(2, 65);
    setColumnWidth(3, 110);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setStretchLastSection(false);

    connectModelSignals();
}

// Disable rename of TreeView element on double click
bool MyTreeView::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event)
{
    if (trigger == QAbstractItemView::DoubleClicked)
        return false;

    return QTreeView::edit(index, trigger, event);
}

void MyTreeView::edit(const QModelIndex &index)
{
    if (index.data().toString() != "..")
        QTreeView::edit(index);
}

// Scrolling to file after searching
void MyTreeView::scrollToFile()
{
    QModelIndex index = this->currentIndex();
    this->scrollTo(index, QAbstractItemView::PositionAtCenter);
    this->clearSelection();
}


// Reimplementation of some events for files selecting
void MyTreeView::keyPressEvent(QKeyEvent *event)
{
    QModelIndex currentIndex = this->currentIndex();

    if (event->modifiers() & Qt::ShiftModifier) {
        switch (event->key()) {
        case Qt::Key_Up:
            shiftSelect(currentIndex, -1);
            break;
        case Qt::Key_Down:
            shiftSelect(currentIndex, 1);
            break;
        default:
            QTreeView::keyPressEvent(event);
            break;
        }
    } else {
        switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageDown:
        case Qt::Key_PageUp:
        case Qt::Key_End:
        case Qt::Key_Home:
            moveCursorWithoutSelection(event->key());
            break;
        case Qt::Key_Right:
        case Qt::Key_Left:
            break;
        default:
            QTreeView::keyPressEvent(event);
            break;
        }
    }
}

void MyTreeView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
        startPos = event->pos();
        QModelIndex oldIndex = this->currentIndex();
        QModelIndex newIndex = this->indexAt(event->pos());
        if (newIndex.isValid()) {
            if (event->modifiers() & Qt::ShiftModifier) {
                if (oldIndex.row() > newIndex.row()) {
                    for (int i = newIndex.row(); i <= oldIndex.row(); i++)
                        this->selectionModel()->select(this->model()->index(i, 0, oldIndex.parent()), QItemSelectionModel::Select | QItemSelectionModel::Rows);
                } else {
                    for (int i = oldIndex.row(); i <= newIndex.row(); i++)
                        this->selectionModel()->select(this->model()->index(i, 0, oldIndex.parent()), QItemSelectionModel::Select | QItemSelectionModel::Rows);
                }
                this->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::NoUpdate);
            } else if (event->modifiers() & Qt::ControlModifier) {
                for (int i = 0; i < this->model()->columnCount(); i++) {
                    this->selectionModel()->select(oldIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                    this->selectionModel()->select(newIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                }
                this->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::NoUpdate);
            } else {
                this->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::NoUpdate);
            }
            event->accept();
            return;
        }
    } else  {
        return;
    }
    QTreeView::mousePressEvent(event);
}

void MyTreeView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        QTreeView::mouseMoveEvent(event);
        return;
    }

    if (!dragging && (event->pos() - startPos).manhattanLength() < QApplication::startDragDistance() + 15)
        return;

    dragging = true;
    QModelIndex index = this->indexAt(startPos);
    if (index.isValid()) {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData;
        if(selectionModel()->selectedIndexes().isEmpty()) {
            QModelIndexList list;
            list.append(selectionModel()->currentIndex());
            mimeData = model()->mimeData(list);
        } else {
            mimeData = model()->mimeData(selectionModel()->selectedIndexes());
        }

        drag->setMimeData(mimeData);
        drag->exec(Qt::CopyAction);
        dragging = false;

        this->clearSelection();
    }
}

void MyTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    dragging = false;
    QTreeView::mouseReleaseEvent(event);
}

void MyTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QModelIndex index = this->indexAt(event->pos());
    if (event->button() == Qt::LeftButton && index.isValid()) {
        emit activated(index);
        QTreeView::mouseDoubleClickEvent(event);
        return;
    }
    QTreeView::mouseDoubleClickEvent(event);
}

void MyTreeView::shiftSelect(const QModelIndex &currentIndex, int offset)
{
    QItemSelectionModel *selectionModel = this->selectionModel();
    QModelIndex nextIndex = this->model()->index(currentIndex.row() + offset, currentIndex.column(), currentIndex.parent());

    if (!nextIndex.isValid())
        return;

    if (selectionModel->isSelected(currentIndex))
        selectionModel->select(currentIndex, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    else
        selectionModel->select(currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);

    selectionModel->setCurrentIndex(nextIndex.siblingAtColumn(currentIndex.column()), QItemSelectionModel::NoUpdate);
}

void MyTreeView::moveCursorWithoutSelection(int key)
{
    QModelIndex currentIndex = this->currentIndex();
    QModelIndex nextIndex;

    switch (key) {
    case Qt::Key_Up:
        nextIndex = this->model()->index(currentIndex.row() - 1, 0, currentIndex.parent());
        break;
    case Qt::Key_Down:
        nextIndex = this->model()->index(currentIndex.row() + 1, 0, currentIndex.parent());
        break;
    case Qt::Key_Home:
        nextIndex = this->model()->index(0, 0, currentIndex.parent());
        break;
    case Qt::Key_End:
        nextIndex = this->model()->index(this->model()->rowCount() - 1, 0, currentIndex.parent());
        break;
    default:
        return;
    }

    if (nextIndex.isValid())
        this->selectionModel()->setCurrentIndex(nextIndex.siblingAtColumn(currentIndex.column()), QItemSelectionModel::NoUpdate);
}

void MyTreeView::forceFocusAfterLayout()
{
    QPointer<MyTreeView> self(this);

    QTimer::singleShot(0, this, [self]() {
        if (!self || !self->model())
            return;

        if (!self->hasFocusNow)
            return;

        int rowCount = self->model()->rowCount();
        if (rowCount == 0)
            return;

        int targetRow = (self->savedRow >= 0 && self->savedRow < rowCount) ? self->savedRow : 0;
        QModelIndex target = self->model()->index(targetRow, 0);

        if (target.isValid()) {
            self->selectionModel()->setCurrentIndex(target, QItemSelectionModel::NoUpdate);
        }

        self->savedRow = -1;  // clear saved row
    });
}

void MyTreeView::connectModelSignals()
{
    if (sortModel && sortModel->fsModel) {
        connect(sortModel->fsModel, &MyFileSystemModel::beforeReset, this, [this]() {
            QModelIndex idx = this->currentIndex();
            savedRow = idx.row();
        });

        connect(sortModel->fsModel, &MyFileSystemModel::afterReset, this, [this]() {
            int rowCount = sortModel->rowCount();
            if (rowCount == 0) return;

            int finalRow = (savedRow >= 0 && savedRow < rowCount) ? savedRow : 0;

            QModelIndex newIndex = this->model()->index(finalRow, 0);

            if (newIndex.isValid()) {
                this->selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::NoUpdate);
            }

            savedRow = -1;  // clear saved row
        });
    }
}

void MyTreeView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    QTreeView::closeEditor(editor, hint);

    QTimer::singleShot(0, this, [this]() {
        this->setFocus(Qt::OtherFocusReason);
    });
}

void MyTreeView::focusInEvent(QFocusEvent *event) {
    hasFocusNow = true;
    QTreeView::focusInEvent(event);
}

void MyTreeView::focusOutEvent(QFocusEvent *event) {
    hasFocusNow = false;
    QTreeView::focusOutEvent(event);
}
