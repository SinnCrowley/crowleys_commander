/*  TODO
 *
 *  ускорить загрузку больших папок (+ возможно подгружать внутренности папок в фоне)
 *
 *
 *  FUTURE
 *
 *  open with on linux
 *
 *  fix progress bars
 *
 *  on device connection add only new device (do not reload all panel)
 *
 *  on file cutting set the icons a bit transparent (?)
 *
 *  add pack/unpack, settings, group rename, copy/delete/move in background
 *
 *  add to menu: copy path, properties, share
 *
 *  add function in settings select or not select file extension while rename
 *
 *  add full UI customization in settings (colors, fonts, sizes)
*/

#include <QtGlobal>
#include <QDebug>
#ifdef Q_OS_WIN
#include "windows.h"
#include "shobjidl.h"
#include "objbase.h"
#endif

#ifdef Q_OS_LINUX
#include <libudev.h>
#endif

#include <QHeaderView>
#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QShortcut>
#include <QSettings>
#include <QDesktopServices>
#include <QClipboard>
#include <QStack>
#include <QtConcurrent/QtConcurrent>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mysearchdialog.h"

#include "myfilesystemmodel.h"
#include "mytreeview.h"
#include "mysortfilterproxymodel.h"


#ifdef Q_OS_WIN
bool updateShortcuts(QString path) {
    CoInitialize(NULL);
    IShellLinkW* psl = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&psl);

    if (SUCCEEDED(hr)) {
        IPersistFile* ppf = nullptr;
        hr = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

        if (SUCCEEDED(hr)) {
            hr = ppf->Load(QFileInfo(path).absoluteFilePath().toStdWString().c_str(), STGM_READWRITE);
            if (SUCCEEDED(hr)) {
                // Пытаемся найти новую цель ярлыка
                hr = psl->Resolve(NULL, SLR_UPDATE | SLR_NO_UI);
                if (SUCCEEDED(hr)) {
                    // Получаем обновленный путь
                    WIN32_FIND_DATAW wfd;
                    WCHAR szNewPath[MAX_PATH];
                    hr = psl->GetPath(szNewPath, MAX_PATH, &wfd, SLGP_UNCPRIORITY);

                    if (SUCCEEDED(hr)) {
                        // Сохраняем обновленный путь в ярлыке
                        hr = ppf->Save(NULL, TRUE);
                        ppf->Release();
                        psl->Release();
                        CoUninitialize();
                        return true;
                    }
                }
            }
            ppf->Release();
        }
        psl->Release();
    }
    CoUninitialize();
    return false;
}
#endif

// create MyTreeView with Filesystem
void MainWindow::createView(QTabWidget *tabBar, QString path) {
    QString folderName;
    path = QDir(path).absolutePath() + "/";
    if(path == "//") {
        path.chop(1);
        folderName = "/";
    }
    else {
        QDir *dirInfo = new QDir(path);
        if(dirInfo->isRoot())
            folderName = path.left(path.indexOf(":") + 1);
        else
            folderName = dirInfo->dirName();
    }

    auto index = tabBar->addTab(new QLabel(folderName), folderName);
    tabBar->setCurrentIndex(index);

    QVBoxLayout *barLayout = new QVBoxLayout(tabBar->currentWidget());
    barLayout->setSpacing(0);
    barLayout->setContentsMargins(0, 0, 0, 0);

    MyTreeView *view = new MyTreeView(path, this);
    QLineEdit *pathEdit = new QLineEdit(this);
    if(QDir(path).isRoot())
        pathEdit->setText(QDir::toNativeSeparators(QDir(path).absolutePath()));
    else
        pathEdit->setText(QDir::toNativeSeparators(QDir(path).absolutePath() + "/"));
    pathEdit->setFocusPolicy(Qt::ClickFocus);

    barLayout->addWidget(pathEdit);
    barLayout->addWidget(view);

    pathEdit->setObjectName(tabBar->objectName().left(tabBar->objectName().indexOf("Bar")));
    view->setObjectName(tabBar->objectName().left(tabBar->objectName().indexOf("Bar")));

    connect(view, SIGNAL(activated(QModelIndex)), this, SLOT(view_activated(QModelIndex)));
    connect(view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_requested(QPoint)));
    connect(pathEdit, SIGNAL(returnPressed()), this, SLOT(pathEdit_returnPressed()));
    //connect(view->header(), SIGNAL(sectionClicked(int)), this, SLOT(viewHeader_clicked(int)));

    diskStatusUpdate(tabBar);

    // create history stacks
    QSettings *settings = new QSettings("./history.ini", QSettings::IniFormat);
    if(view->objectName() == "left") {
        settings->beginGroup("left_panel");
        historyBackLeft.append(QStack<QString>());
        historyForwardLeft.append(QStack<QString>());
    }
    else {
        settings->beginGroup("right_panel");
        historyBackRight.append(QStack<QString>());
        historyForwardRight.append(QStack<QString>());
    }

    int sortingColumn = settings->value("sort_order_column").toInt();

    if(settings->value("sort_direction").toString() == "asc")
        view->sortByColumn(sortingColumn, Qt::AscendingOrder);
    else
        view->sortByColumn(sortingColumn, Qt::DescendingOrder);

    delete settings;


#ifdef Q_OS_WIN
    // check shortcuts and repair if is crashed with Windows API
    QDir *dirInfo = new QDir(path);
    QFileInfoList fileList = dirInfo->entryInfoList(QDir::Files | QDir::System);
    foreach (const QFileInfo &fileInfo, fileList) {
        if(fileInfo.isSymLink() && !QFile::exists(fileInfo.symLinkTarget()))
            updateShortcuts(fileInfo.absoluteFilePath());
    }
#endif
    view->setFocus();
}

// change directory and reload panel
void MainWindow::directoryChange(QString path) {
    QString folderName;
    QString previousPath = path;
    path = QDir(path).absolutePath() + "/";
    if(path == "//") {
        path.chop(1);
        folderName = "/";
    }
    else {
        QDir *dirInfo = new QDir(path);
        if(dirInfo->isRoot())
            folderName = path.left(path.indexOf(":") + 1);
        else
            folderName = dirInfo->dirName();
    }

    QTabWidget *tabWidget;

    if(qApp->focusWidget()->objectName() == "left" || qApp->focusWidget()->objectName() == "diskListLeft")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    MyTreeView *view = tabWidget->currentWidget()->findChild<MyTreeView*>();
    QLineEdit *pathEdit = tabWidget->currentWidget()->findChild<QLineEdit*>();

    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    if(QDir(path).isRoot())
        fsModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files | QDir::System);
    else {
        if(fsModel->filter().testFlag(QDir::NoDotAndDotDot))
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files | QDir::System);
    }

    if(!isNavTriggered) {
        addToHistory(fsModel->rootPath(), tabWidget->currentIndex(), qApp->focusWidget()->objectName());
    }
    isNavTriggered = false;

    fsModel->setRootPath(path);
    view->setRootIndex(sortModel->mapFromSource(fsModel->index(path)));

    qApp->processEvents();

    tabWidget->setTabText(tabWidget->currentIndex(), folderName);

    if(QDir(path).isRoot())
        pathEdit->setText(QDir::toNativeSeparators(QDir(path).absolutePath()));
    else
        pathEdit->setText(QDir::toNativeSeparators(QDir(path).absolutePath() + "/"));

    view->setFocus();
    view->clearSelection();

    tabsUpdate(tabWidget);
    diskStatusUpdate(tabWidget);

    if(previousPath.back() == '/' && previousPath != "/")
        previousPath.chop(1);

    bool isUp = QFileInfo(previousPath).fileName() == "..";

    if(isUp) {
        // set cursor on the previous folder
        previousPath.chop(2);
        connect(sortModel, &QAbstractItemModel::layoutChanged, this, [view, sortModel, fsModel, previousPath]() {
            view->selectionModel()->setCurrentIndex(sortModel->mapFromSource(fsModel->index(previousPath)), QItemSelectionModel::NoUpdate);
            view->scrollToFile();
        });
    }
    else {
        // set cursor on the first row
        connect(sortModel, &QAbstractItemModel::layoutChanged, this, [view, sortModel, fsModel, path]() {
            QModelIndex rootIndex = sortModel->mapFromSource(fsModel->index(path));
            if (rootIndex.isValid() && view->model()->rowCount(rootIndex) > 0) {
                QModelIndex firstChildIndex = view->model()->index(0, 0, rootIndex);
                if (firstChildIndex.isValid()) {
                    view->selectionModel()->setCurrentIndex(firstChildIndex, QItemSelectionModel::NoUpdate);
                }
            }
        });
    }

