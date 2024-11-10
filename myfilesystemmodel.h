#ifndef MYFILESYSTEMMODEL_H
#define MYFILESYSTEMMODEL_H

/*#include <QFileSystemModel>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QProgressDialog>

class MyFileSystemModel : public QFileSystemModel {
    Q_OBJECT

public:
    MyFileSystemModel(const QString path, QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool copyFiles(QStringList source, QString targetPath);
    bool copyDirectory(const QString &sourceDirPath, const QString &targetDirPath, QProgressDialog &progress, qint64 &copiedSize);
    qint64 calculateDirectorySize(const QFileInfo &file);
    void updateProgress(QStringList fileList, QProgressDialog &dialog);

    // Drag and Drop
    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;


protected:
    void fetchMore(const QModelIndex &parent) override;
    bool canFetchMore(const QModelIndex &parent) const override;

private:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
};*/




#include <QAbstractItemModel>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QProgressDialog>
#include <QDir>
#include <QFileSystemWatcher>

class MyFileSystemModel : public QAbstractTableModel {
    Q_OBJECT

public:
    MyFileSystemModel(const QString path, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QString rootPath() const;
    QFileInfo fileInfo(const QModelIndex &index) const;
    QString path(const QModelIndex &index) const;
    QString filePath(const QModelIndex &index) const;
    QModelIndex index(const QString &path, int column = 0) const;

    bool copyFiles(QStringList source, QString targetPath);
    bool copyDirectory(const QString &sourceDirPath, const QString &targetDirPath, QProgressDialog &progress, qint64 &copiedSize);
    qint64 calculateDirectorySize(const QFileInfo &file);

    // Drag and Drop
    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    void setFilter(QDir::Filters newFilters);
    QDir::Filters filter();

signals:
    void numberPopulated(const QString &path, int start, int number, int total);

public slots:
    void setRootPath(const QString &path);

private slots:
    void onDirectoryChanged(const QString &path);
    void onFileChanged(const QString &path);

protected:
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

private:
    QFileInfoList fileList;
    QString m_path;
    int fileCount = 0;
    int batchSize = 1000;
    QFlags<QDir::Filter> filters;
    QFileSystemWatcher *fileSystemWatcher;

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
};




#endif // MYFILESYSTEMMODEL_H
