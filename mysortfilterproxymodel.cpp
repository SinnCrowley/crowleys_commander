#include <QFileInfo>
#include "mysortfilterproxymodel.h"
#include "myfilesystemmodel.h"

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

    // sorting by type
    if (sortColumn() == 1) {
        QString leftType = leftFileInfo.suffix().toLower();
        QString rightType = rightFileInfo.suffix().toLower();

        // at first, trying to sort by extension
        if (leftType != rightType)
            return leftType < rightType;

        // if extensions are the same, sorting by name
        QString leftName = leftFileInfo.fileName().toLower();
        QString rightName = rightFileInfo.fileName().toLower();
        return leftName < rightName;
    }

    // sorting by size
    if (sortColumn() == 2) {
        qint64 leftSize = leftFileInfo.size();
        qint64 rightSize = rightFileInfo.size();
        return leftSize < rightSize;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}


