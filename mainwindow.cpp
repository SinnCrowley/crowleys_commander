/*  TODO
 *
 *  add folders to file search
 *
 *  ускорить загрузку больших папок ( + возможно подгружать внутренности папок в фоне)
 *
 *  навести красоту в коде
 *
 *  навести красоту внешне
 *
 *  add buttons ahead and back
 *
 *  сделать ползунок удаления/копирования/перемещения
 *
 *  add to menu: open this folder in terminal
 *
 *
 *
 *  FUTURE
 *
 *  on device connection add only new device (do not reload all panel)
 *
 *  on file cutting set the icons a bit transparent
 *
 *  set the hidden files icons different
 *
 *  check the default text editor on Windows and open it by button "Edit"
 *
 *  add pack/unpack, settings, group rename, copy/delete/move in background
 *
 *  add to menu: copy path, properties, share
 *
 *  add function in settings select or not select file extension while rename
 *
 *  add full UI customization in settings (colors, fonts, sizes)
 *
*/
#include <QtGlobal>

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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mysearchdialog.h"

#include "myfilesystemmodel.h"
#include "mytreeview.h"
#include "mysortfilterproxymodel.h"
#include "devicewatcher.h"

bool isCutted = false;

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
    QVBoxLayout *barLayout = new QVBoxLayout(tabBar->currentWidget());
    barLayout->setSpacing(0);
    barLayout->setContentsMargins(0, 0, 0, 0);

    MyTreeView *view = new MyTreeView(this);
    QLineEdit *pathEdit = new QLineEdit(this);
    if(QDir(path).isRoot())
        pathEdit->setText(QDir::toNativeSeparators(QDir(path).absolutePath()));
    else
        pathEdit->setText(QDir::toNativeSeparators(QDir(path).absolutePath() + "/"));
    pathEdit->setFocusPolicy(Qt::ClickFocus);

    barLayout->addWidget(pathEdit);
    barLayout->addWidget(view);

    MyFileSystemModel *fsModel = new MyFileSystemModel(this);
    fsModel->setRootPath(path);

    MySortFilterProxyModel *sortModel = new MySortFilterProxyModel();
    sortModel->setSourceModel(fsModel);

    view->setModel(sortModel);
    view->setRootIndex(sortModel->mapFromSource(fsModel->index(path)));

    pathEdit->setObjectName(tabBar->objectName().left(tabBar->objectName().indexOf("B")));
    view->setObjectName(tabBar->objectName().left(tabBar->objectName().indexOf("B")));

    connect(view, SIGNAL(activated(QModelIndex)), this, SLOT(view_activated(QModelIndex)));
    connect(view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_requested(QPoint)));
    connect(pathEdit, SIGNAL(returnPressed()), this, SLOT(pathEdit_returnPressed()));
    connect(view->header(), SIGNAL(sectionClicked(int)), this, SLOT(viewHeader_clicked(int)));

    view->header()->moveSection(1, 2);
    view->setColumnWidth(1, 65);
    view->setColumnWidth(2, 75);
    view->setColumnWidth(3, 100);
    view->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    view->header()->setStretchLastSection(false);

    diskStatusUpdate(tabBar);

    QSettings *settings = new QSettings("./history.ini", QSettings::IniFormat);
    if(view->objectName() == "left")
        settings->beginGroup("left_panel");
    else
        settings->beginGroup("right_panel");

    int sortingColumn = settings->value("sort_order_column").toInt();

    if(settings->value("sort_direction").toString() == "asc")
        view->sortByColumn(sortingColumn, Qt::AscendingOrder);
    else
        view->sortByColumn(sortingColumn, Qt::DescendingOrder);

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
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*) sortModel->sourceModel();
    fsModel->setRootPath(path);
    view->setRootIndex(sortModel->mapFromSource(fsModel->index(path)));

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
        connect(view->model(), &QAbstractItemModel::layoutChanged, this, [view, sortModel, fsModel, previousPath]() {
            view->selectionModel()->setCurrentIndex(sortModel->mapFromSource(fsModel->index(previousPath)), QItemSelectionModel::NoUpdate);
            view->scrollToFile();
        });
    }
    else {
        // set cursor on the first row
        connect(view->model(), &QAbstractItemModel::layoutChanged, this, [view, sortModel, fsModel, path]() {
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
}

// update disk status bar
void MainWindow::diskStatusUpdate(QTabWidget *tabWidget) {
    if(!tabWidget->currentWidget()->findChild<MyTreeView*>())
        return;

    MyTreeView *view = tabWidget->currentWidget()->findChild<MyTreeView*>();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();
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
void MainWindow::getFileList(QStringList &files) {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();

    QStringList filePaths;

    if(view->selectionModel()->selectedIndexes().isEmpty()) {
        QModelIndex fsModelIndex = sortModel->mapToSource(view->selectionModel()->currentIndex());
        QFileInfo info = fsModel->fileInfo(fsModelIndex);

        if(info.fileName() == "..")
            return;
        filePaths.append(info.absoluteFilePath());
    }
    else {
        foreach(const QModelIndex &index, view->selectionModel()->selectedIndexes()) {
            QModelIndex fsModelIndex = sortModel->mapToSource(index);
            QFileInfo info = fsModel->fileInfo(fsModelIndex);

            if(info.fileName() != "..")
                filePaths.append(info.absoluteFilePath());
        }
        filePaths.removeDuplicates();
    }

    files = filePaths;
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
                qWarning() << "Cannot create udev context";
                continue;
            }

            struct udev_enumerate *enumerate = udev_enumerate_new(udev);
            if (!enumerate) {
                udev_unref(udev);
                qWarning() << "Cannot create udev enumerate";
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
    QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui->diskButtonsLeftLayout->addItem(spacer);
    ui->diskButtonsRightLayout->addItem(spacer);

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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , deviceWatcher(new DeviceWatcher(this)) {
    ui->setupUi(this);

    // initialization on first run
    QFile history = QFile("./history.ini");
    if(!history.exists()) {
        history.open(QIODevice::Append);

        QString historyData = "[left_panel]\nsort_order_column=0\nsort_direction=asc\n\n \
[right_panel]\nsort_order_column=0\nsort_direction=asc\n\n \
[left_tabs]\nactive_tab=0\n0_path=%1\n\n \
[right_tabs]\nactive_tab=0\n0_path=%1";
#ifdef Q_OS_WIN
        history.write(historyData.arg("C:\\").toStdString().c_str());
#else
        history.write(historyData.arg("/").toStdString().c_str());
#endif
        history.close();
    }

    connect(deviceWatcher, &DeviceWatcher::deviceConnected, this, &MainWindow::deviceUpdate);
    connect(deviceWatcher, &DeviceWatcher::deviceDisconnected, this, &MainWindow::deviceUpdate);

    deviceUpdate("");

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


    QTabWidget *leftBar = new QTabWidget;
    leftBar->setObjectName("leftBar");
    leftBar->setMovable(true);
    ui->leftPanelLayout->addWidget(leftBar);

    connect(leftBar, SIGNAL(tabBarDoubleClicked(int)), this, SLOT(tabBar_doubleClicked(int)));
    connect(leftBar, SIGNAL(currentChanged(int)), this, SLOT(tabBar_indexChanged(int)));

    QString folderName;
    QSettings *settings = new QSettings("./history.ini", QSettings::IniFormat);
    settings->beginGroup("left_tabs");
    QStringList keys = settings->childKeys();
    int idx = settings->value("active_tab").toInt();
    foreach(const QString key, keys) {
        if(key[0].isDigit()){
            if(QDir(settings->value(key).toString()).isRoot()) {
                folderName = QDir(settings->value(key).toString()).absoluteFilePath(settings->value(key).toString());
                if(folderName != "/")
                    folderName.chop(1);
            }
            else
                folderName = QDir(settings->value(key).toString()).dirName();

            auto index = leftBar->addTab(new QLabel(folderName), folderName);
            leftBar->setCurrentIndex(index);
            createView(leftBar, settings->value(key).toString());
        }
    }
    leftBar->setCurrentIndex(idx);
    settings->endGroup();

    diskStatusUpdate(leftBar);

    QTabWidget *rightBar = new QTabWidget;
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
        if(key[0].isDigit()){
            if(QDir(settings->value(key).toString()).isRoot()) {
                folderName = QDir(settings->value(key).toString()).absoluteFilePath(settings->value(key).toString());
                folderName.chop(1);
            }
            else
                folderName = QDir(settings->value(key).toString()).dirName();

            auto index = rightBar->addTab(new QLabel(folderName), folderName);
            rightBar->setCurrentIndex(index);
            createView(rightBar, settings->value(key).toString());
        }
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
            sortModel = (MySortFilterProxyModel*) view->model();
            fsModel = (MyFileSystemModel*) sortModel->sourceModel();
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files | QDir::Hidden);
        }
        for(int i = 0; i < rightBar->count(); i++) {
            view = rightBar->widget(i)->findChild<MyTreeView*>();
            sortModel = (MySortFilterProxyModel*) view->model();
            fsModel = (MyFileSystemModel*) sortModel->sourceModel();
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files | QDir::Hidden);
        }
    }

    settings->endGroup();

    diskStatusUpdate(rightBar);

    connect(ui->diskListLeft, SIGNAL(textActivated(QString)), this, SLOT(diskList_textActivated(QString)));
    connect(ui->diskListRight, SIGNAL(textActivated(QString)), this, SLOT(diskList_textActivated(QString)));

    leftBar->setFocusPolicy(Qt::NoFocus);
    rightBar->setFocusPolicy(Qt::NoFocus);

    QShortcut *editShortcut = new QShortcut(QKeySequence(Qt::Key_F4), this);
    QObject::connect(editShortcut, SIGNAL(activated()), this, SLOT(on_editBtn_clicked()));

    QShortcut *copyShortcut = new QShortcut(QKeySequence(Qt::Key_F5), this);
    QObject::connect(copyShortcut, SIGNAL(activated()), this, SLOT(on_copyBtn_clicked()));

    QShortcut *moveShortcut = new QShortcut(QKeySequence(Qt::Key_F6), this);
    QObject::connect(moveShortcut, SIGNAL(activated()), this, SLOT(on_moveBtn_clicked()));

    QShortcut *folderShortcut = new QShortcut(QKeySequence(Qt::Key_F7), this);
    QObject::connect(folderShortcut, SIGNAL(activated()), this, SLOT(on_folderBtn_clicked()));

    QShortcut *deleteShortcut = new QShortcut(QKeySequence(Qt::Key_F8), this);
    QObject::connect(deleteShortcut, SIGNAL(activated()), this, SLOT(on_deleteBtn_clicked()));
}

