#include "myfilesystemmodel.h"

#include <QDir>
#include <QMessageBox>
#include <QMimeData>
#include <QUrl>
#include <QPainter>
#include <QDateTime>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileIconProvider>
#include <QTimer>
#include <QQueue>

MyFileSystemModel::MyFileSystemModel(const QString path, const QString position, QObject *parent)
{
    Q_UNUSED(parent);
    m_position = position;
    fileSystemWatcher = new QFileSystemWatcher(this);
    connect(fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &MyFileSystemModel::onFileChanged);
    connect(fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &MyFileSystemModel::onDirectoryChanged);

    setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files | QDir::System);
    setRootPath(path);
}


QString formatBytes(qint64 bytes) {
    QStringList units = {"B", "KB", "MB", "GB", "TB"};
    double size = bytes;
    int i = 0;
    while (size >= 1024 && i < units.size() - 1) {
        size /= 1024;
        ++i;
    }
    return QString::number(size, 'f', 2) + " " + units[i];
}

void MyFileSystemModel::setRootPath(const QString &path)
{
    QDir dir(path);

    beginResetModel();
    this->m_path = path;
    fileList = dir.entryInfoList(filters);
    fileCount = 0;
    endResetModel();

    if (!fileSystemWatcher->directories().isEmpty())
        fileSystemWatcher->removePaths(fileSystemWatcher->directories());

    do {
        fileSystemWatcher->addPath(dir.absolutePath());
    } while (dir.cdUp());

    while (canFetchMore(QModelIndex()))
        fetchMore(QModelIndex());
}

QString MyFileSystemModel::rootPath() const
{
    return this->m_path;
}

QFileInfo MyFileSystemModel::fileInfo(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= fileList.size())
        return QFileInfo();

    return fileList.at(index.row());
}

QString MyFileSystemModel::path(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= fileList.size())
        return "";

    return fileList.at(index.row()).absoluteFilePath();
}

QString MyFileSystemModel::filePath(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= fileList.size())
        return "";

    return fileList.at(index.row()).absoluteFilePath();
}

QModelIndex MyFileSystemModel::index(const QString &path, int column) const
{
    for (int row = 0; row < fileList.size(); ++row) {
        if (fileList[row].absoluteFilePath() == path)
            return createIndex(row, column, const_cast<QFileInfo*>(&fileList[row]));
    }

    return QModelIndex();
}


// Data loading
bool MyFileSystemModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid())
        return false;

    return (fileCount < fileList.size());
}

void MyFileSystemModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
        return;

    const int start = fileCount;
    const int remainder = int(fileList.size()) - start;
    const int itemsToFetch = qMin(batchSize, remainder);

    if (itemsToFetch <= 0)
        return;

    beginInsertRows(QModelIndex(), start, start + itemsToFetch - 1);
    fileCount += itemsToFetch;
    endInsertRows();

    emit numberPopulated(m_path, start, itemsToFetch, int(fileList.size()));
}


// Data presentation
int MyFileSystemModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : fileCount;
}

int MyFileSystemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant MyFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (index.column() == 0 && role == Qt::DecorationRole ) {
        QFileInfo info = fileInfo(index);
        QFileIconProvider provider;
        QIcon icon = provider.icon(info);

        if (info.fileName() == "..")
            return QIcon(QPixmap(":/icons/icons/back.png"));

        QPixmap pixmap(32,32);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);

        if (info.isHidden())
            painter.setOpacity(0.7);

        icon.paint(&painter, QRect(0, 0, 32, 32));
        painter.end();
        return QIcon(pixmap);
    }

    if (role == Qt::DisplayRole) {
        QFileInfo info = fileInfo(index);

        // files and folders that start with dot (.gitignore, .atom)
        if (info.completeBaseName() == "") {
            if(index.column() == 0)
                return "." + info.suffix();
            if(index.column() == 1) {
                if(info.isDir())
                    return "Folder";
                else
                    return "";
            }
        }

        if (index.column() == 0) {
            if (info.isDir() && !info.isSymLink())
                return info.fileName();
            else
                return info.completeBaseName();
        }

        if (index.column() == 1) {
            if (info.isDir() && !info.isSymLink())
                return QString("Folder");
            else if (info.isSymLink())
                return QString("Shortcut");
            else
                return info.suffix().toLower();
        }

        if (index.column() == 2) {
            if (info.isDir() && !info.isSymLink()) {
                return "";
            } else {
                qint64 size = info.size();
                float normalizedSize;
                if (size > 1073741824) {
                    normalizedSize = size / 1073741824.;
                    return QString::number(normalizedSize, 'f', 2) + " Gb";
                }
                if (size > 1048576) {
                    normalizedSize = size / 1048576.;
                    return QString::number(normalizedSize, 'f', 2) + " Mb";
                }
                if (size > 1024) {
                    normalizedSize = size / 1024.;
                    return QString::number(normalizedSize, 'f', 2) + " Kb";
                }
                return QString::number(size) + " b";
            }
        }

        if (index.column() == 3)
            return info.metadataChangeTime();
    }

    if (role == Qt::EditRole) {
        QFileInfo info = fileInfo(index);

        if(index.column() == 0)
            return info.fileName();
    }

    return {};
}