#ifdef Q_OS_WIN
    // check shortcuts and repair if is crashed with Windows API
    QDir *dirInfo = new QDir(path);
    QFileInfoList fileList = dirInfo->entryInfoList(QDir::Files | QDir::System);
    foreach (const QFileInfo &fileInfo, fileList) {
        if(fileInfo.isSymLink() && !QFile::exists(fileInfo.symLinkTarget()))
            updateShortcuts(fileInfo.absoluteFilePath());
    }
#endif
}

// update tabs info in history.ini
void MainWindow::tabsUpdate(QTabWidget *tabWidget) {
    QSettings *settings = new QSettings("./history.ini", QSettings::IniFormat);
    if(tabWidget->objectName() == "leftBar")
        settings->beginGroup("left_tabs");
    else
        settings->beginGroup("right_tabs");

    settings->setValue("active_tab", tabWidget->currentIndex());

    for(QString &key : settings->allKeys()) {
        if(QChar(key.at(0)).isDigit())
            settings->remove(key);
    }
    for(int i = 0; i < tabWidget->count(); i++) {
        QLineEdit *pathEdit = tabWidget->widget(i)->findChild<QLineEdit*>();
        QString path = pathEdit->text();
        settings->setValue(QString::number(i) + "_path", path);
    }
    settings->endGroup();
    delete settings;
}

// update disk status bar
void MainWindow::diskStatusUpdate(QTabWidget *tabWidget) {
    if(!tabWidget->currentWidget()->findChild<MyTreeView*>())
        return;

    MyTreeView *view = tabWidget->currentWidget()->findChild<MyTreeView*>();
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;
    QString path = fsModel->rootPath();
    QStorageInfo storageInfo = QStorageInfo(path);

    QString storageSize = QString("Free space: %1 Gb / ")
                              .arg(QString::number(storageInfo.bytesFree() / 1024.00 / 1024 / 1024, 'f', 2))
                          + QString("%1 Gb").arg(QString::number(storageInfo.bytesTotal() / 1024.00 / 1024 / 1024, 'f', 2));

    if(tabWidget->objectName() == "leftBar") {
        ui->diskListLeft->setCurrentIndex(ui->diskListLeft->findText(path.at(0)));
        ui->diskNameLeft->setText(storageInfo.name());
        ui->diskSizeLeft->setText(storageSize);
    }
    else {
        ui->diskListRight->setCurrentIndex(ui->diskListLeft->findText(path.at(0)));
        ui->diskNameRight->setText(storageInfo.name());
        ui->diskSizeRight->setText(storageSize);
    }
}

// check if filename is valid
bool MainWindow::isValidFileName(QString filename) {
    // check common invalid characters
    const QString invalidChars = "\\/:*?\"<>|";
    if (filename.contains(QRegularExpression("[" + QRegularExpression::escape(invalidChars) + "]")))
        return false;

    // check maximum filename length
    if (filename.length() > 255)
        return false;

    // check if filename is empty
    if(filename == "")
        return false;

#ifdef Q_OS_WIN
    // check invalid names for Windows
    QStringList reservedNames = {"CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"};
    if (reservedNames.contains(filename, Qt::CaseInsensitive))
        return false;

#endif

    return true;
}

// remove all widgets from layout
void MainWindow::clearLayout(QLayout *layout) {
    if (layout == nullptr) {
        return;
    }

    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        if (item->layout()) {
            clearLayout(item->layout());
        }
        delete item;
    }
}

// get the list of selected files for file operations
QStringList MainWindow::getFileList() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return QStringList();

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    QStringList filePaths;
    QModelIndexList selected = view->selectionModel()->selectedIndexes();

    if(selected.isEmpty()) {
        QModelIndex fsModelIndex = sortModel->mapToSource(view->selectionModel()->currentIndex());
        QFileInfo info = fsModel->fileInfo(fsModelIndex);

        if(info.fileName() == "..")
            return QStringList();
        filePaths.append(info.absoluteFilePath());
    }
    else {
        foreach(const QModelIndex &index, selected) {
            QModelIndex fsModelIndex = sortModel->mapToSource(index);
            QFileInfo info = fsModel->fileInfo(fsModelIndex);

            if(info.fileName() != "..")
                filePaths.append(info.absoluteFilePath());
        }
        filePaths.removeDuplicates();
    }

    return filePaths;
}