MainWindow::~MainWindow() {
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

    MyTreeView *view = (MyTreeView*) tabWidget->currentWidget()->findChild<MyTreeView*>();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();

    pathEdit->clearFocus();
    if(QDir(inputPath).exists() && !inputPath.isEmpty()){
        pathEdit->setFocus();
        directoryChange(inputPath);
    }
    else {
        QString prevPath = ((MyFileSystemModel*)sortModel->sourceModel())->rootPath();
        if(prevPath.back() != '/')
            prevPath.append('/');
        pathEdit->setText(QDir::toNativeSeparators(prevPath));
        return;
    }
}

void MainWindow::view_activated(const QModelIndex &index) {
    MyTreeView *view = (MyTreeView*) sender();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();

    QModelIndex fsModelIndex = sortModel->mapToSource(index);

    QFileInfo info = fsModel->fileInfo(fsModelIndex);
    QString path = fsModel->filePath(fsModelIndex);

#ifdef Q_OS_WIN
    if(info.isSymLink() && !QFile::exists(info.symLinkTarget())) {
        updateShortcuts(info.absoluteFilePath());
        path = fsModel->filePath(fsModelIndex);
    }
#endif

    if(info.isDir())
        directoryChange(path);
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void MainWindow::tabBar_doubleClicked(int index) {
    QTabWidget *tabWidget = (QTabWidget*) sender();
    if(tabWidget->count() > 1) {
        tabWidget->removeTab(index);
        tabsUpdate(tabWidget);
        diskStatusUpdate(tabWidget);
    }
    (tabWidget->currentWidget()->findChild<MyTreeView*>())->setFocus();
}

void MainWindow::tabBar_indexChanged(int index){
    Q_UNUSED(index);
    QTabWidget *tabWidget = (QTabWidget*) sender();
    if(tabWidget->currentWidget()->findChild<MyTreeView*>()) {
        MyTreeView *view = tabWidget->currentWidget()->findChild<MyTreeView*>();
        tabsUpdate(tabWidget);
        diskStatusUpdate(tabWidget);

        view->setFocus();
    }
}

void MainWindow::contextMenu_requested(const QPoint &point) {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();

    QModelIndex index = sortModel->mapToSource(view->indexAt(point));
    QString filePath = fsModel->filePath(index);

    if(QFileInfo(filePath).fileName() == "..")
        return;

    QMenu *menu = new QMenu();

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
    QAction *openTerminal = new QAction("Open Terminal in this directory.");
    openTerminal->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));

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

    menu->popup(view->viewport()->mapToGlobal(point));
}

