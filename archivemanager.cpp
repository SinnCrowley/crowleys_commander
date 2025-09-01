#include "archivemanager.h"
#include <QFileInfo>
#include <QFileInfoList>
#include <QStringList>
#include <QSet>
#include <QHash>
#include <QApplication>
#include <QStyle>
#include <QInputDialog>
#include <QMessageBox>
#include <QMap>
#include <QDateTime>
#include <QDir>
#include <QFile>

ArchiveManager::ArchiveManager() {}

ArchiveManager::~ArchiveManager() {
    close();
}

static QStringList getReadableExtensions() {
    return {
        "7z", "ar", "bz2", "cab", "cpio", "deb", "dmg", "gz", "iso", "lha", "lzh", "lzip",
        "lzma", "mtree", "rar", "rpm", "tar", "warc", "xar", "xz", "z", "zip", "zst"
    };
}

static QStringList getWritableExtensions() {
    return {
        "7z", "ar", "bz2", "cpio", "gz", "iso", "lzip", "lzma", "mtree",
        "pax", "tar", "warc", "xar", "xz", "zip", "zst"
    };
}

// Check if file is an archive
bool ArchiveManager::isArchive(const QString& filePath) const {
    QString fileName = QFileInfo(filePath).fileName().toLower();
    QString suffix = QFileInfo(filePath).suffix().toLower();

    static const QStringList readableExtensions = getReadableExtensions();

    for (const QString& ext : readableExtensions) {
        if (fileName.endsWith("." + ext)) {
            return true;
        }
    }

    return readableExtensions.contains(suffix);
}

bool ArchiveManager::openArchive(const QString& filePath, const QString& password) {
    close();

    if (!QFileInfo::exists(filePath))
        return false;

    currentArchivePath = filePath;
    currentPassword = password;

    struct archive *a = archive_read_new();
    if (!a) return false;

    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    if (!password.isEmpty()) {
        archive_read_add_passphrase(a, password.toStdString().c_str());
    }

    int r = archive_read_open_filename(a, filePath.toStdString().c_str(), 10240);
    if (r != ARCHIVE_OK) {
        if (password.isEmpty() && archive_errno(a) && strstr(archive_error_string(a), "password") != nullptr) {
            bool ok;
            QString newPassword = QInputDialog::getText(nullptr, tr("Password required"),
                                                        tr("Enter archive password:"),
                                                        QLineEdit::Password, "", &ok);
            if (ok && !newPassword.isEmpty()) {
                archive_read_free(a);
                return openArchive(filePath, newPassword);
            }
        }
        QMessageBox::warning(nullptr, tr("Error"), tr("Failed to open archive: %1").arg(archive_error_string(a)));
        archive_read_free(a);
        return false;
    }

    struct archive_entry *entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        ArchiveEntry archEntry;
        const char *pathname = archive_entry_pathname_utf8(entry);
        if (!pathname) pathname = archive_entry_pathname(entry);
        archEntry.path = QDir::fromNativeSeparators(QString::fromUtf8(pathname));
        if (!archEntry.path.startsWith("/")) archEntry.path = "/" + archEntry.path;

        QStringList parts = archEntry.path.split("/", Qt::SkipEmptyParts);
        archEntry.name = parts.isEmpty() ? "" : parts.last();

        archEntry.size = archive_entry_size_is_set(entry) ? archive_entry_size(entry) : 0;
        archEntry.compressedSize = 0; // Not available in libarchive
        archEntry.isDir = (archive_entry_filetype(entry) == AE_IFDIR);
        archEntry.modified = QDateTime::fromSecsSinceEpoch(archive_entry_mtime(entry));

        if (archEntry.isDir && !archEntry.path.endsWith("/")) {
            archEntry.path += "/";
        }

        allEntries[archEntry.path] = archEntry;
        archive_read_data_skip(a);
    }

    r = archive_read_free(a);
    if (r != ARCHIVE_OK) {
        QMessageBox::warning(nullptr, tr("Error"), tr("Failed to read archive: %1").arg(archive_error_string(a)));
        return false;
    }

    archiveOpen = true;
    return true;
}