// update list of disks on device connect/disconnect
void MainWindow::deviceUpdate(const QString &device) {
    Q_UNUSED(device);

    clearLayout(ui->diskButtonsLeftLayout);
    clearLayout(ui->diskButtonsRightLayout);

    QIcon hddIcon(QPixmap(":/icons/icons/hdd_96.png"));
    QIcon usbIcon(QPixmap(":/icons/icons/usb_96.png"));
    QIcon cdIcon(QPixmap(":/icons/icons/cdrom_96.png"));
    QIcon networkIcon(QPixmap(":/icons/icons/network_96.png"));

    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
        if (storage.isValid() && storage.isReady() && storage.name() != "") {
            QString fsType;
            if(storage.fileSystemType() == "fuseblk" || storage.fileSystemType() == "ntfs-3g")
                fsType = "ntfs";
            if(storage.fileSystemType() == "iso9660")
                fsType = "fat";

            QString diskText;

            if(storage.rootPath() == "/") {
                diskText = "/";
            }
            else {
                if(storage.device().contains("/dev"))
                    diskText = storage.device().mid(5, storage.device().length()-1);
                else
                    diskText = storage.rootPath().at(0);
            }

            QPushButton *leftButton = new QPushButton(this);
            QPushButton *rightButton = new QPushButton(this);

            leftButton->setToolTip(fsType);
            rightButton->setToolTip(fsType);

            leftButton->setObjectName("left");
            rightButton->setObjectName("right");

            leftButton->setText(diskText);
            rightButton->setText(diskText);

            ui->diskButtonsLeftLayout->addWidget(leftButton);
            ui->diskButtonsRightLayout->addWidget(rightButton);

            connect(leftButton, SIGNAL(clicked()), this, SLOT(diskButton_clicked()));
            connect(rightButton, SIGNAL(clicked()), this, SLOT(diskButton_clicked()));

            // 1: usb, 2: network, 3: cd, 4: hdd/ssd
            int diskType = 0;

            // get drive type
#ifdef Q_OS_WIN
            if(GetDriveType((const wchar_t *)(storage.rootPath()).utf16()) == 2)
                diskType = 1;
            else if(GetDriveType((const wchar_t *)(storage.rootPath()).utf16()) == 4)
                diskType = 2;
            else if(GetDriveType((const wchar_t *)(storage.rootPath()).utf16()) == 5)
                diskType = 3;
            else
                diskType = 4;
#else
            struct udev *udev = udev_new();
            if (!udev) {
                continue;
            }

            struct udev_enumerate *enumerate = udev_enumerate_new(udev);
            if (!enumerate) {
                udev_unref(udev);
                continue;
            }

            udev_enumerate_add_match_subsystem(enumerate, "block");
            udev_enumerate_scan_devices(enumerate);
            struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
            struct udev_list_entry *dev_list_entry;

            udev_list_entry_foreach(dev_list_entry, devices) {
                const char *path = udev_list_entry_get_name(dev_list_entry);
                struct udev_device *dev = udev_device_new_from_syspath(udev, path);
                if (!dev) {
                    continue;
                }

                const char *devNode = udev_device_get_devnode(dev);
                if (devNode && QString(devNode) == storage.device()) {
                    const char *devtype = udev_device_get_devtype(dev);
                    const char *subsystem = udev_device_get_subsystem(dev);

                    if (subsystem && QString(subsystem) == "block") {
                        struct udev_device *parent = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
                        if (parent) {
                            diskType = 1; // USB
                        } else if (devtype && QString(devtype) == "cd") {
                            diskType = 3; // CD
                        } else {
                            diskType = 4; // HDD/SSD (default)
                        }
                    }
                    udev_device_unref(dev);
                    break;
                }
                udev_device_unref(dev);
            }
            udev_enumerate_unref(enumerate);
            udev_unref(udev);
#endif

            switch(diskType) {
                case 1: // usb
                    leftButton->setIcon(usbIcon);
                    rightButton->setIcon(usbIcon);
                    ui->diskListLeft->addItem(usbIcon, diskText);
                    ui->diskListRight->addItem(usbIcon, diskText);
                    break;
                case 2: // network
                    leftButton->setIcon(networkIcon);
                    rightButton->setIcon(networkIcon);
                    ui->diskListLeft->addItem(networkIcon, diskText);
                    ui->diskListRight->addItem(networkIcon, diskText);
                    break;
                case 3: // cd
                    leftButton->setIcon(cdIcon);
                    rightButton->setIcon(cdIcon);
                    ui->diskListLeft->addItem(cdIcon, diskText);
                    ui->diskListRight->addItem(cdIcon, diskText);
                    break;
                default: // hdd/ssd (default)
                    leftButton->setIcon(hddIcon);
                    rightButton->setIcon(hddIcon);
                    ui->diskListLeft->addItem(hddIcon, diskText);
                    ui->diskListRight->addItem(hddIcon, diskText);
                    break;
            }

            leftButton->setFocusPolicy(Qt::ClickFocus);
            rightButton->setFocusPolicy(Qt::ClickFocus);
        }
    }
    QSpacerItem *spacerLeft = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem *spacerRight = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    ui->diskButtonsLeftLayout->addItem(spacerLeft);
    ui->diskButtonsRightLayout->addItem(spacerRight);

    // correct width of disk list view
    int maxWidth = 0;
    QFontMetrics metrics(ui->diskListLeft->font());
    for (int i = 0; i < ui->diskListLeft->count(); ++i) {
        int textWidth = metrics.horizontalAdvance(ui->diskListLeft->itemText(i));
        if (textWidth > maxWidth)
            maxWidth = textWidth;
    }

    // add some space for icons and paddings
    maxWidth += 52;

    ui->diskListLeft->view()->setMinimumWidth(maxWidth);
    ui->diskListRight->view()->setMinimumWidth(maxWidth);
}

// create configs on first run
void MainWindow::initialize() {
    QFile history("./history.ini");
    if(!history.exists()) {
        history.open(QIODevice::Append);
        QString historyData;
        QString fileName;
#ifdef Q_OS_WIN
        fileName = "templates/history_windows.ini";
#else
        fileName = "templates/history_linux.ini";
#endif
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream stream(&file);
            while (!stream.atEnd()){
                historyData = stream.readAll();
            }
        }
        file.close();
        history.write(historyData.toStdString().c_str());
        history.close();
    }

    QFile config("./config.ini");
    QString configData;
    if(!config.exists()) {
        config.open(QIODevice::Append);
        QFile file("templates/config.ini");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream stream(&file);
            while (!stream.atEnd()){
                configData = stream.readAll();
            }
        }
        config.write(configData.toStdString().c_str());
        config.close();
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , deviceWatcher(new DeviceWatcher(this)) {
    ui->setupUi(this);

    // event filer for back/forward buttons click processing
    qApp->installEventFilter(this);

    initialize();

    connect(deviceWatcher, &DeviceWatcher::deviceConnected, this, &MainWindow::deviceUpdate);
    connect(deviceWatcher, &DeviceWatcher::deviceDisconnected, this, &MainWindow::deviceUpdate);

    QPushButton *leftUpButton = new QPushButton(this);
    QPushButton *rightUpButton = new QPushButton(this);
    leftUpButton->setObjectName("left");
    leftUpButton->setIcon(QPixmap(":/icons/icons/up.png"));
    leftUpButton->setMaximumSize(36, 36);
    leftUpButton->setFocusPolicy(Qt::NoFocus);

    rightUpButton->setObjectName("right");
    rightUpButton->setIcon(QPixmap(":/icons/icons/up.png"));
    rightUpButton->setMaximumSize(36, 36);
    rightUpButton->setFocusPolicy(Qt::NoFocus);

    ui->statusDiskLeftLayout->addWidget(leftUpButton);
    ui->statusDiskRightLayout->addWidget(rightUpButton);

    connect(leftUpButton, SIGNAL(clicked()), this, SLOT(upBtn_clicked()));
    connect(rightUpButton, SIGNAL(clicked()), this, SLOT(upBtn_clicked()));


    QTabWidget *leftBar = new QTabWidget(this);
    leftBar->setObjectName("leftBar");
    leftBar->setMovable(true);
    ui->leftPanelLayout->addWidget(leftBar);

    connect(leftBar, SIGNAL(tabBarDoubleClicked(int)), this, SLOT(tabBar_doubleClicked(int)));
    connect(leftBar, SIGNAL(currentChanged(int)), this, SLOT(tabBar_indexChanged(int)));

    QSettings *settings = new QSettings("./history.ini", QSettings::IniFormat);
    settings->beginGroup("left_tabs");
    QStringList keys = settings->childKeys();
    int idx = settings->value("active_tab").toInt();
    foreach(const QString key, keys) {
        if(key[0].isDigit())
            createView(leftBar, settings->value(key).toString());
    }
    leftBar->setCurrentIndex(idx);
    settings->endGroup();

    diskStatusUpdate(leftBar);

    QTabWidget *rightBar = new QTabWidget(this);
    rightBar->setObjectName("rightBar");
    rightBar->setMovable(true);
    ui->rightPanelLayout->addWidget(rightBar);

    connect(rightBar, SIGNAL(tabBarDoubleClicked(int)), this, SLOT(tabBar_doubleClicked(int)));
    connect(rightBar, SIGNAL(currentChanged(int)), this, SLOT(tabBar_indexChanged(int)));

    settings->beginGroup("right_tabs");
    keys.clear();
    keys = settings->childKeys();
    idx = settings->value("active_tab").toInt();
    foreach(const QString key, keys) {
        if(key[0].isDigit())
            createView(rightBar, settings->value(key).toString());
    }
    rightBar->setCurrentIndex(idx);
    settings->endGroup();


    settings = new QSettings("./config.ini", QSettings::IniFormat);
    settings->beginGroup("config");

    MyTreeView *view;
    MySortFilterProxyModel *sortModel;
    MyFileSystemModel *fsModel;

    if(settings->value("show_hidden").toInt()) {
        for(int i = 0; i < leftBar->count(); i++) {
            view = leftBar->widget(i)->findChild<MyTreeView*>();
            sortModel = view->sortModel;
            fsModel = sortModel->fsModel;
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files | QDir::Hidden);
        }
        for(int i = 0; i < rightBar->count(); i++) {
            view = rightBar->widget(i)->findChild<MyTreeView*>();
            sortModel = view->sortModel;
            fsModel = sortModel->fsModel;
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files | QDir::Hidden);
        }
    }

    settings->endGroup();

    diskStatusUpdate(rightBar);

    connect(ui->diskListLeft, SIGNAL(textActivated(QString)), this, SLOT(diskList_textActivated(QString)));
    connect(ui->diskListRight, SIGNAL(textActivated(QString)), this, SLOT(diskList_textActivated(QString)));

    leftBar->setFocusPolicy(Qt::NoFocus);
    rightBar->setFocusPolicy(Qt::NoFocus);

    deviceUpdate("");

    QShortcut *editShortcut = new QShortcut(QKeySequence(Qt::Key_F4), this);
    connect(editShortcut, SIGNAL(activated()), this, SLOT(on_editBtn_clicked()));

    QShortcut *copyShortcut = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(copyShortcut, SIGNAL(activated()), this, SLOT(on_copyBtn_clicked()));

    QShortcut *moveShortcut = new QShortcut(QKeySequence(Qt::Key_F6), this);
    connect(moveShortcut, SIGNAL(activated()), this, SLOT(on_moveBtn_clicked()));

    QShortcut *folderShortcut = new QShortcut(QKeySequence(Qt::Key_F7), this);
    connect(folderShortcut, SIGNAL(activated()), this, SLOT(on_folderBtn_clicked()));

    QShortcut *deleteShortcut = new QShortcut(QKeySequence(Qt::Key_F8), this);
    connect(deleteShortcut, SIGNAL(activated()), this, SLOT(on_deleteBtn_clicked()));
}