void MainWindow::viewHeader_clicked(int localIndex) {
    QHeaderView *header = (QHeaderView*) sender();
    MyTreeView *view = (MyTreeView*) header->parent();

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
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();

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
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    emit view->activated(view->currentIndex());
}

void MainWindow::on_actionOpen_with_triggered() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();

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
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    view->edit(view->currentIndex());
}

void MainWindow::on_actionRemove_triggered() {
    QStringList filePaths;
    getFileList(filePaths);

    QMessageBox msgRemoveConfirm;
    msgRemoveConfirm.setWindowTitle("Delete files");
    msgRemoveConfirm.setText("Do you want to delete files (" + QString::number(filePaths.length()) + ")?");
    msgRemoveConfirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgRemoveConfirm.setDefaultButton(QMessageBox::Yes);
    msgRemoveConfirm.setIcon(QMessageBox::Question);

    if(msgRemoveConfirm.exec() == QMessageBox::Yes) {
        foreach(const QString &file, filePaths) {
            QFile(file).moveToTrash();
        }
    }
}

void MainWindow::on_actionRemove_permanently_triggered() {
    QStringList filePaths;
    getFileList(filePaths);

    QMessageBox msgRemoveConfirm;
    msgRemoveConfirm.setWindowTitle("Permanent deletion");
    msgRemoveConfirm.setText("Do you want to delete files permanently (" + QString::number(filePaths.length()) + ")?");
    msgRemoveConfirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgRemoveConfirm.setDefaultButton(QMessageBox::Yes);
    msgRemoveConfirm.setIcon(QMessageBox::Question);

    if (msgRemoveConfirm.exec() == QMessageBox::Yes) {
        foreach(const QString &file, filePaths) {
            QFileInfo fileInfo(file);
            if (fileInfo.isWritable()) {
                if (fileInfo.isDir() && !fileInfo.isSymLink())
                    QDir(file).removeRecursively();
                else
                    QFile(file).remove();
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
                    if (fileInfo.isDir())
                        QDir(file).removeRecursively();
                    else
                        fileObject.remove();
                }
            }
        }
    }
}