QVariant MyFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Type");
        case 2:
            return tr("Size");
        case 3:
            return tr("Date modified");
        default:
            return QVariant();
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

bool MyFileSystemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.column() == 0) {
        QFileInfo info = fileInfo(index);
        QString oldFilePath = info.filePath();
        QString newFilePath = info.absolutePath() + "/" + value.toString();

        if (info.isSymLink() || info.isDir() || info.isFile()) {
            if (newFilePath == oldFilePath)
                return false;

            if (QFile::rename(oldFilePath, newFilePath)) {
                // update model if successs
                emit dataChanged(index, index);
                return true;
            } else {
                // errors handling
                QMessageBox::warning(nullptr, "Rename error", "Failed to rename the file.");
                return false;
            }
        }
    }
    return QAbstractTableModel::setData(index, value, role);
}


qint64 MyFileSystemModel::calculateDirectorySize(const QFileInfo &file)
{
    qint64 totalSize = 0;
    QQueue<QString> directories;
    directories.enqueue(file.absoluteFilePath());

    while (!directories.isEmpty()) {
        QString currentDirPath = directories.dequeue();
        QDir dir(currentDirPath);

        QFileInfoList entries = dir.entryInfoList(
            QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden | QDir::System
            );

        for (const QFileInfo &entry : entries) {
            if (entry.isSymLink()) continue;

            if (entry.isDir()) {
                directories.enqueue(entry.absoluteFilePath());
            } else {
                totalSize += entry.size();
            }
        }
    }

    return totalSize;
}