bool ArchiveManager::createArchive(const QString& outputPath, const QStringList& files,
                                   const QString& password, const QString& format) {
    QString fmt = format.toLower();
    if (!getWritableExtensions().contains(fmt)) {
        QMessageBox::warning(nullptr, tr("Error"), tr("Unsupported archive format: %1").arg(format));
        return false;
    }

    struct archive *a = archive_write_new();
    if (!a) return false;

    // Set format and compression
    if (fmt == "zip") {
        archive_write_set_format_zip(a);
        archive_write_set_options(a, "compression=deflate");
    } else if (fmt == "7z") {
        archive_write_set_format_7zip(a);
        archive_write_set_options(a, "compression=lzma,compression-level=9");
    } else if (fmt == "tar") {
        archive_write_set_format_gnutar(a);
    } else if (fmt == "gz") {
        archive_write_add_filter_gzip(a);
        archive_write_set_format_pax_restricted(a);
    } else if (fmt == "bz2") {
        archive_write_add_filter_bzip2(a);
        archive_write_set_format_pax_restricted(a);
    } else if (fmt == "xz") {
        archive_write_add_filter_xz(a);
        archive_write_set_format_pax_restricted(a);
    } else if (fmt == "iso") {
        archive_write_set_format_iso9660(a);
    } else {
        archive_write_set_format_pax_restricted(a);
    }

    if (!password.isEmpty() && (fmt == "zip" || fmt == "7z")) {
        qDebug() << password << password.toStdString().c_str();
        archive_write_set_passphrase(a, password.toStdString().c_str());
        if (fmt == "zip") {
            archive_write_set_options(a, "encryption=aes256");
        }
    }

    int r = archive_write_open_filename(a, outputPath.toStdString().c_str());
    if (r != ARCHIVE_OK) {
        QMessageBox::warning(nullptr, tr("Error"), tr("Failed to create archive: %1").arg(archive_error_string(a)));
        archive_write_free(a);
        return false;
    }

    for (const QString &file : files) {
        if (!addToArchive(a, file, QFileInfo(file).fileName())) {
            archive_write_free(a);
            return false;
        }
    }

    r = archive_write_close(a);
    if (r != ARCHIVE_OK) {
        QMessageBox::warning(nullptr, tr("Error"), tr("Failed to close archive: %1").arg(archive_error_string(a)));
    }
    archive_write_free(a);
    return (r == ARCHIVE_OK);
}

bool ArchiveManager::addToArchive(struct archive *a, const QString &filePath, const QString &arcPath) const {
    QFileInfo info(filePath);
    struct archive_entry *entry = archive_entry_new();
    if (!entry) return false;

    archive_entry_set_pathname(entry, arcPath.toUtf8().constData());
    archive_entry_set_size(entry, info.size());
    archive_entry_set_mtime(entry, info.lastModified().toSecsSinceEpoch(), 0);
    archive_entry_set_perm(entry, info.isDir() ? 0755 : 0644);

    if (info.isSymLink()) {
        archive_entry_set_filetype(entry, AE_IFLNK);
        archive_entry_set_symlink(entry, info.symLinkTarget().toUtf8().constData());
    } else if (info.isDir()) {
        archive_entry_set_filetype(entry, AE_IFDIR);
    } else {
        archive_entry_set_filetype(entry, AE_IFREG);
    }

    int r = archive_write_header(a, entry);
    if (r != ARCHIVE_OK) {
        archive_entry_free(entry);
        return false;
    }

    if (info.isFile()) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            archive_entry_free(entry);
            return false;
        }
        QByteArray buffer;
        while (!file.atEnd()) {
            buffer = file.read(8192);
            archive_write_data(a, buffer.constData(), buffer.size());
        }
        file.close();
    } else if (info.isDir()) {
        QDir dir(filePath);
        foreach (const QFileInfo &child, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden | QDir::System)) {
            QString childArcPath = arcPath + (arcPath.endsWith("/") ? "" : "/") + child.fileName();
            if (!addToArchive(a, child.absoluteFilePath(), childArcPath)) {
                archive_entry_free(entry);
                return false;
            }
        }
    }

    archive_entry_free(entry);
    return true;
}

QByteArray ArchiveManager::extractFile(const QString& internalPath) const {
    if (!archiveOpen) return QByteArray();

    struct archive *a = archive_read_new();
    if (!a) return QByteArray();

    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    if (!currentPassword.isEmpty()) {
        archive_read_add_passphrase(a, currentPassword.toStdString().c_str());
    }

    int r = archive_read_open_filename(a, currentArchivePath.toStdString().c_str(), 10240);
    if (r != ARCHIVE_OK) {
        archive_read_free(a);
        return QByteArray();
    }

    struct archive_entry *entry;
    QString searchPath = internalPath;
    if (searchPath.startsWith("/")) searchPath = searchPath.mid(1);

    QByteArray data;
    bool found = false;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *pathname = archive_entry_pathname_utf8(entry);
        if (!pathname) pathname = archive_entry_pathname(entry);
        QString entryPath = QString::fromUtf8(pathname);

        if (entryPath == searchPath) {
            found = true;
            if (archive_entry_filetype(entry) == AE_IFDIR) {
                QMessageBox::warning(nullptr, tr("Error"), tr("Cannot extract directory as file: %1").arg(internalPath));
                break;
            }

            const void *buff;
            size_t size;
            la_int64_t offset;
            while ((r = archive_read_data_block(a, &buff, &size, &offset)) == ARCHIVE_OK) {
                data.append(static_cast<const char*>(buff), static_cast<int>(size));
            }
            if (r < ARCHIVE_WARN) {
                QMessageBox::warning(nullptr, tr("Error"), tr("Failed to extract data: %1").arg(archive_error_string(a)));
                data.clear();
            }
            break;
        }
        archive_read_data_skip(a);
    }

    if (!found) {
        QMessageBox::warning(nullptr, tr("Error"), tr("File not found in archive: %1").arg(internalPath));
    }

    if (r != ARCHIVE_EOF && !found) {
        if (currentPassword.isEmpty() && strstr(archive_error_string(a), "password") != nullptr) {
            bool ok;
            QString password = QInputDialog::getText(nullptr, tr("Password required"),
                                                     tr("Enter file password:"),
                                                     QLineEdit::Password, "", &ok);
            if (ok && !password.isEmpty()) {
                const_cast<ArchiveManager*>(this)->currentPassword = password;
                archive_read_free(a);
                return extractFile(internalPath);
            }
        }
        QMessageBox::warning(nullptr, tr("Error"), tr("Failed to extract file: %1").arg(archive_error_string(a)));
    }

    archive_read_free(a);
    return data;
}