MainWindow::~MainWindow() {
    disconnect();

    delete deviceWatcher;
    delete ui;
}


// actions
void MainWindow::diskList_textActivated(const QString &text) {
#ifdef Q_OS_WIN
    directoryChange(text + ":/");
#else
    QString disk;
    if(text != "/") {
        disk = "/dev/" + text;
        foreach(const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
            if(storage.device() == disk)
                directoryChange(storage.rootPath());
        }
    }
    else
        directoryChange(text);

#endif
}

void MainWindow::diskButton_clicked() {
    QPushButton *button = (QPushButton*) sender();
#ifdef Q_OS_WIN
    directoryChange(button->text() + ":/");
#else
    QString disk;
    if(button->text() != "/") {
        disk = "/dev/" + button->text();
        foreach(const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
            if(storage.device() == disk)
                directoryChange(storage.rootPath());
        }
    }
    else
        directoryChange(button->text());
#endif
}

void MainWindow::pathEdit_returnPressed() {
    QLineEdit *pathEdit = (QLineEdit*) sender();
    QString inputPath = pathEdit->text();

    QTabWidget *tabWidget;
    if(pathEdit->objectName() == "left")
        tabWidget = (QTabWidget*) ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = (QTabWidget*) ui->rightPanel->findChild<QTabWidget*>();

    MyTreeView *view = tabWidget->currentWidget()->findChild<MyTreeView*>();
    MySortFilterProxyModel *sortModel = view->sortModel;

    QFileInfo info(inputPath);
    pathEdit->clearFocus();

    QString prevPath = sortModel->fsModel->rootPath();
    if(prevPath.back() != '/')
        prevPath.append('/');

    if(QDir(inputPath).exists() && !inputPath.isEmpty()){
        if(info.isReadable() && info.isExecutable()) {
            pathEdit->setFocus();
            directoryChange(inputPath);
        }
        else {
            pathEdit->setText(QDir::toNativeSeparators(prevPath));
            QMessageBox::warning(this, "Access denied", "Do not have access rights to the directory.");
        }
    }
    else {
        pathEdit->setText(QDir::toNativeSeparators(prevPath));
        view->setFocus();
    }
}

void MainWindow::view_activated(const QModelIndex &index) {
    MyTreeView *view = static_cast<MyTreeView*> (sender());
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    QModelIndex fsModelIndex = sortModel->mapToSource(index);

    QFileInfo info = fsModel->fileInfo(fsModelIndex);
    QString path = fsModel->filePath(fsModelIndex);

#ifdef Q_OS_WIN
    if(info.isSymLink() && !QFile::exists(info.symLinkTarget())) {
        updateShortcuts(info.absoluteFilePath());
        path = fsModel->filePath(fsModelIndex);
    }
#endif

    if(info.isDir()) {
        if(info.isReadable() && info.isExecutable())
            directoryChange(path);
        else
            QMessageBox::warning(this, "Access denied", "Do not have access rights to the directory.");
    }
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::tabBar_doubleClicked(int index) {
    QTabWidget *tabWidget = static_cast<QTabWidget*> (sender());
    if(tabWidget->count() > 1) {
        tabWidget->removeTab(index);
        tabsUpdate(tabWidget);
        diskStatusUpdate(tabWidget);

        if(tabWidget->objectName() == "leftBar") {
            historyBackLeft.remove(index);
            historyForwardLeft.remove(index);
        }
        else {
            historyBackRight.remove(tabWidget->currentIndex());
            historyForwardRight.remove(tabWidget->currentIndex());
        }
    }
    (tabWidget->currentWidget()->findChild<MyTreeView*>())->setFocus();
}

void MainWindow::tabBar_indexChanged(int index){
    Q_UNUSED(index);
    QTabWidget *tabWidget = static_cast<QTabWidget*> (sender());
    if(tabWidget->currentWidget()->findChild<MyTreeView*>()) {
        MyTreeView *view = tabWidget->currentWidget()->findChild<MyTreeView*>();
        tabsUpdate(tabWidget);
        diskStatusUpdate(tabWidget);

        view->setFocus();
    }
}

void MainWindow::contextMenu_requested(const QPoint &point) {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    QModelIndex index = sortModel->mapToSource(view->indexAt(point));
    QString filePath = fsModel->filePath(index);

    if(QFileInfo(filePath).fileName() == "..")
        return;

    QMenu *menu = new QMenu(this);

    QAction *openAction = new QAction("Open");
    openAction->setShortcut(QKeySequence(Qt::Key_Return));
#ifdef Q_OS_WIN
    QAction *openWithAction = new QAction("Open with");
    openWithAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Return));
    connect(openWithAction, SIGNAL(triggered()), this, SLOT(on_actionOpen_with_triggered()));
    menu->addAction(openWithAction);
#endif
    QAction *editAction = new QAction("Edit");
    editAction->setShortcut(QKeySequence(Qt::Key_F4));
    QAction *removeAction = new QAction("Remove");
    removeAction->setShortcut(QKeySequence(Qt::Key_Delete));
    QAction *removePermanentlyAction = new QAction("Remove permanently");
    removePermanentlyAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Delete));
    QAction *renameAction = new QAction("Rename");
    renameAction->setShortcut(QKeySequence(Qt::Key_F2));
    QAction *cutAction = new QAction("Cut");
    cutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));
    QAction *copyAction = new QAction("Copy");
    copyAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
    QAction *pasteAction = new QAction("Paste");
    pasteAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_V));
    QAction *createShortcutAction = new QAction("Create shortcut");
    createShortcutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    QAction *newFileAction = new QAction("New file");
    newFileAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    QAction *newFolderAction = new QAction("New folder");
    newFolderAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    QAction *openTerminalAction = new QAction("Open Terminal in this directory.");
    openTerminalAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));

    connect(openAction, SIGNAL(triggered()), this, SLOT(on_actionOpen_selected_file_triggered()));

    connect(editAction, SIGNAL(triggered()), this, SLOT(on_editBtn_clicked()));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(on_actionRemove_triggered()));
    connect(removePermanentlyAction, SIGNAL(triggered()), this, SLOT(on_actionRemove_permanently_triggered()));
    connect(renameAction, SIGNAL(triggered()), this, SLOT(on_actionRename_triggered()));
    connect(cutAction, SIGNAL(triggered()), this, SLOT(on_actionCut_triggered()));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(on_actionCopy_triggered()));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(on_actionPaste_triggered()));
    connect(createShortcutAction, SIGNAL(triggered()), this, SLOT(on_actionCreate_Shortcut_triggered()));
    connect(newFileAction, SIGNAL(triggered()), this, SLOT(on_actionNew_File_triggered()));
    connect(newFolderAction, SIGNAL(triggered()), this, SLOT(on_actionCreate_Folder_triggered()));
    connect(openTerminalAction, SIGNAL(triggered()), this, SLOT(open_Terminal_triggered()));
    menu->addAction(openAction);
    menu->addAction(editAction);
    menu->addSeparator();
    menu->addAction(removeAction);
    menu->addAction(removePermanentlyAction);
    menu->addSeparator();
    menu->addAction(cutAction);
    menu->addAction(copyAction);
    menu->addAction(pasteAction);
    menu->addSeparator();
    menu->addAction(renameAction);
    menu->addAction(createShortcutAction);
    menu->addSeparator();
    menu->addAction(newFileAction);
    menu->addAction(newFolderAction);
    menu->addSeparator();
    menu->addAction(openTerminalAction);

    menu->popup(view->viewport()->mapToGlobal(point));
}

