#ifndef MYFILESYSTEMMODEL_H
#define MYFILESYSTEMMODEL_H

#include <QCoreApplication>
#include <QtGui>

class MyFileSystemModel : public QFileSystemModel {
    Q_OBJECT
public:
    MyFileSystemModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role = Qt :: DisplayRole) const;

    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool dropMimeData(const QMimeData *data,
        Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    void copyFiles(QStringList source, const QString targetPath);
    void copyFilesWithOverwriting(const QStringList source, const QString targetPath);
    void copyDirectory(const QString source, const QString targetPath);

private:
    bool canDropMimeData(const QMimeData *data,
        Qt::DropAction action, int row, int column, const QModelIndex &parent) const;
};


#endif // MYFILESYSTEMMODEL_H