bool MyFileSystemModel::copyFiles(QStringList source, QString targetPath) {
    if (targetPath.at(targetPath.length() - 1) != '/')
        targetPath.append('/');

    QProgressDialog *progressDialog = new QProgressDialog("Copying files...", "Cancel", 0, source.size(), nullptr);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setWindowTitle("Copy Progress");
    progressDialog->setMinimumDuration(50);


    // create destination folder if not exists
    QDir destinationDir(targetPath);
    if (!destinationDir.exists()) {
        if (!destinationDir.mkdir(targetPath)) {
            QMessageBox::warning(nullptr, "Error", "Failed to create the destination folder.");
            return false;
        }
    }

    // disable copying to the same path
    if (targetPath == (source[0].left(source[0].lastIndexOf(QChar('/'))) + '/')) {
        QMessageBox::warning(nullptr, "Error", "Cannot copy file into the same folder.");
        return false;
    }

    qint64 totalSize = 0;
    qint64 copiedSize = 0;

    bool overwriteAll = false;
    bool skipAll = false;

    foreach (const QString &file, source) {
        QFileInfo fileInfo(file);
        if(fileInfo.isDir())
            totalSize += calculateDirectorySize(fileInfo);
        else
            totalSize += fileInfo.size();
    }

    progressDialog->setLabelText(
        QString("Copied %1 of %2")
            .arg(formatBytes(copiedSize))
            .arg(formatBytes(totalSize))
        );

    progressDialog->show();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    progressDialog->setMaximum(100);
    progressDialog->setValue(0);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    foreach (const QString &file, source) {
        if (progressDialog->wasCanceled()) {
            return false;
        }

        QFileInfo fileInfo(file);

        // check if source directory is not destination parent
        if (targetPath.contains(file) && !targetPath.contains(file + " - Copy") && fileInfo.isDir()) {
            QMessageBox::warning(nullptr, "Error", "The source directory is the parent of the target directory!");
            return false;
        }

        // ask if file exists
        if (QFileInfo::exists(targetPath + fileInfo.fileName())) {
            if (!overwriteAll && !skipAll) {
                QMessageBox msgBox;
                msgBox.setWindowTitle("File exists");
                msgBox.setText("File \"" + targetPath + fileInfo.fileName() + "\" already exists! Do you want to overwrite it?");
                QPushButton *overwriteButton = msgBox.addButton("Overwrite", QMessageBox::YesRole);
                QPushButton *overwriteAllButton = msgBox.addButton("Overwrite All", QMessageBox::YesRole);
                QPushButton *skipButton = msgBox.addButton("Skip", QMessageBox::NoRole);
                QPushButton *skipAllButton = msgBox.addButton("Skip All", QMessageBox::NoRole);
                msgBox.exec();

                if (msgBox.clickedButton() == (QAbstractButton*)overwriteButton) {
                    QFile::remove(targetPath + fileInfo.fileName());
                } else if (msgBox.clickedButton() == (QAbstractButton*)overwriteAllButton) {
                    overwriteAll = true;
                    QFile::remove(targetPath + fileInfo.fileName());
                } else if (msgBox.clickedButton() == (QAbstractButton*)skipButton) {
                    continue;
                } else if (msgBox.clickedButton() == (QAbstractButton*)skipAllButton) {
                    skipAll = true;
                    continue;
                }
            } else if (overwriteAll) {
                QFile::remove(targetPath + fileInfo.fileName());
            } else if (skipAll) {
                continue;
            }
        }

        // Copying files
        if (fileInfo.isFile() || fileInfo.isSymLink()) {
            QFile sourceFile(file);
            QString newName = targetPath + fileInfo.fileName();

            QFile out(newName);

            if (!sourceFile.open(QIODevice::ReadOnly))
                return false;
            if (!out.open(QIODevice::WriteOnly))
                return false;

            const qint64 bufferSize = 4 * 1024 * 1024; // 4 MB buffer
            QByteArray buffer;
            buffer.reserve(bufferSize);

            QElapsedTimer updateTimer;
            updateTimer.start();

            while (!sourceFile.atEnd()) {
                buffer = sourceFile.read(bufferSize);
                if (buffer.isEmpty())
                    break;

                qint64 written = out.write(buffer);
                if (written < 0)
                    return false;

                copiedSize += written;

                // update buffer every 100 ms
                if (progressDialog && updateTimer.elapsed() > 100) {
                    progressDialog->setValue((copiedSize * 100) / totalSize);
                    progressDialog->setLabelText(
                        QString("Copied %1 of %2")
                            .arg(formatBytes(copiedSize))
                            .arg(formatBytes(totalSize))
                        );
                    QCoreApplication::processEvents();
                    updateTimer.restart();
                }
            }

            sourceFile.close();
            out.close();

        } else if (fileInfo.isDir()) {
            copyDirectory(file, targetPath + fileInfo.fileName(), *progressDialog, copiedSize, totalSize);
        }
    }

    progressDialog->setValue(100);
    progressDialog->close();

    return true;
}
/*bool MyFileSystemModel::copyFiles(QStringList source, QString targetPath)
{
    if (targetPath.at(targetPath.length() - 1) != '/')
        targetPath.append('/');

    QProgressDialog *progressDialog = new QProgressDialog("Copying files...", "Cancel", 0, source.size(), nullptr);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setWindowTitle("Copy Progress");
    progressDialog->setMinimumDuration(50);

    // create destination folder if not exists
    QDir destinationDir(targetPath);
    if (!destinationDir.exists()) {
        if (!destinationDir.mkdir(targetPath)) {
            QMessageBox::warning(nullptr, "Error", "Failed to create the destination folder.");
            return false;
        }
    }

    // disable copying to the same path
    if (targetPath == (source[0].left(source[0].lastIndexOf(QChar('/'))) + '/')) {
        QMessageBox::warning(nullptr, "Error", "Cannot copy file into the same folder.");
        return false;
    }

    qint64 totalSize = 0;
    qint64 copiedSize = 0;

    bool overwriteAll = false;
    bool skipAll = false;

    foreach (const QString &file, source) {
        QFileInfo fileInfo(file);
        if(fileInfo.isDir())
            totalSize += calculateDirectorySize(fileInfo);
        else
            totalSize += fileInfo.size();
    }

    progressDialog->setMaximum(totalSize);
    progressDialog->setValue(0);
    progressDialog->show();
    QCoreApplication::processEvents();

    foreach (const QString &file, source) {
        if (progressDialog->wasCanceled()) {
            return false;
        }

        QFileInfo fileInfo(file);

        // check if source directory is not destination parent
        if (targetPath.contains(file) && !targetPath.contains(file + " - Copy") && fileInfo.isDir()) {
            QMessageBox::warning(nullptr, "Error", "The source directory is the parent of the target directory!");
            return false;
        }

        // ask if file exists
        if (QFileInfo::exists(targetPath + fileInfo.fileName())) {
            if (!overwriteAll && !skipAll) {
                QMessageBox msgBox;
                msgBox.setWindowTitle("File exists");
                msgBox.setText("File \"" + targetPath + fileInfo.fileName() + "\" already exists! Do you want to overwrite it?");
                QPushButton *overwriteButton = msgBox.addButton("Overwrite", QMessageBox::YesRole);
                QPushButton *overwriteAllButton = msgBox.addButton("Overwrite All", QMessageBox::YesRole);
                QPushButton *skipButton = msgBox.addButton("Skip", QMessageBox::NoRole);
                QPushButton *skipAllButton = msgBox.addButton("Skip All", QMessageBox::NoRole);
                msgBox.exec();

                if (msgBox.clickedButton() == (QAbstractButton*)overwriteButton) {
                    QFile::remove(targetPath + fileInfo.fileName());
                } else if (msgBox.clickedButton() == (QAbstractButton*)overwriteAllButton) {
                    overwriteAll = true;
                    QFile::remove(targetPath + fileInfo.fileName());
                } else if (msgBox.clickedButton() == (QAbstractButton*)skipButton) {
                    continue;
                } else if (msgBox.clickedButton() == (QAbstractButton*)skipAllButton) {
                    skipAll = true;
                    continue;
                }
            } else if (overwriteAll) {
                QFile::remove(targetPath + fileInfo.fileName());
            } else if (skipAll) {
                continue;
            }
        }

        // Copying files
        if (fileInfo.isFile() || fileInfo.isSymLink()) {
            QFile::copy(file, targetPath + fileInfo.fileName());
            copiedSize += fileInfo.size();
            progressDialog->setValue(copiedSize);
            QCoreApplication::processEvents();

        } else if (fileInfo.isDir()) {
            copyDirectory(file, targetPath + fileInfo.fileName(), *progressDialog, copiedSize);
        }
    }

    progressDialog->setValue(totalSize);
    progressDialog->close();

    return true;
}*/