void MainWindow::viewHeader_clicked(int localIndex) {
    QHeaderView *header = static_cast<QHeaderView*> (sender());
    MyTreeView *view = static_cast<MyTreeView*> (header->parent());

    QSettings *settings = new QSettings("./history.ini", QSettings::IniFormat);
    if(view->objectName() == "left")
        settings->beginGroup("left_panel");
    else
        settings->beginGroup("right_panel");

    settings->setValue("sort_order_column", localIndex);

    if(header->sortIndicatorOrder() == Qt::AscendingOrder)
        settings->setValue("sort_direction", "asc");
    else
        settings->setValue("sort_direction", "desc");

    QTabWidget *tabWidget = (QTabWidget*) view->parent()->parent()->parent();
    for(int i = 0; i < tabWidget->count(); i++) {
        view = tabWidget->widget(i)->findChild<MyTreeView*>();
        if(header->sortIndicatorOrder() == Qt::AscendingOrder)
            view->sortByColumn(localIndex, Qt::AscendingOrder);
        else
            view->sortByColumn(localIndex, Qt::DescendingOrder);
    }
}


// file menu actions
void MainWindow::on_actionNew_File_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    bool ok = false;
    QString filename = QInputDialog::getText(this, tr("New File"), tr("Enter the file name:"), QLineEdit::Normal, "", &ok);

    if (ok && !filename.isEmpty()) {
        QFile file(fsModel->rootPath() + "/" + filename);
        if(isValidFileName(filename)) {
            if(!file.exists())
                file.open(QIODevice::NewOnly);
            else
                QMessageBox(QMessageBox::Warning, "Error", "File already exists!").exec();
        }
        else
            QMessageBox(QMessageBox::Warning, "Error", "Invalid file name!").exec();
    }

    view->clearSelection();
}

void MainWindow::on_actionOpen_selected_file_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    emit view->activated(view->currentIndex());
}

void MainWindow::on_actionOpen_with_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    QModelIndex fsModelIndex = sortModel->mapToSource(view->currentIndex());
    QFileInfo info = fsModel->fileInfo(fsModelIndex);
    QString filePath = info.filePath();

    if(info.isFile()) {
#ifdef Q_OS_WIN
        SHELLEXECUTEINFO shExecInfo = {sizeof(SHELLEXECUTEINFO)};
        shExecInfo.lpFile = reinterpret_cast<LPCWSTR>(filePath.utf16());
        shExecInfo.nShow = SW_SHOWNORMAL;
        shExecInfo.fMask = SEE_MASK_INVOKEIDLIST;
        shExecInfo.lpVerb = L"openas";

        if (ShellExecuteEx(&shExecInfo)) {
            // success
        } else {
            // error
            QMessageBox::critical(nullptr, "Error", "Something went wrong!");
        }
#else
        /*QMap<QString, QString> appMap;

        QDir dir("/usr/share/applications");
        QFileInfoList fileList = dir.entryInfoList(QStringList() << "*.desktop", QDir::Files);
        foreach(const QFileInfo &fileInfo, fileList) {
            QFile file(fileInfo.absoluteFilePath());
            if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QString appName, execCommand, iconPath;
                while(!in.atEnd()) {
                    QString line = in.readLine();
                    if(line.startsWith("Name="))
                        appName = line.section('=', 1);
                    else if(line.startsWith("Exec="))
                        execCommand = line.section('=', 1);

                    if(!appName.isEmpty() && !execCommand.isEmpty()) {
                        appMap.insert(appName, execCommand);
                        break;
                    }
                }
                file.close();
            }
        }
        dir = QDir(QDir::homePath() + "/.local/share/applications");
        fileList = dir.entryInfoList(QStringList() << "*.desktop", QDir::Files);
        foreach(const QFileInfo &fileInfo, fileList) {
            QFile file(fileInfo.absoluteFilePath());
            if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                QString appName, execCommand;
                while(!in.atEnd()) {
                    QString line = in.readLine();
                    if(line.startsWith("Name="))
                        appName = line.section('=', 1);
                    else if(line.startsWith("Exec="))
                        execCommand = line.section('=', 1);

                    if(!appName.isEmpty() && !execCommand.isEmpty()) {
                        appMap.insert(appName, execCommand);
                        break;
                    }
                }
                file.close();
            }
        }
        for (auto it = appMap.keyValueBegin(); it != appMap.keyValueEnd(); ++it) {
            qDebug() << it->first << it->second;
        }*/
#endif
    }
}

void MainWindow::on_actionRename_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    view->edit(view->currentIndex());
}

void MainWindow::on_actionRemove_triggered() {
    QStringList filePaths = getFileList();
    if (filePaths.isEmpty())
        return;

    QMessageBox msgRemoveConfirm;
    msgRemoveConfirm.setWindowTitle("Delete files");
    msgRemoveConfirm.setText("Do you want to delete files (" + QString::number(filePaths.length()) + ")?");
    msgRemoveConfirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgRemoveConfirm.setDefaultButton(QMessageBox::Yes);
    msgRemoveConfirm.setIcon(QMessageBox::Question);

    if(msgRemoveConfirm.exec() == QMessageBox::Yes) {
        QProgressDialog *progressDialog = new QProgressDialog("Removing files...", "Cancel", 0, filePaths.length());
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->setWindowTitle("Remove Progress");
        progressDialog->setMinimumDuration(50);

        int count = 0;
        progressDialog->open();
        foreach(const QString &file, filePaths) {
            QFile(file).moveToTrash();
            count++;
            progressDialog->setValue(count);
        }

        progressDialog->setValue(filePaths.length());
        progressDialog->close();
    }
}