void MainWindow::on_actionCreate_Folder_triggered() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();

    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();

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
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();

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
    QStringList filePaths;
    getFileList(filePaths);

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
    QStringList filePaths;
    getFileList(filePaths);

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
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    QClipboard *clipboard = QApplication::clipboard();

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();

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
    QString destPath = fsModel->filePath(fsModelIndex);

    if(destInfo.isDir()) {
        if(fsModelIndex.data().toString() == "..")
            destPath.chop(2);

        if(destPath.at(destPath.length() - 1) != '/')
            destPath.append('/');
    }
    else
        destPath = destInfo.absolutePath() + "/";

    // if files was cutted, then copy with removing or move
    if(isCutted) {
        // if storage is the same, then just move file, otherwise copy with removing
        if(QStorageInfo(filePaths[0]).device() == QStorageInfo(destPath).device()) {
            for(QString &file : filePaths) {
                QFileInfo fileInfo = QFileInfo(file);

                // check if moved folder is not parent of destination folder
                if(fileInfo.isDir() && destPath.contains(file)) {
                    QMessageBox::warning(nullptr, "Error", "The source directory is the parent of the target directory!");
                    return;
                }

                // check if moved file/folder is not destination file/folder
                if(file == destPath + fileInfo.fileName()) {
                    QMessageBox::warning(nullptr, "Error", "It is not possible to move file or folder to itself!");
                    return;
                }

                if(QFileInfo::exists(destPath + fileInfo.fileName())) {
                    QMessageBox msgFileExists;
                    msgFileExists.setIcon(QMessageBox::Question);
                    msgFileExists.setWindowTitle("File exists");
                    msgFileExists.setText("File \"" + destPath + fileInfo.fileName() +
                                          "\" already exists! Do you want to overwrite the file?");
                    QPushButton *overwriteButton = msgFileExists.addButton("Overwrite", QMessageBox::YesRole);
                    QPushButton *overwriteAllButton = msgFileExists.addButton("Overwrite All", QMessageBox::YesRole);
                    QPushButton *skipButton = msgFileExists.addButton("Skip", QMessageBox::NoRole);
                    QPushButton *skipAllButton = msgFileExists.addButton("Skip All", QMessageBox::NoRole);
                    msgFileExists.exec();

                    if(msgFileExists.clickedButton() == (QAbstractButton*)overwriteButton) {
                        QFile::remove(destPath + fileInfo.fileName());
                        QFile::rename(file, destPath + fileInfo.fileName());
                    }
                    if(msgFileExists.clickedButton() == (QAbstractButton*)overwriteAllButton) {
                        QStringList rest;
                        for(QList<QString>::iterator it = filePaths.begin() + filePaths.indexOf(file); it != filePaths.end(); ++it)
                            rest.append(*it);
                        for(QString &path : rest) {
                            QFile::remove(destPath + QFileInfo(path).fileName());
                            QFile::rename(path, destPath + QFileInfo(path).fileName());
                        }
                        break;
                    }
                    if(msgFileExists.clickedButton() == (QAbstractButton*)skipButton) {
                        continue;
                    }
                    if(msgFileExists.clickedButton() == (QAbstractButton*)skipAllButton) {
                        QStringList rest;
                        for(QList<QString>::iterator it = filePaths.begin() + filePaths.indexOf(file); it != filePaths.end(); ++it) {
                            if(!QFile::exists(*it))
                                rest.append(*it);
                        }
                        for(QString &path : rest) {
                            QFile::remove(destPath + QFileInfo(path).fileName());
                            QFile::rename(path, destPath + QFileInfo(path).fileName());
                        }
                        break;
                    }
                }
                else {
                    QFile::rename(file, destPath + fileInfo.fileName());
                }

            }
        }
        else {
            fsModel->copyFiles(filePaths, destPath);
            for(QString &file : filePaths) {
                QFileInfo fileInfo = QFileInfo(file);
                if(fileInfo.exists() && QFileInfo::exists(destPath + fileInfo.fileName())) { // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
        fsModel->copyFiles(filePaths, destPath);

    isCutted = false;
    view->clearSelection();
}


// selection menu actions
void MainWindow::on_actionSelect_file_triggered() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    QModelIndex index = view->currentIndex();

    if (view->selectionModel()->isSelected(index)) {
        view->selectionModel()->select(index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    } else {
        view->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

void MainWindow::on_actionSelect_all_triggered() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    view->selectAll();

    QModelIndex dotdot = view->model()->index(0, 0, view->rootIndex());
    if (dotdot.isValid() && dotdot.data() == "..") {
        view->selectionModel()->select(dotdot, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    }
}

void MainWindow::on_actionRemove_selection_triggered() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    view->clearSelection();
}


// tab menu actions
void MainWindow::on_actionCreate_a_new_tab_triggered() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*) sortModel->sourceModel();

    QTabWidget *tabWidget = (QTabWidget*) view->parent()->parent()->parent();
    QString path = fsModel->rootPath();
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
}

void MainWindow::on_actionClose_this_tab_triggered() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    QTabWidget *tabWidget;
    if(qApp->focusWidget()->objectName() == "left")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    if(tabWidget->count() > 1) {
        tabWidget->removeTab(tabWidget->currentIndex());

        tabsUpdate(tabWidget);
        diskStatusUpdate(tabWidget);
    }
    (tabWidget->currentWidget()->findChild<MyTreeView*>())->setFocus();
}

void MainWindow::on_actionClose_all_tabs_triggered() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    QTabWidget *tabWidget;
    if(qApp->focusWidget()->objectName() == "left")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();


    tabWidget->tabBar()->moveTab(tabWidget->currentIndex(), 0);
    int cnt = tabWidget->count();

    for(int i = 0; i < cnt - 1; i++)
        tabWidget->removeTab(1);

    tabsUpdate(tabWidget);
}