bool MyFileSystemModel::copyDirectory(const QString &sourceDirPath, const QString &targetDirPath, QProgressDialog &progress,
                                      qint64 &copiedSize, qint64 totalSize)
{
    QDir sourceDir(sourceDirPath);
    QDir targetDir(targetDirPath);

    // create destination folder if not exists
    if (!targetDir.exists()) {
        if (!QDir().mkdir(targetDirPath)) {
            QMessageBox::warning(nullptr, "Error", "Failed to create the target directory: " + targetDirPath);
            return false;
        }
    }

    QFileInfoList fileList = sourceDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Hidden | QDir::System);
    foreach (const QFileInfo &fileInfo, fileList) {
        QString targetFilePath = targetDirPath + "/" + fileInfo.fileName();
        if (fileInfo.isDir()) {
            copyDirectory(fileInfo.filePath(), targetFilePath, progress, copiedSize, totalSize);
        } else {
            /*QFile::copy(fileInfo.filePath(), targetFilePath);
            copiedSize += fileInfo.size();
            progress.setValue(copiedSize); */

            QFile sourceFile(fileInfo.filePath());
            QString newName = targetFilePath;

            QFile out(newName);

            if (!sourceFile.open(QIODevice::ReadOnly))
                return false;
            if (!out.open(QIODevice::WriteOnly))
                return false;

            const qint64 bufferSize = 4 * 1024 * 1024; // 4 MB buffer
            QByteArray buffer;
            buffer.reserve(bufferSize);

            QElapsedTimer updateTimer;
            updateTimer.start();

            while (!sourceFile.atEnd()) {
                buffer = sourceFile.read(bufferSize);
                if (buffer.isEmpty())
                    break;

                qint64 written = out.write(buffer);
                if (written < 0)
                    return false;

                copiedSize += written;

                // update progress every 100 ms
                if (updateTimer.elapsed() > 100) {
                    progress.setValue((copiedSize * 100) / totalSize);
                    progress.setLabelText(
                        QString("Copied %1 of %2")
                            .arg(formatBytes(copiedSize))
                            .arg(formatBytes(totalSize))
                        );
                    QCoreApplication::processEvents();
                    updateTimer.restart();
                }
            }

            sourceFile.close();
            out.close();
        }
    }

    return true;
}

