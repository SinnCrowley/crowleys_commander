#ifndef MYFILESYSTEMMODEL_H
#define MYFILESYSTEMMODEL_H

#include <QAbstractTableModel>
#include <QFileInfo>
#include <QProgressDialog>
#include <QDir>
#include <QFileSystemWatcher>

#include "archivemanager.h"

struct RemovePolicy {
    bool removeAllReadOnly = false;
    bool skipAllReadOnly = false;
};

class MyFileSystemModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    MyFileSystemModel(const QString path, const QString position, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QString rootPath() const;
    QFileInfo fileInfo(const QModelIndex &index) const;
    QString path(const QModelIndex &index) const;
    QString filePath(const QModelIndex &index) const;

    using QAbstractTableModel::index;
    QModelIndex index(const QString &path, int column = 0) const;

    bool copyFiles(QStringList source, QString targetPath);
    bool copyDirectory(const QString &sourceDirPath, const QString &targetDirPath, QProgressDialog &progress, qint64 &copiedSize, qint64 totalSize);
    qint64 calculateDirectorySize(const QFileInfo &file);
    bool removeFileWithPolicy(const QString &filePath, RemovePolicy &policy);
    bool removeDirectory(const QString &dirPath, QProgressDialog *progressDialog, int &count, int total, RemovePolicy &policy);
    int countEntriesInDirectory(const QString &dirPath);

    // Drag and Drop
    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    void setFilter(QDir::Filters newFilters);
    QDir::Filters filter();

    ArchiveManager archiveManager;
    bool inArchiveMode = false;
    QString archiveInternalPath = "/";
    QList<ArchiveEntry> archiveEntries;
    QList<ArchiveEntry> archiveEntriesList() const { return archiveEntries; }
    bool isInArchiveMode() const { return inArchiveMode; }

signals:
    void numberPopulated(const QString &path, int start, int number, int total);

    // signal after external deleting current directory
    void rootPathChanged(const QString &newPath, const QString &position);

    // signals for focus reseting after model reset
    void beforeReset();
    void afterReset();

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
    QString m_position;
};

#endif // MYFILESYSTEMMODEL_H
