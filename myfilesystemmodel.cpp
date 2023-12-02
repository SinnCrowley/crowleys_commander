#include <QFileIconProvider>
#include <QtGui>
#include <QMessageBox>

#include "myfilesystemmodel.h"


MyFileSystemModel::MyFileSystemModel(QObject *parent) {
    Q_UNUSED(parent);

    setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files);
    setReadOnly(false);
}

// filesystem icons
QVariant MyFileSystemModel::data ( const QModelIndex & index, int role ) const {
    if(index.column() == 0 && role == Qt::DecorationRole ) {
        QFileInfo info = MyFileSystemModel::fileInfo(index);
        QFileIconProvider provider;
        QIcon icon = provider.icon(info);

        if(info.fileName() == "..")
            return QIcon(QPixmap(":/icons/icons/back.png"));

        return icon;
    }
    return QFileSystemModel::data(index, role);
}

// !!!!!!!!!!!!!!!!!!!!!!!
// починить работу копирования папок
// !!!!!!!!!!!!!!!!!!!!!!!

// file actions
void MyFileSystemModel::copyFiles(QStringList source, const QString targetPath) {
    for(const QString &file : source) {
        QFileInfo *fileInfo = new QFileInfo(file);

        if(QFileInfo::exists(targetPath + fileInfo->fileName())) {
            QMessageBox msgFileExists;
            msgFileExists.setIcon(QMessageBox::Question);
            msgFileExists.setWindowTitle("File exists");
            msgFileExists.setText("File \"" + targetPath + fileInfo->fileName() +
                                   "\" already exists! Do you want to overwrite the file?");
            QPushButton *overwriteButton = msgFileExists.addButton("Overwrite", QMessageBox::YesRole);
            QPushButton *overwriteAllButton = msgFileExists.addButton("Overwrite All", QMessageBox::YesRole);
            QPushButton *skipButton = msgFileExists.addButton("Skip", QMessageBox::NoRole);
            QPushButton *skipAllButton = msgFileExists.addButton("Skip All", QMessageBox::NoRole);
            msgFileExists.exec();

            if(msgFileExists.clickedButton() == (QAbstractButton*)overwriteButton) {
                QStringList filesList;
                filesList.append(file);
                copyFilesWithOverwriting(filesList, targetPath);
            }
            if(msgFileExists.clickedButton() == (QAbstractButton*)overwriteAllButton) {
                QStringList rest;
                for(QList<QString>::iterator it = source.begin() + source.indexOf(file); it != source.end(); ++it)
                    rest.append(*it);
                copyFilesWithOverwriting(rest, targetPath);
                break;
            }
            if(msgFileExists.clickedButton() == (QAbstractButton*)skipButton) {
                continue;
            }
            if(msgFileExists.clickedButton() == (QAbstractButton*)skipAllButton) {
                QStringList rest;
                for(QList<QString>::iterator it = source.begin() + source.indexOf(file); it != source.end(); ++it) {
                    if(!QFile::exists(*it))
                        rest.append(*it);
                }
                copyFiles(rest, targetPath);
                break;
            }
        }
        else {
            if(fileInfo->isFile())
                QFile::copy(file, targetPath + fileInfo->fileName());
            else {
                copyDirectory(file, targetPath + fileInfo->fileName());
            }
        }
    }
}

void MyFileSystemModel::copyFilesWithOverwriting(const QStringList source, const QString targetPath) {
    for (const QString &file : source) {
        QFileInfo fileInfo(file);
        QString destPath;
        if(targetPath[targetPath.length()-1] != '/')
            destPath = targetPath + '/' + fileInfo.fileName();
        else
            destPath = targetPath  + fileInfo.fileName();

        if (fileInfo.isDir()) {
            // if it is a dir, remove recursively and copy it
            if(QDir(destPath).exists())
                QDir(destPath).removeRecursively();
            copyDirectory(file, destPath);
        }
        else {
            // if it is a file, remove it before copying
            QFile::remove(destPath);
            if (!QFile::copy(file, destPath)) {
                QMessageBox msg;
                msg.setText("Error! Something went wrong.");
                msg.exec();
            }
        }
    }
}