void MainWindow::on_actionRemove_permanently_triggered() {
    QStringList filePaths = getFileList();
    if (filePaths.isEmpty())
        return;

    QMessageBox msgRemoveConfirm;
    msgRemoveConfirm.setWindowTitle("Permanent deletion");
    msgRemoveConfirm.setText("Do you want to delete files permanently (" + QString::number(filePaths.length()) + ")?");
    msgRemoveConfirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgRemoveConfirm.setDefaultButton(QMessageBox::Yes);
    msgRemoveConfirm.setIcon(QMessageBox::Question);

    if (msgRemoveConfirm.exec() == QMessageBox::Yes) {
        QProgressDialog *progressDialog = new QProgressDialog("Removing files permanently...", "Cancel", 0, filePaths.length());
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->setWindowTitle("Remove Progress");
        progressDialog->setMinimumDuration(50);

        int total = filePaths.length();
        int count = 0;

        progressDialog->open();

        foreach(const QString &file, filePaths) {
            QFileInfo fileInfo(file);

            if (fileInfo.isWritable()) {
                if (fileInfo.isDir() && !fileInfo.isSymLink()) {
                    QDir(file).removeRecursively();
                    total += QDir(file).count();
                    count += QDir(file).count();
                    progressDialog->setMaximum(total);
                }
                else {
                    QFile(file).remove();
                    count++;
                }
                progressDialog->setValue(count);
            } else { // if a file has read-only attribute
                QMessageBox readOnlyMsg;
                readOnlyMsg.setWindowTitle("Read-only file");
                readOnlyMsg.setText("The file \"" + fileInfo.fileName() + "\" is read-only. Do you want to delete it?");
                readOnlyMsg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                readOnlyMsg.setDefaultButton(QMessageBox::Yes);
                readOnlyMsg.setIcon(QMessageBox::Warning);

                if (readOnlyMsg.exec() == QMessageBox::Yes) {
                    QFile fileObject(file);
                    // Remove read-only attribute
                    fileObject.setPermissions(fileObject.permissions() | QFile::WriteOwner);
                    // Remove the file
                    if (fileInfo.isDir()) {
                        QDir(file).removeRecursively();
                        total += QDir(file).count();
                        count += QDir(file).count();
                        progressDialog->setMaximum(total);
                    } else {
                        fileObject.remove();
                        count++;
                    }
                    progressDialog->setValue(count);
                }
            }
        }
        progressDialog->setValue(total);
        progressDialog->close();
    }
}

void MainWindow::on_actionCreate_Folder_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());

    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    bool ok = false;
    QString foldername = QInputDialog::getText(this, tr("New Folder"), tr("Enter the folder name:"), QLineEdit::Normal, "", &ok);

    if (ok && !foldername.isEmpty()) {
        QDir folder(fsModel->rootPath() + "/" + foldername);
        if(isValidFileName(foldername)) {
            if(!folder.exists()) {
                folder.mkdir(fsModel->rootPath() + "/" + foldername);
            }
            else
                QMessageBox(QMessageBox::Warning, "Error", "Directory already exists!").exec();
        }
        else
            QMessageBox(QMessageBox::Warning, "Error", "Invalid folder name!").exec();
    }
    view->clearSelection();
}

void MainWindow::on_actionCreate_Shortcut_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    QModelIndex fsModelIndex = sortModel->mapToSource(view->selectionModel()->currentIndex());
    QString filePath = fsModel->filePath(fsModelIndex);

    if(QFileInfo(filePath).fileName() == "..")
        return;

    QString targetPath = QFileInfo(filePath).absoluteFilePath() + " - Shortcut";

#ifdef Q_OS_WIN
    targetPath.append(".lnk");
#endif

    if(!QFile(targetPath).exists())
        QFile::link(filePath, targetPath);
    else
        QMessageBox(QMessageBox::Warning, "Error", "Shortcut already exists!").exec();

    view->clearSelection();
}

void MainWindow::on_actionCut_triggered() {
    QStringList filePaths = getFileList();
    if (filePaths.isEmpty())
        return;

    QClipboard *clipboard = QApplication::clipboard();
    QMimeData *data = new QMimeData();
    QList<QUrl> urlList;
    foreach(const QString &file, filePaths) {
        urlList.append(QUrl::fromLocalFile(file));
    }

    data->setUrls(urlList);
    clipboard->setMimeData(data);

    if(!urlList.isEmpty())
        isCutted = true;
}

void MainWindow::on_actionCopy_triggered() {
    QStringList filePaths = getFileList();
    if (filePaths.isEmpty())
        return;

    isCutted = false;

    QClipboard *clipboard = QApplication::clipboard();
    QMimeData *data = new QMimeData();
    QList<QUrl> urlList;
    for(QString &file : filePaths)
        urlList.append(QUrl::fromLocalFile(file));

    data->setUrls(urlList);
    clipboard->setMimeData(data);
}

void MainWindow::on_actionPaste_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    QClipboard *clipboard = QApplication::clipboard();

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    const QMimeData *data = clipboard->mimeData();
    QList<QUrl> urlList = data->urls();
    QStringList filePaths;

    foreach(const QUrl &url, urlList){
#ifdef Q_OS_WIN
        filePaths.append(url.path().remove(0, 1));
#else
        filePaths.append(url.path());
#endif
    }

    QModelIndex fsModelIndex = sortModel->mapToSource(view->currentIndex());
    QFileInfo destInfo = fsModel->fileInfo(fsModelIndex);
    QString targetPath = fsModel->filePath(fsModelIndex);

    if(destInfo.isDir()) {
        if(fsModelIndex.data().toString() == "..")
            targetPath.chop(2);

        if(targetPath.at(targetPath.length() - 1) != '/')
            targetPath.append('/');
    }
    else
        targetPath = destInfo.absolutePath() + "/";

    // if files was cutted, then copy with removing or move
    if(isCutted) {
        // if storage is the same, then just move file, otherwise copy with removing
        if(QStorageInfo(filePaths[0]).device() == QStorageInfo(targetPath).device()) {
            bool skipAll = false;
            bool overwriteAll = false;
            for(QString &file : filePaths) {
                QFileInfo fileInfo = QFileInfo(file);

                // check if moved folder is not parent of destination folder
                if(fileInfo.isDir() && targetPath.contains(file)) {
                    QMessageBox::warning(nullptr, "Error", "The source directory is the parent of the target directory!");
                    return;
                }

                // check if moved file/folder is not destination file/folder
                if(file == targetPath + fileInfo.fileName()) {
                    QMessageBox::warning(nullptr, "Error", "It is not possible to move file or folder to itself!");
                    return;
                }

                if(QFileInfo::exists(targetPath + fileInfo.fileName())) {
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
                else {
                    QFile::rename(file, targetPath + fileInfo.fileName());
                }

            }
        }
        else {
            fsModel->copyFiles(filePaths, targetPath);
            for(QString &file : filePaths) {
                QFileInfo fileInfo = QFileInfo(file);
                if(fileInfo.exists() && QFileInfo::exists(targetPath + fileInfo.fileName())) {
                    if(fileInfo.isDir() && !fileInfo.isSymLink())
                        QDir(file).removeRecursively();
                    else
                        QFile(file).moveToTrash();
                }
            }
        }
    }
    //else just copy files
    else
        fsModel->copyFiles(filePaths, targetPath);

    isCutted = false;
    view->clearSelection();
}


// selection menu actions
void MainWindow::on_actionSelect_file_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    QModelIndex index = view->currentIndex();

    if (view->selectionModel()->isSelected(index)) {
        view->selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    } else {
        view->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

void MainWindow::on_actionSelect_all_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    view->selectAll();

    QModelIndex dotdot = view->model()->index(0, 0, view->rootIndex());
    if (dotdot.isValid() && dotdot.data() == "..") {
        view->selectionModel()->select(dotdot, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    }
}

void MainWindow::on_actionRemove_selection_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    view->clearSelection();
}


// tab menu actions
void MainWindow::on_actionCreate_a_new_tab_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());

    QTabWidget *tabWidget = static_cast<QTabWidget*> (view->parent()->parent()->parent());
    QString path = view->sortModel->fsModel->rootPath();

    createView(tabWidget, path);
    tabsUpdate(tabWidget);
}

void MainWindow::on_actionClose_this_tab_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    QTabWidget *tabWidget;
    if(qApp->focusWidget()->objectName() == "left")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    if(tabWidget->count() > 1) {
        tabWidget->removeTab(tabWidget->currentIndex());

        tabsUpdate(tabWidget);
        diskStatusUpdate(tabWidget);

        if(qApp->focusWidget()->objectName() == "left") {
            historyBackLeft.remove(tabWidget->currentIndex());
            historyForwardLeft.remove(tabWidget->currentIndex());
        }
        else {
            historyBackRight.remove(tabWidget->currentIndex());
            historyForwardRight.remove(tabWidget->currentIndex());
        }
    }
    (tabWidget->currentWidget()->findChild<MyTreeView*>())->setFocus();
}

