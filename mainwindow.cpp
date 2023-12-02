#include "windows.h"
#include "shtypes.h"
#include "shlobj.h"
#include "shobjidl.h"
#include "objbase.h"

#include <QHeaderView>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myfilesystemmodel.h"
#include "mytreeview.h"


// create MyTreeView with Filesystem
void MainWindow::createView(QTabWidget *tabBar, QString path) {
    QVBoxLayout *barLayout = new QVBoxLayout(tabBar->currentWidget());
    barLayout->setSpacing(0);
    barLayout->setContentsMargins(0, 0, 0, 0);

    MyTreeView *view = new MyTreeView(this);
    QLineEdit *pathEdit = new QLineEdit(this);
    pathEdit->setText(path);
    pathEdit->setFocusPolicy(Qt::ClickFocus);


    barLayout->addWidget(pathEdit);
    barLayout->addWidget(view);


    MyFileSystemModel *fsModel = new MyFileSystemModel(this);
    fsModel->setRootPath(path);


    view->setModel(fsModel);
    view->setRootIndex(fsModel->index(path));

    view->setColumnWidth(1, 60);
    view->setColumnWidth(2, 90);
    view->setColumnWidth(3, 100);
    view->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    view->header()->setStretchLastSection(false);
    view->sortByColumn(2, Qt::SortOrder::AscendingOrder);


    //connect(fsModel, SIGNAL(directoryLoaded(QString)), this, SLOT(directoryChanged(QString)));
    connect(view, SIGNAL(activated(QModelIndex)), this, SLOT(view_activated(QModelIndex)));
    //connect(view, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu_requested(const QPoint &)));
    connect(view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_requested(QPoint)));
    connect(pathEdit, SIGNAL(returnPressed()), this, SLOT(pathEdit_returnPressed()));
    connect(ui->diskListLeft, SIGNAL(currentTextChanged(QString)), this, SLOT(diskList_currentTextChanged(QString)));
    connect(ui->diskListRight, SIGNAL(currentTextChanged(QString)), this, SLOT(diskList_currentTextChanged(QString)));


    QString storageSize = QString("Free space: %1 Gb / ").arg(QString::number(QStorageInfo(path).bytesFree() / 1024.00 / 1024 / 1024, 'f', 2)) + QString("%1 Gb").arg(QString::number(QStorageInfo("C:/").bytesTotal() / 1024.00 / 1024 / 1024, 'f', 2));

    if(tabBar->parent()->objectName() == "leftPanel"){
        ui->diskSizeLeft->setText(storageSize);
        ui->diskNameLeft->setText(QStorageInfo(path).displayName());
        pathEdit->setObjectName("left");
        view->setObjectName("left");
    }
    else {
        ui->diskSizeRight->setText(storageSize);
        ui->diskNameRight->setText(QStorageInfo(path).displayName());
        pathEdit->setObjectName("right");
        view->setObjectName("right");
    }

    view->setFocus();
}

