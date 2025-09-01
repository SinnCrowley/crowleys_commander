#ifndef ARCHIVEMANAGER_H
#define ARCHIVEMANAGER_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QList>
#include <QDateTime>
#include <QIcon>
#include <archive.h>
#include <archive_entry.h>

struct ArchiveEntry {
    QString path;
    QString name;
    qint64 size = 0;
    qint64 compressedSize = 0; // Libarchive does not provide compressed size directly
    bool isDir = false;
    QDateTime modified;
    QIcon icon;
};

class ArchiveManager : public QObject {
    Q_OBJECT
public:
    ArchiveManager();
    ~ArchiveManager();

    bool isArchive(const QString& filePath) const;
    bool isOpen() const;
    bool openArchive(const QString& filePath, const QString& password = "");
    void close();
    QString getCurrentArchivePath() const;
    QString getArchiveInfo() const;
    QList<ArchiveEntry> listEntries(const QString& internalPath) const;
    QByteArray extractFile(const QString& internalPath) const;
    bool createArchive(const QString& outputPath, const QStringList& files,
                       const QString& password = "", const QString& format = "zip");

    QString normalizePath(const QString& path) const;
    QString getParentPath(const QString& path) const;

private:
    QString currentArchivePath;
    QString currentPassword;
    QHash<QString, ArchiveEntry> allEntries;
    mutable QHash<QString, QList<ArchiveEntry>> pathCache;
    bool archiveOpen = false;

    bool addToArchive(struct archive *a, const QString &filePath, const QString &arcPath) const;
};

#endif // ARCHIVEMANAGER_H