void MainWindow::on_actionClose_all_tabs_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    QTabWidget *tabWidget;

    if(qApp->focusWidget()->objectName() == "left")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    int cnt = tabWidget->count();

    if(qApp->focusWidget()->objectName() == "left") {
        historyBackLeft.move(tabWidget->currentIndex(), 0);
        historyForwardLeft.move(tabWidget->currentIndex(), 0);

        for (int i = 0; i < cnt - 1; i++) {
            historyBackLeft.remove(1);
            historyForwardLeft.remove(1);
        }
    }
    else {
        historyBackRight.move(tabWidget->currentIndex(), 0);
        historyForwardRight.move(tabWidget->currentIndex(), 0);

        for (int i = 0; i < cnt - 1; i++) {
            historyBackRight.remove(1);
            historyForwardRight.remove(1);
        }
    }

    tabWidget->tabBar()->moveTab(tabWidget->currentIndex(), 0);

    for(int i = 0; i < cnt - 1; i++)
        tabWidget->removeTab(1);

    tabsUpdate(tabWidget);
}

void MainWindow::on_actionSwitch_to_the_next_tab_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    QTabWidget *tabWidget;
    if(qApp->focusWidget()->objectName() == "left")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    if(tabWidget->currentIndex() + 1 < tabWidget->count())
        tabWidget->setCurrentIndex(tabWidget->currentIndex() + 1);

    tabsUpdate(tabWidget);
}

void MainWindow::on_actionSwitch_to_the_previous_tab_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    QTabWidget *tabWidget;
    if(qApp->focusWidget()->objectName() == "left")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    if(tabWidget->currentIndex() - 1 >= 0)
        tabWidget->setCurrentIndex(tabWidget->currentIndex() - 1);

    tabsUpdate(tabWidget);
}

void MainWindow::on_actionOpen_the_folder_in_the_new_tab_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    QTabWidget *tabWidget;
    if(qApp->focusWidget()->objectName() == "left")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    QModelIndex fsModelIndex = sortModel->mapToSource(view->currentIndex());
    QString filePath = fsModel->filePath(fsModelIndex);

    if(QFileInfo(filePath).isFile())
        return;

    createView(tabWidget, filePath);
    tabsUpdate(tabWidget);
}

void MainWindow::on_actionOpen_the_folder_in_the_new_tab_in_another_bar_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    QTabWidget *tabWidget;
    if(qApp->focusWidget()->objectName() == "right")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    QModelIndex fsModelIndex = sortModel->mapToSource(view->currentIndex());
    QString filePath = fsModel->filePath(fsModelIndex);

    if(QFileInfo(filePath).isFile())
        return;

    createView(tabWidget, filePath);
    tabsUpdate(tabWidget);
}


// tools menu actions
void MainWindow::on_actionFile_search_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    QModelIndex fsModelIndex = sortModel->mapToSource(view->currentIndex());
    QString filePath = fsModel->filePath(fsModelIndex).left(fsModel->filePath(fsModelIndex).lastIndexOf('/')) + '/';

    MySearchDialog *searchDialog = new MySearchDialog(filePath);
    searchDialog->exec();

    if(!searchDialog->getSuccess())
        return;

    QString file = searchDialog->getFileToShow();

    QTabWidget *tabWidget = (QTabWidget*) view->parent()->parent()->parent();
    QString path = file.left(file.lastIndexOf('/'));
    QString folderName;
    if(QDir(path).isRoot()) {
        folderName = QDir(path).absoluteFilePath(path);
        folderName.chop(1);
    }
    else
        folderName = QDir(path).dirName();

    auto index = tabWidget->addTab(new QLabel(folderName), folderName);
    tabWidget->setCurrentIndex(index);
    createView(tabWidget, path);

    tabsUpdate(tabWidget);
    diskStatusUpdate(tabWidget);

    view = tabWidget->currentWidget()->findChild<MyTreeView*>();
    sortModel = view->sortModel;
    fsModel = sortModel->fsModel;

    fsModelIndex = fsModel->index(file);
    QModelIndex sortModelIndex = sortModel->mapFromSource(fsModelIndex);

    view->setVisible(true);
    view->setFocus();
    view->setCurrentIndex(sortModelIndex);
    view->scrollToFile();
}

void MainWindow::on_actionShow_Hide_hidden_files_triggered() {
    QTabWidget *tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    MyTreeView *view;
    MySortFilterProxyModel *sortModel;
    MyFileSystemModel *fsModel;

    QSettings *settings = new QSettings("./config.ini", QSettings::IniFormat);
    settings->beginGroup("config");

    if(!settings->value("show_hidden").toInt()) {
        for(int i = 0; i < tabWidget->count(); i++) {
            view = tabWidget->widget(i)->findChild<MyTreeView*>();
            sortModel = view->sortModel;
            fsModel = sortModel->fsModel;
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files | QDir::Hidden);
        }
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();
        for(int i = 0; i < tabWidget->count(); i++) {
            view = tabWidget->widget(i)->findChild<MyTreeView*>();
            sortModel = view->sortModel;
            fsModel = sortModel->fsModel;
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files | QDir::Hidden);
        }
        settings->setValue("show_hidden", 1);
    }
    else {
        for(int i = 0; i < tabWidget->count(); i++) {
            view = tabWidget->widget(i)->findChild<MyTreeView*>();
            sortModel = view->sortModel;
            fsModel = sortModel->fsModel;
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files);
        }
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();
        for(int i = 0; i < tabWidget->count(); i++) {
            view = tabWidget->widget(i)->findChild<MyTreeView*>();
            sortModel = view->sortModel;
            fsModel = sortModel->fsModel;
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files);
        }
        settings->setValue("show_hidden", 0);
    }

    settings->endGroup();
}

void MainWindow::open_Terminal_triggered() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    QModelIndex fsModelIndex = sortModel->mapToSource(view->currentIndex());
    QFileInfo file = fsModel->fileInfo(fsModelIndex);

    QString path;
    if(file.isDir()) {
        if(view->currentIndex().data() == "..") {
            path = file.filePath();
            path.chop(2);
        }
        else
            path = file.absoluteFilePath();
    }
    else
        path = file.absolutePath();

#ifdef Q_OS_WIN
    // Open cmd on Windows
    QProcess::startDetached("C:/Windows/System32/cmd.exe", QStringList() << "/K" << QString("cd /d \"%1\"").arg(path));
    return;
#elif Q_OS_MACOS
    // Open Terminal on MacOS
    QProcess::startDetached("open", QStringList() << "-a" << "Terminal" << path);
    return;
#else
    // Find and open terminal on Linux
    QStringList terminalList = {
        qgetenv("TERMINAL"), "x-terminal-emulator", "mate-terminal", "gnome-terminal", "terminator",
        "xfce4-terminal", "urxvt", "rxvt", "termit", "Eterm", "aterm", "uxterm", "xterm",
        "roxterm", "termite", "lxterminal", "terminology", "st", "qterminal",
        "lilyterm", "tilix", "terminix", "konsole", "kitty", "guake", "tilda",
        "alacritty", "hyper", "wezterm", "rio"
    };
    for (const QString &terminal : terminalList) {
        QProcess process;
        process.start("which", QStringList() << terminal);
        process.waitForFinished();

        if (process.exitCode() == 0) {
            QStringList args;
            args << "-e" << QString("bash -c 'cd \"%1\" && exec bash'").arg(path);
            QProcess::startDetached(terminal, args);
        }
    }
#endif
}


