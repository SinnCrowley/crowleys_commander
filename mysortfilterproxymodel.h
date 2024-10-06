#ifndef MYSORTFILTERPROXYMODEL_H
#define MYSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class MySortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    MySortFilterProxyModel(QWidget *parent = nullptr);
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif /* MYSORTFILTERPROXYMODEL_H */