void MainWindow::on_actionSwitch_to_the_next_tab_triggered() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

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
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

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
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    QTabWidget *tabWidget;
    if(qApp->focusWidget()->objectName() == "left")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();

    QModelIndex fsModelIndex = sortModel->mapToSource(view->currentIndex());
    QString filePath = fsModel->filePath(fsModelIndex);

    if(QFileInfo(filePath).isFile())
        return;

    QString folderName;
    if(QDir(filePath).isRoot()) {
        folderName = QDir(filePath).absoluteFilePath(filePath);
        folderName.chop(1);
    }
    else
        folderName = QDir(filePath).dirName();

    auto index = tabWidget->addTab(new QLabel(folderName), folderName);
    tabWidget->setCurrentIndex(index);
    createView(tabWidget, filePath);

    tabsUpdate(tabWidget);
}

void MainWindow::on_actionOpen_the_folder_in_the_new_tab_in_another_bar_triggered() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    QTabWidget *tabWidget;
    if(qApp->focusWidget()->objectName() == "right")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();

    QModelIndex fsModelIndex = sortModel->mapToSource(view->currentIndex());
    QString filePath = fsModel->filePath(fsModelIndex);

    if(QFileInfo(filePath).isFile())
        return;

    QString folderName;
    if(QDir(filePath).isRoot()) {
        folderName = QDir(filePath).absoluteFilePath(filePath);
        folderName.chop(1);
    }
    else
        folderName = QDir(filePath).dirName();

    auto index = tabWidget->addTab(new QLabel(folderName), folderName);
    tabWidget->setCurrentIndex(index);
    createView(tabWidget, filePath);

    tabsUpdate(tabWidget);
}