// bottom buttons actions
void MainWindow::on_editBtn_clicked() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
    MySortFilterProxyModel *sortModel = view->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;

    QModelIndex fsModelIndex = sortModel->mapToSource(view->currentIndex());
    QFileInfo file = fsModel->fileInfo(fsModelIndex);

    if(file.isFile() && file.isWritable()) {
        QProcess *process = new QProcess(this);

#ifdef Q_OS_WIN
        // Open with Notepad in Windows
        process->start("C:/Windows/System32/Notepad.exe", QStringList() << file.absoluteFilePath());
        return;
#else
        // open with default text editor in Linux
        process->start("xdg-mime", QStringList() << "query" << "default" << "text/plain");
        process->waitForFinished();
        QString output = process->readAllStandardOutput().trimmed();
        QStringList splittedOutput = output.split(".");
        output = splittedOutput[splittedOutput.size()-2];
        process->start(output, QStringList() << file.absoluteFilePath());
#endif
    }
}

void MainWindow::on_copyBtn_clicked() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    MyTreeView *sourceView = static_cast<MyTreeView*> (qApp->focusWidget());

    QStringList filePaths = getFileList();
    if (filePaths.isEmpty())
        return;

    // find target tabwidget, view and models
    QTabWidget *tabWidget;
    if(sourceView->objectName() == "left")
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();

    MyTreeView *targetView = tabWidget->currentWidget()->findChild<MyTreeView*>();
    MyFileSystemModel *targetFsModel = targetView->sortModel->fsModel;

    QString destPath = targetFsModel->rootPath();
    if(destPath.at(destPath.length() - 1) != '/')
        destPath.append('/');

    QMessageBox msgCopyConfirm;
    msgCopyConfirm.setWindowTitle("Copy files");
    msgCopyConfirm.setText("Do you want to copy files (" + QString::number(filePaths.length()) + ") to " + destPath + "?");
    msgCopyConfirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgCopyConfirm.setDefaultButton(QMessageBox::Yes);
    msgCopyConfirm.setIcon(QMessageBox::Question);

    if(msgCopyConfirm.exec() == QMessageBox::Yes) {
        targetFsModel->copyFiles(filePaths, destPath);
        sourceView->clearSelection();
    }
}

void MainWindow::on_moveBtn_clicked() {
    if (!qobject_cast<MyTreeView*>(qApp->focusWidget())) return;

    QStringList filePaths = getFileList();
    if (filePaths.isEmpty())
        return;

    MyTreeView *sourceView = static_cast<MyTreeView*> (qApp->focusWidget());

    on_actionCut_triggered();

    // find target tabwidget, view and models
    QTabWidget *tabWidget;
    if(sourceView->objectName() == "left")
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();

    MyTreeView *targetView = tabWidget->currentWidget()->findChild<MyTreeView*>();
    MySortFilterProxyModel *sortModel = targetView->sortModel;
    MyFileSystemModel *fsModel = sortModel->fsModel;
    QString rootPath = fsModel->rootPath();

    QMessageBox *msgMoveConfirm = new QMessageBox(this);
    msgMoveConfirm->setWindowTitle("Move files");
    msgMoveConfirm->setModal(true);
    msgMoveConfirm->setText("Do you want to move files (" + QString::number(filePaths.length()) + ") to " + rootPath + "?");
    msgMoveConfirm->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgMoveConfirm->setDefaultButton(QMessageBox::Yes);
    msgMoveConfirm->setIcon(QMessageBox::Question);

    if(msgMoveConfirm->exec() == QMessageBox::Yes) {
        // set cursor on the first row
        QModelIndex rootIndex = sortModel->mapFromSource(fsModel->index(rootPath));
        if (rootIndex.isValid() && targetView->model()->rowCount(rootIndex) > 0) {
            QModelIndex firstChildIndex = targetView->model()->index(0, 0, rootIndex);
            if (firstChildIndex.isValid()) {
                targetView->selectionModel()->setCurrentIndex(firstChildIndex, QItemSelectionModel::NoUpdate);
            }
        }
    }

    QTimer::singleShot(50, this, [=]() {
        targetView->setFocus();
        on_actionPaste_triggered();
    });
}

void MainWindow::on_folderBtn_clicked() {
    on_actionCreate_Folder_triggered();
}

void MainWindow::on_deleteBtn_clicked() {
    on_actionRemove_triggered();
}


// up button action
void MainWindow::upBtn_clicked() {
    QPushButton *button = static_cast<QPushButton*> (sender());
    QTabWidget *tabWidget;

    if(button->objectName() == "left")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    MyTreeView *view = static_cast<MyTreeView*>(tabWidget->findChild<MyTreeView*>());
    if(!view) return;

    QLineEdit *pathEdit = tabWidget->findChild<QLineEdit*>();
    QString currentPath = pathEdit->text();

    if(!QDir(currentPath).isRoot()) {
        view->setFocus();
        directoryChange(currentPath + "..");
    }
}

// history and buttons back/forward
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {

        if (!qobject_cast<MyTreeView*> (qApp->focusWidget()))
            return QMainWindow::eventFilter(obj, event);

        MyTreeView *view = static_cast<MyTreeView*> (qApp->focusWidget());
        QTabWidget *tabWidget;
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*> (event);

        if (mouseEvent->button() == Qt::BackButton) {
            if (view->objectName() == "left") {
                tabWidget = ui->leftPanel->findChild<QTabWidget*>();
                QLineEdit *pathEdit = static_cast<QLineEdit*>(tabWidget->currentWidget()->findChild<QLineEdit*>());
                QString currentPath = pathEdit->text();
                int currentTab = tabWidget->currentIndex();

                if(!historyBackLeft[currentTab].isEmpty()) {
                    historyForwardLeft[currentTab].push(currentPath);
                    QString previousPath = historyBackLeft[currentTab].pop();
                    isNavTriggered = true;
                    directoryChange(previousPath);
                }
            }
            else {
                tabWidget = ui->rightPanel->findChild<QTabWidget*>();
                QLineEdit *pathEdit = static_cast<QLineEdit*>(tabWidget->currentWidget()->findChild<QLineEdit*>());
                QString currentPath = pathEdit->text();
                int currentTab = tabWidget->currentIndex();

                if(!historyBackRight[currentTab].isEmpty()) {
                    historyForwardRight[currentTab].push(currentPath);
                    QString previousPath = historyBackRight[currentTab].pop();
                    isNavTriggered = true;
                    directoryChange(previousPath);
                }
            }
            return true;
        }
        else if (mouseEvent->button() == Qt::ForwardButton) {
            if (view->objectName() == "left") {
                tabWidget = ui->leftPanel->findChild<QTabWidget*>();
                QLineEdit *pathEdit = tabWidget->currentWidget()->findChild<QLineEdit*>();
                QString currentPath = pathEdit->text();
                int currentTab = tabWidget->currentIndex();

                if(!historyForwardLeft[currentTab].isEmpty()) {
                    historyBackLeft[currentTab].push(currentPath);
                    QString nextPath = historyForwardLeft[currentTab].pop();
                    isNavTriggered = true;
                    directoryChange(nextPath);
                }

            }
            else {
                tabWidget = ui->rightPanel->findChild<QTabWidget*>();
                QLineEdit *pathEdit = tabWidget->currentWidget()->findChild<QLineEdit*>();
                QString currentPath = pathEdit->text();
                int currentTab = tabWidget->currentIndex();

                if (!historyForwardRight[currentTab].isEmpty()) {
                    historyBackRight[currentTab].push(currentPath);
                    QString nextPath = historyForwardRight[currentTab].pop();
                    isNavTriggered = true;
                    directoryChange(nextPath);
                }
            }
            return true;
        }

    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::addToHistory(QString path, int currentTab, QString panel) {
    if(panel == "left") {
        historyForwardLeft[currentTab].clear();
        historyBackLeft[currentTab].push(path);
    }
    else {
        historyForwardRight[currentTab].clear();
        historyBackRight[currentTab].push(path);
    }
}
