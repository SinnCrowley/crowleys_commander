#include <QFileInfo>
#include "mysortfilterproxymodel.h"
#include "myfilesystemmodel.h"
#include "archivemanager.h"

MySortFilterProxyModel::MySortFilterProxyModel(QString path, const QString position, QWidget *parent)
{
    Q_UNUSED(parent);
    fsModel = new MyFileSystemModel(path, position, this);

    setRecursiveFilteringEnabled(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSourceModel(fsModel);
}

bool MySortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    MyFileSystemModel *fsModel = this->fsModel;
    bool asc = sortOrder() == Qt::AscendingOrder ? true : false;

    if (fsModel->isInArchiveMode()) {
        const QList<ArchiveEntry> entries = fsModel->archiveEntriesList();

        if (left.row() >= entries.size() || right.row() >= entries.size())
            return false;

        const ArchiveEntry leftEntry = entries[left.row()];
        const ArchiveEntry rightEntry = entries[right.row()];

        if (leftEntry.name == "..")
            return asc;
        if (rightEntry.name == "..")
            return !asc;

        // Move dirs upper (not dir shortcuts)
        bool leftIsDir = leftEntry.isDir && !QFileInfo(leftEntry.name).isSymLink();
        bool rightIsDir = rightEntry.isDir && !QFileInfo(rightEntry.name).isSymLink();

        if (!leftIsDir && rightIsDir) {
            return !asc;
        }
        if (leftIsDir && !rightIsDir) {
            return asc;
        }

        QString leftName = leftEntry.name.toLower();
        QString rightName = rightEntry.name.toLower();

        QString leftType = leftEntry.name.right(leftEntry.name.lastIndexOf('.')).toLower();
        QString rightType = rightEntry.name.right(rightEntry.name.lastIndexOf('.')).toLower();

        // sorting by name
        if (sortColumn() == 0) {
            // at first, trying to sort by extension
            if (leftName != rightName)
                return leftName < rightName;

            // if names are the same, sorting by extension
            return leftType < rightType;
        }

        // sorting by type
        if (sortColumn() == 1) {

            if (!leftEntry.isDir && !rightEntry.isDir) {
                // at first, trying to sort by extension
                if (leftType != rightType)
                    return leftType < rightType;

                // if extensions are the same, sorting by name
                return leftName < rightName;
            } else {
                return leftName < rightName;
            }
        }
        // sorting by size
        if (sortColumn() == 2) {
            qint64 leftSize = leftEntry.size;
            qint64 rightSize = rightEntry.size;
            return leftSize < rightSize;
        }

        // sorting by modification date
        if (sortColumn() == 3) {
            QDateTime leftTime = leftEntry.modified;
            QDateTime rightTime = rightEntry.modified;

            if (leftTime != rightTime)
                return leftTime < rightTime;

            return leftName < rightName;
        }
    } else {
        QFileInfo leftFileInfo  = fsModel->fileInfo(left);
        QFileInfo rightFileInfo = fsModel->fileInfo(right);

        // If DotAndDot move in the beginning
        if (leftFileInfo.fileName() == "..")
            return asc;
        if (rightFileInfo.fileName() == "..")
            return !asc;

        // Move dirs upper (not dir shortcuts)
        bool leftIsDir = leftFileInfo.isDir() && !leftFileInfo.isSymLink();
        bool rightIsDir = rightFileInfo.isDir() && !rightFileInfo.isSymLink();

        if (!leftIsDir && rightIsDir) {
            return !asc;
        }
        if (leftIsDir && !rightIsDir) {
            return asc;
        }

        QString leftName = leftFileInfo.fileName().toLower();
        QString rightName = rightFileInfo.fileName().toLower();

        QString leftType = leftFileInfo.suffix().toLower();
        QString rightType = rightFileInfo.suffix().toLower();

        // sorting by name
        if (sortColumn() == 0) {
            // at first, trying to sort by extension
            if (leftName != rightName)
                return leftName < rightName;

            // if names are the same, sorting by extension
            return leftType < rightType;
        }

        // sorting by type
        if (sortColumn() == 1) {
            if (!leftFileInfo.isDir() && !rightFileInfo.isDir()) {
                // at first, trying to sort by extension
                if (leftType != rightType)
                    return leftType < rightType;

                // if extensions are the same, sorting by name
                return leftName < rightName;
            } else {
                return leftName < rightName;
            }
        }

        // sorting by size
        if (sortColumn() == 2) {
            qint64 leftSize = leftFileInfo.size();
            qint64 rightSize = rightFileInfo.size();
            return leftSize < rightSize;
        }

        // sorting by modification date
        if (sortColumn() == 3) {
            QDateTime leftTime = leftFileInfo.lastModified();
            QDateTime rightTime = rightFileInfo.lastModified();
            return leftTime < rightTime;
        }
    }

    return QSortFilterProxyModel::lessThan(left, right);
}