// tools menu actions
void MainWindow::on_actionFile_search_triggered() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();

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

    view = tabWidget->currentWidget()->findChild<MyTreeView*>();
    sortModel = (MySortFilterProxyModel*) view->model();
    fsModel = (MyFileSystemModel*)sortModel->sourceModel();

    fsModelIndex = fsModel->index(file);
    QModelIndex sortModelIndex = sortModel->mapFromSource(fsModelIndex);

    view->setCurrentIndex(sortModelIndex);
    QTimer::singleShot(0, [=]() {
        view->scrollToFile();
    });

    tabsUpdate(tabWidget);
    diskStatusUpdate(tabWidget);
}

void MainWindow::on_actionShow_Hide_hidden_files_triggered() {
    QTabWidget *tabWidget = (QTabWidget*) ui->leftPanel->findChild<QTabWidget*>();
    MyTreeView *view;
    MySortFilterProxyModel *sortModel;
    MyFileSystemModel *fsModel;

    QSettings *settings = new QSettings("./config.ini", QSettings::IniFormat);
    settings->beginGroup("config");

    if(!settings->value("show_hidden").toInt()) {
        for(int i = 0; i < tabWidget->count(); i++) {
            view = tabWidget->widget(i)->findChild<MyTreeView*>();
            sortModel = (MySortFilterProxyModel*) view->model();
            fsModel = (MyFileSystemModel*) sortModel->sourceModel();
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files | QDir::Hidden);
        }
        tabWidget = (QTabWidget*) ui->rightPanel->findChild<QTabWidget*>();
        for(int i = 0; i < tabWidget->count(); i++) {
            view = tabWidget->widget(i)->findChild<MyTreeView*>();
            sortModel = (MySortFilterProxyModel*) view->model();
            fsModel = (MyFileSystemModel*) sortModel->sourceModel();
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files | QDir::Hidden);
        }
        settings->setValue("show_hidden", 1);
    }
    else {
        for(int i = 0; i < tabWidget->count(); i++) {
            view = tabWidget->widget(i)->findChild<MyTreeView*>();
            sortModel = (MySortFilterProxyModel*) view->model();
            fsModel = (MyFileSystemModel*) sortModel->sourceModel();
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files);
        }
        tabWidget = (QTabWidget*) ui->rightPanel->findChild<QTabWidget*>();
        for(int i = 0; i < tabWidget->count(); i++) {
            view = tabWidget->widget(i)->findChild<MyTreeView*>();
            sortModel = (MySortFilterProxyModel*) view->model();
            fsModel = (MyFileSystemModel*) sortModel->sourceModel();
            fsModel->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files);
        }
        settings->setValue("show_hidden", 0);
    }

    settings->endGroup();
}