// change directory and reload panel
void MainWindow::directoryChange(QString path) {
    QString folderName;
    path = QDir(path).absolutePath();
    QDir *dirInfo = new QDir(path);

    if(dirInfo->isRoot())
        folderName = path.left(path.lastIndexOf('/'));
    else
        folderName = dirInfo->dirName();


    QTabWidget *tabWidget;

    QSettings *settings = new QSettings("./history.ini", QSettings::IniFormat);

    auto storage = QStorageInfo(path);
    QString sizeString =
            QString("Free space: %1 Gb / ").arg(QString::number(storage.bytesFree() / 1024.00 / 1024 / 1024, 'f', 2))
            + QString("%1 Gb").arg(QString::number(storage.bytesTotal() / 1024.00 / 1024 / 1024, 'f', 2));

    if(qApp->focusWidget()->objectName()=="left") {
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();

        ui->diskListLeft->setCurrentText(path[0]);

        ui->diskNameLeft->setText(storage.displayName());
        ui->diskSizeLeft->setText(sizeString);


        settings->beginGroup("left_tabs");
        QString activeTab = settings->value("active_tab").toString();
        settings->setValue(activeTab + "_path", dirInfo->absolutePath());
        settings->endGroup();
    }
    else if(qApp->focusWidget()->objectName()== "right") {
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

        ui->diskListRight->setCurrentText(path[0]);

        ui->diskNameRight->setText(storage.displayName());
        ui->diskSizeRight->setText(sizeString);

        settings->beginGroup("right_tabs");
        QString activeTab = settings->value("active_tab").toString();
        settings->setValue(activeTab + "_path", dirInfo->absolutePath());
        settings->endGroup();
    }
    else {
        settings->destroyed();
        return;
    }

    MyTreeView *view = tabWidget->currentWidget()->findChild<MyTreeView*>();
    QLineEdit *pathEdit = tabWidget->currentWidget()->findChild<QLineEdit*>();
    MyFileSystemModel *fsModel = (MyFileSystemModel*)view->model();

    fsModel->setRootPath(path);
    view->setRootIndex(fsModel->index(path));

    view->setSortingEnabled(true);
    view->sortByColumn(2, Qt::SortOrder::AscendingOrder);


    tabWidget->setTabText(tabWidget->currentIndex(), folderName);

    QString pathText = dirInfo->absolutePath();
    if(pathText[pathText.length()-1] != '/')
        pathText += '/';
    pathEdit->setText(pathText);
    ui->cmdPath->setText(dirInfo->absolutePath() + " > ");

    view->setFocus();
    view->setCurrentIndex(view->indexAt(QPoint(0,0)));

    //view->selectionModel()->select(view->currentIndex(), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    //view->selectionModel()->select(view->indexAt(QPoint(0,0)), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Make a list of mounted drives
    QStringList drives;
    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
        if (storage.isValid() && storage.isReady()) {
            if (!storage.isReadOnly())
                drives.append(storage.rootPath()[0]);
        }
    }

    // Create drives buttons and list
    foreach(const QString drive, drives) {
        // buttons
        QPushButton *leftButton = new QPushButton(this);
        QPushButton *rightButton = new QPushButton(this);
        leftButton->setText(drive);
        leftButton->setObjectName("left");
        rightButton->setText(drive);
        rightButton->setObjectName("right");
        ui->diskButtonsLeftLayout->addWidget(leftButton);
        ui->diskButtonsRightLayout->addWidget(rightButton);

        connect(leftButton, SIGNAL(clicked()), this, SLOT(diskButton_clicked()));
        connect(rightButton, SIGNAL(clicked()), this, SLOT(diskButton_clicked()));

        QIcon hddIcon(QPixmap(":/icons/icons/hdd_96.png"));
        QIcon usbIcon(QPixmap(":/icons/icons/usb_96.png"));
        QIcon cdIcon(QPixmap(":/icons/icons/cdrom_96.png"));
        QIcon networkIcon(QPixmap(":/icons/icons/network_96.png"));

        // get the type of the storage
        if(GetDriveType((const wchar_t *)((drive)+":/").utf16()) == 2) {
            leftButton->setIcon(usbIcon);
            rightButton->setIcon(usbIcon);
            ui->diskListLeft->addItem(usbIcon, drive);
            ui->diskListRight->addItem(usbIcon, drive);
        }
        else if(GetDriveType((const wchar_t *)((drive)+":/").utf16()) == 4) {
            leftButton->setIcon(networkIcon);
            rightButton->setIcon(networkIcon);
            ui->diskListLeft->addItem(networkIcon, drive);
            ui->diskListRight->addItem(networkIcon, drive);
        }
        else if(GetDriveType((const wchar_t *)((drive)+":/").utf16()) == 5) {
            leftButton->setIcon(cdIcon);
            rightButton->setIcon(cdIcon);
            ui->diskListLeft->addItem(cdIcon, drive);
            ui->diskListRight->addItem(cdIcon, drive);
        }
        else {
            leftButton->setIcon(hddIcon);
            rightButton->setIcon(hddIcon);
            ui->diskListLeft->addItem(hddIcon, drive);
            ui->diskListRight->addItem(hddIcon, drive);
        }

        leftButton->setFocusPolicy(Qt::NoFocus);
        rightButton->setFocusPolicy(Qt::NoFocus);
    }

    QTabWidget *leftBar = new QTabWidget;
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


    QTabWidget *rightBar = new QTabWidget;
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

    //connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focusCh(QWidget*,QWidget*)));

    leftBar->setFocusPolicy(Qt::NoFocus);
    rightBar->setFocusPolicy(Qt::NoFocus);
}

MainWindow::~MainWindow() {
    delete ui;
}


// actions
void MainWindow::diskList_currentTextChanged(const QString &arg1) {
    QComboBox *comboBox = (QComboBox*)sender();
    QTabWidget *tabWidget;
    if(comboBox->objectName() == "diskListLeft")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    MyTreeView *view = tabWidget->currentWidget()->findChild<MyTreeView*>();
    view->setFocus();

    directoryChange(arg1 + ":/");
}

void MainWindow::diskButton_clicked() {
    QPushButton *button = (QPushButton*) sender();

    QTabWidget *tabWidget;
    if(button->objectName() == "left")
        tabWidget = ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = ui->rightPanel->findChild<QTabWidget*>();

    MyTreeView *view = tabWidget->currentWidget()->findChild<MyTreeView*>();
    view->setFocus();

    QString path = button->text() + ":/";
    directoryChange(path);
}

