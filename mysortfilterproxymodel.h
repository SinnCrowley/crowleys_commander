#ifndef MYSORTFILTERPROXYMODEL_H
#define MYSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "myfilesystemmodel.h"

class MySortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    MySortFilterProxyModel(const QString path, const QString position, QWidget *parent);
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    MyFileSystemModel *fsModel;
};

#endif // MYSORTFILTERPROXYMODEL_H
