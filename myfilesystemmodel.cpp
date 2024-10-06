#include <QFileIconProvider>
#include <QtGui>
#include <QMessageBox>
#include <QApplication>

#include <QProgressDialog>

#include "myfilesystemmodel.h"

MyFileSystemModel::MyFileSystemModel(QObject *parent) {
    Q_UNUSED(parent);

    setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files | QDir::System);
    setReadOnly(false);
}

QVariant MyFileSystemModel::data (const QModelIndex &index, int role) const {
    if(index.column() == 0 && role == Qt::DecorationRole ) {
        QFileInfo info = MyFileSystemModel::fileInfo(index);
        QFileIconProvider provider;
        QIcon icon = provider.icon(info);

        if(info.fileName() == "..")
            return QIcon(QPixmap(":/icons/icons/back.png"));

        return icon;
    }

    if(role == Qt::DisplayRole) {
        QFileInfo info = MyFileSystemModel::fileInfo(index);

        // files and folders that start with dot (.gitignore, .atom)
        if(info.completeBaseName() == "") {
            if(index.column() == 0)
                return "." + info.suffix();
            if(index.column() == 2) {
                if(info.isDir())
                    return "Folder";
                else
                    return "";
            }
        }

        if(index.column() == 0) {
            if(info.isDir() && !info.isSymLink())
                return info.fileName();
            else
                return info.completeBaseName();
        }

        if(index.column() == 2) {
            if(info.isDir() && !info.isSymLink())
                return QString("Folder");
            else
                return info.suffix().toLower();
        }
    }

    if(role == Qt::EditRole) {
        QFileInfo info = MyFileSystemModel::fileInfo(index);

        if(index.column() == 0)
            if(info.isSymLink())
                return info.fileName();
    }
    return QFileSystemModel::data(index, role);
}

bool MyFileSystemModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole && index.column() == 0) {
        QFileInfo info = fileInfo(index);
        QString oldFilePath = info.filePath();
        QString newFilePath = info.absolutePath() + "/" + value.toString();

        if (info.isSymLink() || info.isDir() || info.isFile()) {
            if(newFilePath == oldFilePath)
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
    return QFileSystemModel::setData(index, value, role);
}


// file actions
/*void MyFileSystemModel::updateProgress(QStringList fileList, QProgressDialog &dialog) {
    qint64 filesSize = 0;
    for (const QString &file : fileList) {
        QFileInfo fileInfo(file);
        if(fileInfo.exists()) {
            if (fileInfo.isFile() || fileInfo.isSymLink()) {
                filesSize += fileInfo.size();
            } else if (fileInfo.isDir()) {
                filesSize += calculateDirectorySize(fileInfo);
            }
        }
    }
    dialog.setValue(filesSize);
}

qint64 MyFileSystemModel::calculateDirectorySize(const QFileInfo& dirInfo) {
    qint64 totalSize = 0;
    QDir dir(dirInfo.filePath());
    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::AllDirs);

    for (const QFileInfo& fileInfo : fileList) {
        if (fileInfo.isDir()) {
            totalSize += calculateDirectorySize(fileInfo);
        } else {
            totalSize += fileInfo.size();
        }
    }

    return totalSize;
}
*/

bool MyFileSystemModel::copyFiles(QStringList source, QString targetPath) {

    if (targetPath.at(targetPath.length() - 1) != '/')
        targetPath.append('/');

    // Создаем диалоговое окно для отслеживания прогресса копирования
    QProgressDialog progressDialog("Copying files...", "Cancel", 0, source.size());
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(500);  // Появится только если копирование занимает больше 500 мс
    progressDialog.setWindowTitle("Copy Progress");

    // Создание директории назначения
    QDir destinationDir(targetPath);
    if (!destinationDir.exists()) {
        if (!destinationDir.mkdir(targetPath)) {
            QMessageBox::warning(nullptr, "Error", "Failed to create the destination folder.");
            return false;
        }
    }

    bool overwriteAll = false;
    bool skipAll = false;

    for (int i = 0; i < source.size(); ++i) {
        // Обновляем прогресс
        progressDialog.setValue(i);

        // Проверка на отмену пользователем
        if (progressDialog.wasCanceled()) {
            QMessageBox::warning(nullptr, "Operation Cancelled", "File copying operation was cancelled.");
            return false;
        }

        // Обработка событий интерфейса, чтобы не блокировать окно
        QCoreApplication::processEvents();

        QFileInfo fileInfo(source[i]);

        // Проверка, является ли папка-источник родителем папки-назначения
        if (targetPath.contains(source[i]) && fileInfo.isDir()) {
            QMessageBox::warning(nullptr, "Error", "The source directory is the parent of the target directory!");
            return false;
        }

        // Если файл уже существует, спрашиваем, что делать
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
                    // Перезаписываем только этот файл
                    QFile::remove(targetPath + fileInfo.fileName());
                } else if (msgBox.clickedButton() == (QAbstractButton*)overwriteAllButton) {
                    overwriteAll = true;
                    QFile::remove(targetPath + fileInfo.fileName());
                } else if (msgBox.clickedButton() == (QAbstractButton*)skipButton) {
                    // Пропускаем только этот файл
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

        // Копирование файла или директории
        if (fileInfo.isFile() || fileInfo.isSymLink()) {
            QFile::copy(source[i], targetPath + fileInfo.fileName());
        } else if (fileInfo.isDir()) {
            copyDirectory(source[i], targetPath + fileInfo.fileName());
        }
    }

    // Завершаем прогресс
    progressDialog.setValue(source.size());

    return true;
}

bool MyFileSystemModel::copyDirectory(const QString &sourceDirPath, const QString &targetDirPath) {
    QDir sourceDir(sourceDirPath);
    QDir targetDir(targetDirPath);

    // Создаем папку назначения, если она не существует
    if (!targetDir.exists()) {
        if (!QDir().mkdir(targetDirPath)) {
            QMessageBox::warning(nullptr, "Error", "Failed to create the target directory: " + targetDirPath);
            return false;
        }
    }

    // Копируем все файлы из исходной папки
    QFileInfoList fileList = sourceDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Hidden);
    for (const QFileInfo &fileInfo : fileList) {
        QString targetFilePath = targetDirPath + "/" + fileInfo.fileName();
        if (fileInfo.isDir()) {
            // Рекурсивно копируем вложенные папки
            copyDirectory(fileInfo.filePath(), targetFilePath);
        } else {
            // Копируем файл
            QFile::copy(fileInfo.filePath(), targetFilePath);
        }
    }

    return true;
}