// remove directory recursively with progress bar update
bool MyFileSystemModel::removeDirectory(const QString &dirPath, QProgressDialog *progressDialog, int &count, int total)
{
    QDir dir(dirPath);
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden | QDir::System);

    for (const QFileInfo &entry : entries) {
        if (progressDialog->wasCanceled())
            return false;

        if (entry.isDir()) {
            if (!removeDirectory(entry.absoluteFilePath(), progressDialog, count, total))
                return false;
        } else {
            QFile::remove(entry.absoluteFilePath());
            count++;
            progressDialog->setValue(count);
            progressDialog->setLabelText(
                QString("Removed %1 of %2")
                    .arg(count)
                    .arg(total)
                );
            QCoreApplication::processEvents();
        }
    }

    dir.rmdir(dirPath);
    count++;
    progressDialog->setValue(count);
    QCoreApplication::processEvents();

    return true;
}

// get directory files count recursively
int MyFileSystemModel::countEntriesInDirectory(const QString &dirPath)
{
    int total = 1; // the folder

    QDir dir(dirPath);
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden | QDir::System);

    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            total += countEntriesInDirectory(entry.absoluteFilePath());
        } else {
            total++; // file
        }
    }

    return total;
}


// Drag and Drop
Qt::DropActions MyFileSystemModel::supportedDropActions() const
{
    return Qt::CopyAction;
}

Qt::ItemFlags MyFileSystemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index) | Qt::ItemIsEditable;

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList MyFileSystemModel::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list";
    return types;
}

QMimeData *MyFileSystemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urlList;
    QStringList filePaths;
    foreach (const QModelIndex &index, indexes) {
        QFileInfo info = this->fileInfo(index);
        QString filePath = info.absoluteFilePath();

        if (index.data() != "..")
            filePaths.append(filePath);
    }
    filePaths.removeDuplicates();

    foreach (const QString &file, filePaths)
        urlList.append(QUrl::fromLocalFile(file));

    mimeData->setUrls(urlList);
    return mimeData;
}

bool MyFileSystemModel::canDropMimeData(const QMimeData *data,
                                        Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(parent);

    if (!data->hasFormat("text/uri-list") || column > 0)
        return false;

    QList<QUrl> urlList = data->urls();
    if(urlList.isEmpty())
        return false;

    return true;
}

bool MyFileSystemModel::dropMimeData(const QMimeData *data,
                                     Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    QList<QUrl> urlList = data->urls();
    QStringList filePaths;

    foreach (const QUrl &url, urlList) {
#ifdef Q_OS_WIN
        filePaths.append(url.path().remove(0, 1));
#else
        filePaths.append(url.path());
#endif
    }

    QString destPath;

    if (!parent.isValid()) {
        destPath = this->m_path;
    } else {
        destPath = this->path(parent);
        QFileInfo *destInfo = new QFileInfo(destPath);
        if (destInfo->isDir()) {
            if (parent.data().toString() == "..") {
                if (destPath.endsWith(".."))
                    destPath.chop(2);
                destPath = this->m_path;
            }

            if (destPath.at(destPath.length() - 1) != '/')
                destPath.append('/');
        } else {
            destPath = this->m_path;
        }
    }

    if (destPath == (filePaths[0].left(filePaths[0].lastIndexOf(QChar('/'))) + '/'))
        return false;

    QMessageBox msgCopyConfirm;
    msgCopyConfirm.setWindowTitle("Copy files");
    msgCopyConfirm.setText("Do you want to copy files (" + QString::number(filePaths.length()) + ")?");
    msgCopyConfirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgCopyConfirm.setDefaultButton(QMessageBox::Yes);
    msgCopyConfirm.setIcon(QMessageBox::Question);

    if (msgCopyConfirm.exec() == QMessageBox::Yes)
        copyFiles(filePaths, destPath);

    return true;
}

void MyFileSystemModel::setFilter(QDir::Filters newFilters)
{
    filters = newFilters;
    setRootPath(m_path);
}
QDir::Filters MyFileSystemModel::filter()
{
    return filters;
}


// Filesystem changes
void MyFileSystemModel::onDirectoryChanged(const QString &path)
{
    QDir dir(path);
    emit beforeReset();
    beginResetModel();
    fileList = dir.entryInfoList(filters);
    fileCount = fileList.count();;
    endResetModel();
    emit afterReset();

    if (!QDir(m_path).exists() && dir.exists()) {
        m_path = path;
        emit rootPathChanged(path, m_position);
    }
}

void MyFileSystemModel::onFileChanged(const QString &path)
{
    QModelIndex index = this->index(path);
    emit dataChanged(index, index);
}