bool ArchiveManager::isOpen() const {
    return archiveOpen;
}

void ArchiveManager::close() {
    allEntries.clear();
    pathCache.clear();
    currentArchivePath.clear();
    currentPassword.clear();
    archiveOpen = false;
}

QString ArchiveManager::getCurrentArchivePath() const {
    return currentArchivePath;
}

QString ArchiveManager::getArchiveInfo() const {
    if (!archiveOpen) return QString();

    QFileInfo archiveFileInfo(currentArchivePath);
    QString info = QString("Archive: %1\n").arg(archiveFileInfo.fileName());
    info += QString("Format: %1\n").arg(archiveFileInfo.suffix().toUpper());
    info += QString("Size: %1 bytes\n").arg(archiveFileInfo.size());

    int fileCount = 0, dirCount = 0;
    qint64 totalSize = 0;

    for (auto it = allEntries.constBegin(); it != allEntries.constEnd(); ++it) {
        const ArchiveEntry& entry = it.value();
        if (entry.isDir) {
            dirCount++;
        } else {
            fileCount++;
            totalSize += entry.size;
        }
    }

    info += QString("Files: %1, Folders: %2\n").arg(fileCount).arg(dirCount);
    info += QString("Total uncompressed size: %1 bytes").arg(totalSize);

    return info;
}

QList<ArchiveEntry> ArchiveManager::listEntries(const QString& internalPath) const {
    if (!archiveOpen) return {};

    QString normalizedPath = normalizePath(internalPath);

    if (pathCache.contains(normalizedPath)) {
        return pathCache[normalizedPath];
    }

    QList<ArchiveEntry> entries;

    ArchiveEntry parent;
    parent.name = "..";
    parent.path = getParentPath(normalizedPath);
    parent.isDir = true;
    parent.icon = QApplication::style()->standardIcon(QStyle::SP_FileDialogToParent);
    entries.append(parent);

    QSet<QString> addedItems;

    for (auto it = allEntries.constBegin(); it != allEntries.constEnd(); ++it) {
        const ArchiveEntry& entry = it.value();
        QString entryPath = entry.path;

        if (!entryPath.startsWith(normalizedPath) || entryPath == normalizedPath) continue;

        QString relativePath = entryPath.mid(normalizedPath.length());
        if (relativePath.startsWith("/")) relativePath = relativePath.mid(1);
        if (relativePath.isEmpty() || relativePath == "./") continue;

        int slashPos = relativePath.indexOf('/');
        QString itemName = (slashPos == -1) ? relativePath : relativePath.left(slashPos);

        if (addedItems.contains(itemName)) continue;
        addedItems.insert(itemName);

        ArchiveEntry displayEntry;
        displayEntry.name = itemName;
        displayEntry.path = normalizedPath + (normalizedPath.endsWith("/") ? "" : "/") + itemName;

        if (slashPos == -1) {
            displayEntry = entry;
            displayEntry.name = itemName;
            displayEntry.icon = QApplication::style()->standardIcon(QStyle::SP_FileIcon);
        } else {
            displayEntry.isDir = true;
            displayEntry.size = 0;
            displayEntry.compressedSize = 0;
            displayEntry.icon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);

            QDateTime latestTime;
            for (auto entryIt = allEntries.constBegin(); entryIt != allEntries.constEnd(); ++entryIt) {
                const QString& checkPath = entryIt.value().path;
                if (checkPath.startsWith(displayEntry.path + "/")) {
                    const QDateTime& entryTime = entryIt.value().modified;
                    if (latestTime.isNull() || entryTime > latestTime) {
                        latestTime = entryTime;
                    }
                }
            }
            displayEntry.modified = latestTime;
        }

        entries.append(displayEntry);
    }

    pathCache[normalizedPath] = entries;
    return entries;
}

QString ArchiveManager::normalizePath(const QString& path) const {
    QString normalized = path;
    if (!normalized.startsWith("/")) normalized = "/" + normalized;
    if (normalized != "/" && normalized.endsWith("/")) normalized.chop(1);
    return normalized;
}

QString ArchiveManager::getParentPath(const QString& path) const {
    if (path == "/") return "..";
    QString p = path;
    if (p.endsWith("/")) p.chop(1);
    int lastSlash = p.lastIndexOf("/");
    return (lastSlash <= 0) ? "/" : p.left(lastSlash + 1);
}
