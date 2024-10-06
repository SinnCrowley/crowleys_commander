#ifndef MYFILESYSTEMMODEL_H
#define MYFILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QMimeData>
#include <QDragEnterEvent>

class MyFileSystemModel : public QFileSystemModel {
    Q_OBJECT

public:
    explicit MyFileSystemModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool copyFiles(QStringList source, QString targetPath);
    bool copyDirectory(const QString &sourceDirPath, const QString &targetDirPath);

    // Drag and Drop
    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

private:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
};

#endif // MYFILESYSTEMMODEL_H