void MyFileSystemModel::copyDirectory(const QString source, const QString targetPath) {
    /*QDirIterator it(source, QDirIterator::Subdirectories);
    QDir dir(source);
    const int absSourcePathLength = dir.absoluteFilePath(source).length();

    while (it.hasNext()){
        it.next();
        const auto fileInfo = it.fileInfo();
        if(!fileInfo.isHidden()) { //filters dot and dotdot
            const QString subPathStructure = fileInfo.absoluteFilePath().mid(absSourcePathLength);
            const QString constructedAbsolutePath = targetPath + subPathStructure;

            if(fileInfo.isDir()){
                //Create directory in target folder
                //dir.mkpath(constructedAbsolutePath);
                QDir(constructedAbsolutePath).mkpath(".");
            } else if(fileInfo.isFile()) {
                //Copy File to target directory
                //Remove file at target location, if it exists, or QFile::copy will fail
                QFile::remove(constructedAbsolutePath);
                QFile::copy(fileInfo.absoluteFilePath(), constructedAbsolutePath);
            }
        }
    }*/

    QDir sourceDir(source);
    QDir targetDir(targetPath);

    // create folder if not exists
    if (!targetDir.exists()) {
        targetDir.mkpath(".");
    }

    QFileInfoList entries = sourceDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QFileInfo &entryInfo, entries) {
        const QString srcPath = entryInfo.filePath();
        QString destPath;
        if(targetPath[targetPath.length()-1] != '/')
            destPath = targetPath + '/' + entryInfo.fileName();
        else
            destPath = targetPath  + entryInfo.fileName();

        if (entryInfo.isDir()) {
            copyDirectory(srcPath, destPath);
        }
        else {
            if (!QFile::copy(srcPath, destPath)) {
                QMessageBox msg;
                msg.setText("Error! Something went wrong.");
                msg.exec();
            }
        }
    }
}

// Drag and Drop

Qt::DropActions MyFileSystemModel::supportedDropActions() const {
    //return Qt::CopyAction | Qt::MoveAction;
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
    types << "application/vnd.text.list";
    return types;
}

QMimeData *MyFileSystemModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (index.isValid()) {
            stream << ((MyFileSystemModel*)index.model())->filePath(index);
        }
    }
    mimeData->setData("application/vnd.text.list", encodedData);
    return mimeData;
}

bool MyFileSystemModel::canDropMimeData(const QMimeData *data,
    Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(parent);

    if (!data->hasFormat("application/vnd.text.list"))
        return false;

    if (column > 0)
        return false;

    return true;
}

bool MyFileSystemModel::dropMimeData(const QMimeData *data,
    Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    if (action == Qt::IgnoreAction)
        return true;

    QByteArray encodedData = data->data("application/vnd.text.list");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList newItems;

    while (!stream.atEnd()) {
        QString text;
        stream >> text;
        newItems << text;
    }
    newItems.removeDuplicates();

    QString target = ((MyFileSystemModel*)parent.model())->filePath(parent);
    QFileInfo *targetInfo = new QFileInfo(target);

    if(targetInfo->isFile())
        target = targetInfo->absolutePath();

    if(target[target.length()-1] != '/')
        target += '/';

    if(target == (newItems[0].left(newItems[0].lastIndexOf(QChar('/')))+'/'))
        return false;



    QMessageBox msgCopyConfirm;
    msgCopyConfirm.setWindowTitle("Copy files");
    msgCopyConfirm.setText("Do you want to copy files (" + QString::number(newItems.length()) + ")?");
    msgCopyConfirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgCopyConfirm.setDefaultButton(QMessageBox::Yes);
    msgCopyConfirm.setIcon(QMessageBox::Question);

    if(msgCopyConfirm.exec() == QMessageBox::Yes)
        copyFiles(newItems, target);

    return true;
}

void MyFileSystemModel::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/vnd.text.list"))
        event->acceptProposedAction();
}
void MyFileSystemModel::dropEvent(QDropEvent *event) {
    event->acceptProposedAction();
}