// bottom buttons actions
void MainWindow::on_editBtn_clicked() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    MySortFilterProxyModel *sortModel = (MySortFilterProxyModel*) view->model();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)sortModel->sourceModel();

    QModelIndex fsModelIndex = sortModel->mapToSource(view->currentIndex());
    QFileInfo file = fsModel->fileInfo(fsModelIndex);

    if(file.isFile() && file.isWritable()) {
        QProcess *process = new QProcess(this);

#ifdef Q_OS_WIN
        // Open with Notepad in Windows
        process->start("C:/Windows/System32/notepad.exe", QStringList() << file.absoluteFilePath());
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
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *sourceView = (MyTreeView*)qApp->focusWidget();

    QStringList filePaths;
    getFileList(filePaths);

    // find target tabwidget, view and models
    QTabWidget *tabWidget;
    if(sourceView->objectName() == "left")
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();

    MyTreeView *targetView = tabWidget->currentWidget()->findChild<MyTreeView*>();
    MySortFilterProxyModel *targetSortModel = (MySortFilterProxyModel*) targetView->model();
    MyFileSystemModel *targetFsModel = (MyFileSystemModel*)targetSortModel->sourceModel();

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
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *sourceView = (MyTreeView*)qApp->focusWidget();

    QStringList filePaths;
    getFileList(filePaths);

    // find target tabwidget, view and models
    QTabWidget *tabWidget;
    if(sourceView->objectName() == "left")
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();

    MyTreeView *targetView = tabWidget->currentWidget()->findChild<MyTreeView*>();
    MySortFilterProxyModel *targetSortModel = (MySortFilterProxyModel*) targetView->model();
    MyFileSystemModel *targetFsModel = (MyFileSystemModel*)targetSortModel->sourceModel();

    QString destPath = targetFsModel->rootPath();
    if(destPath.at(destPath.length() - 1) != '/')
        destPath.append('/');

    QMessageBox msgMoveConfirm;
    msgMoveConfirm.setWindowTitle("Move files");
    msgMoveConfirm.setText("Do you want to move files (" + QString::number(filePaths.length()) + ") to " + destPath + "?");
    msgMoveConfirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgMoveConfirm.setDefaultButton(QMessageBox::Yes);
    msgMoveConfirm.setIcon(QMessageBox::Question);

    if(msgMoveConfirm.exec() == QMessageBox::Yes) {
        for(QString &file : filePaths) {
            QFileInfo fileInfo = QFileInfo(file);

            // check if moved folder is not parent of destination folder
            if(fileInfo.isDir() && destPath.contains(file)) {
                QMessageBox::warning(nullptr, "Error", "The source directory is the parent of the target directory!");
                return;
            }

            // check if moved file/folder is not destination file/folder
            if(file == destPath + fileInfo.fileName()) {
                QMessageBox::warning(nullptr, "Error", "It is not possible to move file or folder to itself!");
                return;
            }

            if(QFileInfo::exists(destPath + fileInfo.fileName())) {
                QMessageBox msgFileExists;
                msgFileExists.setIcon(QMessageBox::Question);
                msgFileExists.setWindowTitle("File exists");
                msgFileExists.setText("File \"" + destPath + fileInfo.fileName() +
                                      "\" already exists! Do you want to overwrite the file?");
                QPushButton *overwriteButton = msgFileExists.addButton("Overwrite", QMessageBox::YesRole);
                QPushButton *overwriteAllButton = msgFileExists.addButton("Overwrite All", QMessageBox::YesRole);
                QPushButton *skipButton = msgFileExists.addButton("Skip", QMessageBox::NoRole);
                QPushButton *skipAllButton = msgFileExists.addButton("Skip All", QMessageBox::NoRole);
                msgFileExists.exec();

                if(msgFileExists.clickedButton() == (QAbstractButton*)overwriteButton) {
                    QFile::remove(destPath + fileInfo.fileName());
                    QFile::rename(file, destPath + fileInfo.fileName());
                }
                if(msgFileExists.clickedButton() == (QAbstractButton*)overwriteAllButton) {
                    QStringList rest;
                    for(QList<QString>::iterator it = filePaths.begin() + filePaths.indexOf(file); it != filePaths.end(); ++it)
                        rest.append(*it);
                    for(QString &path : rest) {
                        QFile::remove(destPath + QFileInfo(path).fileName());
                        QFile::rename(path, destPath + QFileInfo(path).fileName());
                    }
                    break;
                }
                if(msgFileExists.clickedButton() == (QAbstractButton*)skipButton) {
                    continue;
                }
                if(msgFileExists.clickedButton() == (QAbstractButton*)skipAllButton) {
                    QStringList rest;
                    for(QList<QString>::iterator it = filePaths.begin() + filePaths.indexOf(file); it != filePaths.end(); ++it) {
                        if(!QFile::exists(*it))
                            rest.append(*it);
                    }
                    for(QString &path : rest) {
                        QFile::remove(destPath + QFileInfo(path).fileName());
                        QFile::rename(path, destPath + QFileInfo(path).fileName());
                    }
                    break;
                }
            }
            else {
                QFile::rename(file, destPath + fileInfo.fileName());
            }
        }
    }
}

void MainWindow::on_folderBtn_clicked() {
    on_actionCreate_Folder_triggered();
}

void MainWindow::on_deleteBtn_clicked() {
    on_actionRemove_triggered();
}


// up button action
void MainWindow::upBtn_clicked() {
    QPushButton *button = (QPushButton*) sender();
    QTabWidget *tabWidget;

    if(button->objectName() == "left")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    QLineEdit *pathEdit = tabWidget->currentWidget()->findChild<QLineEdit*>();
    QString currentPath = pathEdit->text();

    if(!QDir(currentPath).isRoot()) {
        directoryChange(currentPath + "..");
    }
}