void MainWindow::pathEdit_returnPressed() {
    QLineEdit *pathEdit = (QLineEdit*) sender();
    QString inputPath = pathEdit->text();
    if(inputPath[inputPath.size()-1] != *"/")
        inputPath.append("/");

    QTabWidget *tabWidget;
    if(pathEdit->objectName() == "left")
        tabWidget = (QTabWidget*) ui->leftPanel->findChild<QTabWidget*>();
    else
        tabWidget = (QTabWidget*) ui->rightPanel->findChild<QTabWidget*>();

    MyTreeView *view = (MyTreeView*) tabWidget->currentWidget()->findChild<MyTreeView*>();

    pathEdit->clearFocus();
    if(QDir(inputPath).exists()){
        pathEdit->setFocus();
        directoryChange(inputPath);
        view->setFocus();
    }
    else {
        pathEdit->setText(((MyFileSystemModel*)view->model())->rootPath());
        return;
    }
}

void MainWindow::view_activated(const QModelIndex &index) {
    MyFileSystemModel *fsModel = (MyFileSystemModel*) sender();
    QString path;
    path = fsModel->filePath(index) + "/";
    if(fsModel->fileInfo(index).isDir()) {
        if(path[path.size()-1] == QChar('.') && path[path.size()-2] == QChar('.')) {
            path.chop(3);
            path = path.left(path.lastIndexOf(QChar('/'))+1);
        }
        directoryChange(path);
    }
    else {QDesktopServices::openUrl(QUrl::fromLocalFile(fsModel->filePath(index)));}
}

void MainWindow::tabBar_doubleClicked(int index) {
    QTabWidget *tab = (QTabWidget*) sender();
    if(tab->count() > 1) {
        tab->removeTab(index);

        QSettings *settings = new QSettings("./history.ini", QSettings::IniFormat);
        if(tab->parent()->objectName() == "leftPanel")
            settings->beginGroup("left_tabs");
        else
            settings->beginGroup("right_tabs");

        settings->remove(QString::number(index) + "_path");

        // all tabs from index in ini file --
        for(int i = index; i < tab->count(); i++) {
            QString key = settings->value(QString::number(i+1) + "_path").toString();
            settings->remove(QString::number(i+1) + "_path");
            settings->setValue(QString::number(i) + "_path", key);
        }
        settings->setValue("active_tab", tab->currentIndex());
        settings->endGroup();
    }
}

void MainWindow::tabBar_indexChanged(int index){
    QTabWidget *tab = (QTabWidget*) sender();
    QSettings *settings = new QSettings("./history.ini", QSettings::IniFormat);

    if(tab->parent()->objectName() == "leftPanel")
        settings->beginGroup("left_tabs");
    else
        settings->beginGroup("right_tabs");

    settings->setValue("active_tab", index);
    settings->endGroup();
}

//void MainWindow::contextMenu_requested(const QPoint &point) {
void MainWindow::contextMenu_requested(QPoint point) {
    MyTreeView *view = (MyTreeView*) sender();
    MyFileSystemModel *model = (MyFileSystemModel*)view->model();
    QModelIndex index = view->indexAt(point);
    QString filePath = model->filePath(index);


    //openShellContextMenuForObject(file_path.toStdWString(), QCursor::pos().x(), QCursor::pos().y());
    /*file_path.replace("/", "\\");
    wchar_t* pth = (wchar_t*) malloc(sizeof(wchar_t)*file_path.length()+1);
    file_path.toWCharArray(pth);
    pth[file_path.length()] = 0;

    InvokeContextMenu(GetForegroundWindow(), pth);*/
}


// menu actions

void MainWindow::on_actionCreate_a_new_tab_triggered() {
    if(strcmp(qApp->focusWidget()->metaObject()->className(), "MyTreeView") != 0)
        return;

    MyTreeView *view = (MyTreeView*)qApp->focusWidget();
    MyFileSystemModel *model = (MyFileSystemModel*)view->model();
    QTabWidget *tabWidget = (QTabWidget*)view->parent()->parent()->parent();
    QString path = model->rootPath();
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

    QSettings *settings = new QSettings("./history.ini", QSettings::IniFormat);
    if(tabWidget->parent()->objectName() == "leftPanel")
        settings->beginGroup("left_tabs");
    else
        settings->beginGroup("right_tabs");

    settings->setValue(QString::number(tabWidget->currentIndex()) + "_path", path);
    settings->setValue("active_tab", tabWidget->currentIndex());
    settings->endGroup();
}


void MainWindow::on_actionRename_triggered() {
    MyTreeView *view = (MyTreeView*)qApp->focusWidget();

}