// Drag and Drop
Qt::DropActions MyFileSystemModel::supportedDropActions() const {
    return Qt::CopyAction;
}

Qt::ItemFlags MyFileSystemModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QFileSystemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList MyFileSystemModel::mimeTypes() const {
    QStringList types;
    types << "text/uri-list";
    return types;
}

QMimeData *MyFileSystemModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urlList;
    QStringList filePaths;
    for(const QModelIndex &index : indexes) {
        QFileInfo info = ((MyFileSystemModel*)index.model())->fileInfo(index);
        QString filePath = info.absoluteFilePath();

        if(index.data() != "..")
            filePaths.append(filePath);
    }
    filePaths.removeDuplicates();

    for(QString &file : filePaths) {
        urlList.append(QUrl::fromLocalFile(file));
    }

    mimeData->setUrls(urlList);
    return mimeData;
}

bool MyFileSystemModel::canDropMimeData(const QMimeData *data,
    Qt::DropAction action, int row, int column, const QModelIndex &parent) const {
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
    Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    if (action == Qt::IgnoreAction)
        return true;

    QList<QUrl> urlList = data->urls();
    QStringList filePaths;

    for(const QUrl &url : urlList)
        filePaths.append(url.path().remove(0, 1));

    QString destPath = ((MyFileSystemModel*)parent.model())->filePath(parent);
    QFileInfo *destInfo = new QFileInfo(destPath);

    if(destInfo->isDir()) {
        if(parent.data().toString() == "..")
            destPath.chop(2);

        if(destPath.at(destPath.length() - 1) != '/')
            destPath.append('/');
    }
    else
        destPath = destInfo->absolutePath() + "/";

    if(destPath == (filePaths[0].left(filePaths[0].lastIndexOf(QChar('/')))+'/'))
        return false;


    QMessageBox msgCopyConfirm;
    msgCopyConfirm.setWindowTitle("Copy files");
    msgCopyConfirm.setText("Do you want to copy files (" + QString::number(filePaths.length()) + ")?");
    msgCopyConfirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgCopyConfirm.setDefaultButton(QMessageBox::Yes);
    msgCopyConfirm.setIcon(QMessageBox::Question);

    if(msgCopyConfirm.exec() == QMessageBox::Yes)
        copyFiles(filePaths, destPath);

    return true;
}

void MyFileSystemModel::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}
void MyFileSystemModel::dropEvent(QDropEvent *event) {
    event->acceptProposedAction();
}


