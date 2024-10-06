/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionNew_File;
    QAction *actionOpen_selected_file;
    QAction *actionOpen_with;
    QAction *actionRename;
    QAction *actionRemove;
    QAction *actionRemove_permanently;
    QAction *actionCreate_Folder;
    QAction *actionCreate_Shortcut;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionSelect_file;
    QAction *actionSelect_all;
    QAction *actionRemove_selection;
    QAction *actionCreate_a_new_tab;
    QAction *actionClose_this_tab;
    QAction *actionClose_all_tabs;
    QAction *actionSwitch_to_the_next_tab;
    QAction *actionSwitch_to_the_previous_tab;
    QAction *actionOpen_the_folder_in_the_new_tab;
    QAction *actionOpen_this_folder_in_the_new_tab_in_another_bar;
    QAction *actionFile_Search;
    QAction *actionShow_Hide_hidden_files;
    QWidget *mainWidget;
    QVBoxLayout *verticalLayout;
    QWidget *mainBar;
    QHBoxLayout *horizontalLayout;
    QWidget *leftPanel;
    QVBoxLayout *leftPanelLayout;
    QWidget *diskButtonsLeft;
    QHBoxLayout *diskButtonsLeftLayout;
    QWidget *statusDiskLeft;
    QHBoxLayout *statusDiskLeftLayout;
    QComboBox *diskListLeft;
    QLabel *diskNameLeft;
    QLabel *diskSizeLeft;
    QWidget *rightPanel;
    QVBoxLayout *rightPanelLayout;
    QWidget *diskButtonsRight;
    QHBoxLayout *diskButtonsRightLayout;
    QWidget *statusDiskRight;
    QHBoxLayout *statusDiskRightLayout;
    QComboBox *diskListRight;
    QLabel *diskNameRight;
    QLabel *diskSizeRight;
    QWidget *bottomButtons;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *editBtn;
    QPushButton *copyBtn;
    QPushButton *moveBtn;
    QPushButton *folderBtn;
    QPushButton *deleteBtn;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuSelection;
    QMenu *menuTabs;
    QMenu *menuTools;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(930, 600);
        MainWindow->setStyleSheet(QString::fromUtf8("QWidget#mainWidget {\n"
"	background: #333;\n"
"	color: #fff;\n"
"}\n"
"\n"
"QPushButton {\n"
"	border: 1px solid #333;\n"
"	background: #222;\n"
"	padding: 5px 7px;\n"
"	border-radius: 3px;\n"
"}\n"
"QPushButton:hover {\n"
"	background: #292929;\n"
"}\n"
"QPushButton:pressed {\n"
"	background: #222;\n"
"}\n"
"\n"
"QComboBox {\n"
"	background: #222;\n"
"	border: 1px solid #444;\n"
"	border-radius: 3px;\n"
"}\n"
"QComboBox:hover {\n"
"	background: #292929;\n"
"}\n"
"QComboBox::drop-down {\n"
"	border: none;\n"
"	background: transparent;\n"
"}\n"
"QComboBox::down-arrow {\n"
"	width: 12px;\n"
"	height: 12px;\n"
"	margin-right: 6px;\n"
"	image: url(:/icons/icons/arrow.png);\n"
"}\n"
"\n"
"QWidget#leftPanel, QWidget#rightPanel {\n"
"	border: 1px solid #444;\n"
"}\n"
"\n"
"QWidget#statusDiskLeft, QWidget#statusDiskRight {\n"
"	border-top: 1px solid #444;\n"
"	border-bottom: 1px solid #444;\n"
"}\n"
"\n"
"QWidget#bottomButtons {\n"
"	border-top: 1px solid #444;\n"
"}\n"
"\n"
"QWidget#mainWidget QLineEdit {\n"
"	backg"
                        "round: #ea4242;\n"
"	padding: 4px 0;\n"
"	border-radius: 0;\n"
"	color: #222;\n"
"}\n"
"\n"
"QWidget#mainWidget QLineEdit:focus {\n"
"	background: #fa5252;\n"
"	color: #000;\n"
"}\n"
"\n"
"QTabBar::tab {\n"
"	background: #222;\n"
"	padding: 6px 10px;\n"
"	border-top-left-radius: 5px;\n"
"	border-top-right-radius: 5px;\n"
"	border: 1px solid #444;\n"
"}\n"
"QTabBar::tab:hover {\n"
"	background: #292929;\n"
"}\n"
"QTabBar::tab:selected {\n"
"	background: #fa5252;\n"
"	color: #000;\n"
"}\n"
"QTabBar::tab:selected:hover {\n"
"	background: #ff5757;\n"
"	color: #000;\n"
"}\n"
"\n"
"QTreeView::item {\n"
"	border-bottom: 1px solid #444;\n"
"	padding: 4px 0;\n"
"}\n"
"QTreeView::item:hover {\n"
"	background: #393939;\n"
"}\n"
"QTreeView::item:selected {\n"
"	color: #fa5252;\n"
"	background: transparent;\n"
"}\n"
"QHeaderView::section {\n"
"	background: #222;\n"
"	border-bottom: 1px solid #555;\n"
"	border-right: 1px solid #444;\n"
"	height: 20px;\n"
"	margin-top: 8px;\n"
"	margin-left: 5px;\n"
"}\n"
"QHeaderView::secti"
                        "on:first {\n"
"	margin-left: -15px;\n"
"}\n"
"QHeaderView::section:hover {\n"
"	background: #292929;\n"
"}\n"
"QHeaderView::down-arrow {\n"
"    image: url(:icons/icons/arrow.png);\n"
"	width: 12px;\n"
"	height: 12px;\n"
"	margin-right: 8px;\n"
"}\n"
"QHeaderView::up-arrow {\n"
"    image: url(:icons/icons/arrow_up.png);\n"
"	width: 12px;\n"
"	height: 12px;\n"
"	margin-right: 8px;\n"
"}"));
        actionNew_File = new QAction(MainWindow);
        actionNew_File->setObjectName("actionNew_File");
        actionOpen_selected_file = new QAction(MainWindow);
        actionOpen_selected_file->setObjectName("actionOpen_selected_file");
        actionOpen_selected_file->setEnabled(true);
        actionOpen_selected_file->setShortcutContext(Qt::ShortcutContext::WidgetShortcut);
        actionOpen_selected_file->setPriority(QAction::Priority::LowPriority);
        actionOpen_with = new QAction(MainWindow);
        actionOpen_with->setObjectName("actionOpen_with");
        actionRename = new QAction(MainWindow);
        actionRename->setObjectName("actionRename");
        actionRemove = new QAction(MainWindow);
        actionRemove->setObjectName("actionRemove");
        actionRemove_permanently = new QAction(MainWindow);
        actionRemove_permanently->setObjectName("actionRemove_permanently");
        actionCreate_Folder = new QAction(MainWindow);
        actionCreate_Folder->setObjectName("actionCreate_Folder");
        actionCreate_Shortcut = new QAction(MainWindow);
        actionCreate_Shortcut->setObjectName("actionCreate_Shortcut");
        actionCut = new QAction(MainWindow);
        actionCut->setObjectName("actionCut");
        actionCopy = new QAction(MainWindow);
        actionCopy->setObjectName("actionCopy");
        actionPaste = new QAction(MainWindow);
        actionPaste->setObjectName("actionPaste");
        actionSelect_file = new QAction(MainWindow);
        actionSelect_file->setObjectName("actionSelect_file");
        actionSelect_all = new QAction(MainWindow);
        actionSelect_all->setObjectName("actionSelect_all");
        actionRemove_selection = new QAction(MainWindow);
        actionRemove_selection->setObjectName("actionRemove_selection");
        actionCreate_a_new_tab = new QAction(MainWindow);
        actionCreate_a_new_tab->setObjectName("actionCreate_a_new_tab");
        actionClose_this_tab = new QAction(MainWindow);
        actionClose_this_tab->setObjectName("actionClose_this_tab");
        actionClose_all_tabs = new QAction(MainWindow);
        actionClose_all_tabs->setObjectName("actionClose_all_tabs");
        actionSwitch_to_the_next_tab = new QAction(MainWindow);
        actionSwitch_to_the_next_tab->setObjectName("actionSwitch_to_the_next_tab");
        actionSwitch_to_the_previous_tab = new QAction(MainWindow);
        actionSwitch_to_the_previous_tab->setObjectName("actionSwitch_to_the_previous_tab");
        actionOpen_the_folder_in_the_new_tab = new QAction(MainWindow);
        actionOpen_the_folder_in_the_new_tab->setObjectName("actionOpen_the_folder_in_the_new_tab");
        actionOpen_this_folder_in_the_new_tab_in_another_bar = new QAction(MainWindow);
        actionOpen_this_folder_in_the_new_tab_in_another_bar->setObjectName("actionOpen_this_folder_in_the_new_tab_in_another_bar");
        actionFile_Search = new QAction(MainWindow);
        actionFile_Search->setObjectName("actionFile_Search");
        actionShow_Hide_hidden_files = new QAction(MainWindow);
        actionShow_Hide_hidden_files->setObjectName("actionShow_Hide_hidden_files");
        mainWidget = new QWidget(MainWindow);
        mainWidget->setObjectName("mainWidget");
        verticalLayout = new QVBoxLayout(mainWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        mainBar = new QWidget(mainWidget);
        mainBar->setObjectName("mainBar");
        horizontalLayout = new QHBoxLayout(mainBar);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        leftPanel = new QWidget(mainBar);
        leftPanel->setObjectName("leftPanel");
        leftPanelLayout = new QVBoxLayout(leftPanel);
        leftPanelLayout->setSpacing(0);
        leftPanelLayout->setObjectName("leftPanelLayout");
        leftPanelLayout->setContentsMargins(0, 0, 0, 0);
        diskButtonsLeft = new QWidget(leftPanel);
        diskButtonsLeft->setObjectName("diskButtonsLeft");
        diskButtonsLeftLayout = new QHBoxLayout(diskButtonsLeft);
        diskButtonsLeftLayout->setSpacing(0);
        diskButtonsLeftLayout->setObjectName("diskButtonsLeftLayout");
        diskButtonsLeftLayout->setContentsMargins(0, 0, 0, 0);

        leftPanelLayout->addWidget(diskButtonsLeft);

        statusDiskLeft = new QWidget(leftPanel);
        statusDiskLeft->setObjectName("statusDiskLeft");
        statusDiskLeftLayout = new QHBoxLayout(statusDiskLeft);
        statusDiskLeftLayout->setSpacing(5);
        statusDiskLeftLayout->setObjectName("statusDiskLeftLayout");
        statusDiskLeftLayout->setContentsMargins(0, 0, 0, 0);
        diskListLeft = new QComboBox(statusDiskLeft);
        diskListLeft->setObjectName("diskListLeft");
        diskListLeft->setMinimumSize(QSize(0, 20));
        diskListLeft->setMaximumSize(QSize(75, 20));
        diskListLeft->setFocusPolicy(Qt::FocusPolicy::NoFocus);

        statusDiskLeftLayout->addWidget(diskListLeft);

        diskNameLeft = new QLabel(statusDiskLeft);
        diskNameLeft->setObjectName("diskNameLeft");

        statusDiskLeftLayout->addWidget(diskNameLeft);

        diskSizeLeft = new QLabel(statusDiskLeft);
        diskSizeLeft->setObjectName("diskSizeLeft");
        diskSizeLeft->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        statusDiskLeftLayout->addWidget(diskSizeLeft);


        leftPanelLayout->addWidget(statusDiskLeft);


        horizontalLayout->addWidget(leftPanel);

        rightPanel = new QWidget(mainBar);
        rightPanel->setObjectName("rightPanel");
        rightPanelLayout = new QVBoxLayout(rightPanel);
        rightPanelLayout->setSpacing(0);
        rightPanelLayout->setObjectName("rightPanelLayout");
        rightPanelLayout->setContentsMargins(0, 0, 0, 0);
        diskButtonsRight = new QWidget(rightPanel);
        diskButtonsRight->setObjectName("diskButtonsRight");
        diskButtonsRightLayout = new QHBoxLayout(diskButtonsRight);
        diskButtonsRightLayout->setSpacing(0);
        diskButtonsRightLayout->setObjectName("diskButtonsRightLayout");
        diskButtonsRightLayout->setContentsMargins(0, 0, 0, 0);

        rightPanelLayout->addWidget(diskButtonsRight);

        statusDiskRight = new QWidget(rightPanel);
        statusDiskRight->setObjectName("statusDiskRight");
        statusDiskRight->setMaximumSize(QSize(16777215, 16777215));
        statusDiskRightLayout = new QHBoxLayout(statusDiskRight);
        statusDiskRightLayout->setSpacing(5);
        statusDiskRightLayout->setObjectName("statusDiskRightLayout");
        statusDiskRightLayout->setContentsMargins(0, 0, 0, 0);
        diskListRight = new QComboBox(statusDiskRight);
        diskListRight->setObjectName("diskListRight");
        diskListRight->setMinimumSize(QSize(0, 20));
        diskListRight->setMaximumSize(QSize(75, 20));
        diskListRight->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

        statusDiskRightLayout->addWidget(diskListRight);

        diskNameRight = new QLabel(statusDiskRight);
        diskNameRight->setObjectName("diskNameRight");

        statusDiskRightLayout->addWidget(diskNameRight);

        diskSizeRight = new QLabel(statusDiskRight);
        diskSizeRight->setObjectName("diskSizeRight");
        diskSizeRight->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        statusDiskRightLayout->addWidget(diskSizeRight);


        rightPanelLayout->addWidget(statusDiskRight);


        horizontalLayout->addWidget(rightPanel);


        verticalLayout->addWidget(mainBar);

        bottomButtons = new QWidget(mainWidget);
        bottomButtons->setObjectName("bottomButtons");
        bottomButtons->setMaximumSize(QSize(16777215, 40));
        horizontalLayout_2 = new QHBoxLayout(bottomButtons);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setSizeConstraint(QLayout::SizeConstraint::SetDefaultConstraint);
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        editBtn = new QPushButton(bottomButtons);
        editBtn->setObjectName("editBtn");
        editBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);

        horizontalLayout_2->addWidget(editBtn);

        copyBtn = new QPushButton(bottomButtons);
        copyBtn->setObjectName("copyBtn");
        copyBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);

        horizontalLayout_2->addWidget(copyBtn);

        moveBtn = new QPushButton(bottomButtons);
        moveBtn->setObjectName("moveBtn");
        moveBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);

        horizontalLayout_2->addWidget(moveBtn);

        folderBtn = new QPushButton(bottomButtons);
        folderBtn->setObjectName("folderBtn");
        folderBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);

        horizontalLayout_2->addWidget(folderBtn);

        deleteBtn = new QPushButton(bottomButtons);
        deleteBtn->setObjectName("deleteBtn");
        deleteBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);

        horizontalLayout_2->addWidget(deleteBtn);


        verticalLayout->addWidget(bottomButtons);

        MainWindow->setCentralWidget(mainWidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 930, 25));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuSelection = new QMenu(menubar);
        menuSelection->setObjectName("menuSelection");
        menuTabs = new QMenu(menubar);
        menuTabs->setObjectName("menuTabs");
        menuTools = new QMenu(menubar);
        menuTools->setObjectName("menuTools");
        MainWindow->setMenuBar(menubar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuSelection->menuAction());
        menubar->addAction(menuTabs->menuAction());
        menubar->addAction(menuTools->menuAction());
        menuFile->addSeparator();
        menuFile->addAction(actionNew_File);
        menuFile->addAction(actionOpen_selected_file);
        menuFile->addSeparator();
        menuFile->addAction(actionOpen_with);
        menuFile->addAction(actionRename);
        menuFile->addAction(actionRemove);
        menuFile->addSeparator();
        menuFile->addAction(actionRemove_permanently);
        menuFile->addAction(actionCreate_Folder);
        menuFile->addAction(actionCreate_Shortcut);
        menuFile->addSeparator();
        menuFile->addAction(actionCut);
        menuFile->addAction(actionCopy);
        menuFile->addAction(actionPaste);
        menuSelection->addAction(actionSelect_file);
        menuSelection->addAction(actionSelect_all);
        menuSelection->addAction(actionRemove_selection);
        menuTabs->addAction(actionCreate_a_new_tab);
        menuTabs->addAction(actionClose_this_tab);
        menuTabs->addSeparator();
        menuTabs->addAction(actionClose_all_tabs);
        menuTabs->addAction(actionSwitch_to_the_next_tab);
        menuTabs->addSeparator();
        menuTabs->addAction(actionSwitch_to_the_previous_tab);
        menuTabs->addAction(actionOpen_the_folder_in_the_new_tab);
        menuTabs->addAction(actionOpen_this_folder_in_the_new_tab_in_another_bar);
        menuTools->addAction(actionFile_Search);
        menuTools->addAction(actionShow_Hide_hidden_files);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Crowley's Commander", nullptr));
        actionNew_File->setText(QCoreApplication::translate("MainWindow", "New File", nullptr));
#if QT_CONFIG(shortcut)
        actionNew_File->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionOpen_selected_file->setText(QCoreApplication::translate("MainWindow", "Open selected file", nullptr));
#if QT_CONFIG(shortcut)
        actionOpen_selected_file->setShortcut(QCoreApplication::translate("MainWindow", "Return", nullptr));
#endif // QT_CONFIG(shortcut)
        actionOpen_with->setText(QCoreApplication::translate("MainWindow", "Open with", nullptr));
#if QT_CONFIG(shortcut)
        actionOpen_with->setShortcut(QCoreApplication::translate("MainWindow", "Shift+Return", nullptr));
#endif // QT_CONFIG(shortcut)
        actionRename->setText(QCoreApplication::translate("MainWindow", "Rename", nullptr));
#if QT_CONFIG(shortcut)
        actionRename->setShortcut(QCoreApplication::translate("MainWindow", "F2", nullptr));
#endif // QT_CONFIG(shortcut)
        actionRemove->setText(QCoreApplication::translate("MainWindow", "Remove", nullptr));
#if QT_CONFIG(shortcut)
        actionRemove->setShortcut(QCoreApplication::translate("MainWindow", "Del", nullptr));
#endif // QT_CONFIG(shortcut)
        actionRemove_permanently->setText(QCoreApplication::translate("MainWindow", "Remove permanently", nullptr));
#if QT_CONFIG(shortcut)
        actionRemove_permanently->setShortcut(QCoreApplication::translate("MainWindow", "Shift+Del", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCreate_Folder->setText(QCoreApplication::translate("MainWindow", "Create Folder", nullptr));
#if QT_CONFIG(shortcut)
        actionCreate_Folder->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCreate_Shortcut->setText(QCoreApplication::translate("MainWindow", "Create Shortcut", nullptr));
#if QT_CONFIG(shortcut)
        actionCreate_Shortcut->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+L", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCut->setText(QCoreApplication::translate("MainWindow", "Cut", nullptr));
#if QT_CONFIG(shortcut)
        actionCut->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+X", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCopy->setText(QCoreApplication::translate("MainWindow", "Copy", nullptr));
#if QT_CONFIG(shortcut)
        actionCopy->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+C", nullptr));
#endif // QT_CONFIG(shortcut)
        actionPaste->setText(QCoreApplication::translate("MainWindow", "Paste", nullptr));
#if QT_CONFIG(shortcut)
        actionPaste->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+V", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSelect_file->setText(QCoreApplication::translate("MainWindow", "Select file", nullptr));
#if QT_CONFIG(shortcut)
        actionSelect_file->setShortcut(QCoreApplication::translate("MainWindow", "Space", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSelect_all->setText(QCoreApplication::translate("MainWindow", "Select all", nullptr));
#if QT_CONFIG(shortcut)
        actionSelect_all->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+A", nullptr));
#endif // QT_CONFIG(shortcut)
        actionRemove_selection->setText(QCoreApplication::translate("MainWindow", "Remove selection", nullptr));
#if QT_CONFIG(shortcut)
        actionRemove_selection->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+D", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCreate_a_new_tab->setText(QCoreApplication::translate("MainWindow", "Create a new tab", nullptr));
#if QT_CONFIG(shortcut)
        actionCreate_a_new_tab->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+T", nullptr));
#endif // QT_CONFIG(shortcut)
        actionClose_this_tab->setText(QCoreApplication::translate("MainWindow", "Close this tab", nullptr));
#if QT_CONFIG(shortcut)
        actionClose_this_tab->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+W", nullptr));
#endif // QT_CONFIG(shortcut)
        actionClose_all_tabs->setText(QCoreApplication::translate("MainWindow", "Close all tabs", nullptr));
#if QT_CONFIG(shortcut)
        actionClose_all_tabs->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+W", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSwitch_to_the_next_tab->setText(QCoreApplication::translate("MainWindow", "Switch to the next tab", nullptr));
#if QT_CONFIG(shortcut)
        actionSwitch_to_the_next_tab->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Right", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSwitch_to_the_previous_tab->setText(QCoreApplication::translate("MainWindow", "Switch to the previous tab", nullptr));
#if QT_CONFIG(shortcut)
        actionSwitch_to_the_previous_tab->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Left", nullptr));
#endif // QT_CONFIG(shortcut)
        actionOpen_the_folder_in_the_new_tab->setText(QCoreApplication::translate("MainWindow", "Open the folder in the new tab", nullptr));
#if QT_CONFIG(shortcut)
        actionOpen_the_folder_in_the_new_tab->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Up", nullptr));
#endif // QT_CONFIG(shortcut)
        actionOpen_this_folder_in_the_new_tab_in_another_bar->setText(QCoreApplication::translate("MainWindow", "Open the folder in the new tab in another bar", nullptr));
#if QT_CONFIG(shortcut)
        actionOpen_this_folder_in_the_new_tab_in_another_bar->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+Up", nullptr));
#endif // QT_CONFIG(shortcut)
        actionFile_Search->setText(QCoreApplication::translate("MainWindow", "File search", nullptr));
#if QT_CONFIG(shortcut)
        actionFile_Search->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+F", nullptr));
#endif // QT_CONFIG(shortcut)
        actionShow_Hide_hidden_files->setText(QCoreApplication::translate("MainWindow", "Show/Hide hidden files", nullptr));
#if QT_CONFIG(shortcut)
        actionShow_Hide_hidden_files->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+H", nullptr));
#endif // QT_CONFIG(shortcut)
        diskNameLeft->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        diskSizeLeft->setText(QCoreApplication::translate("MainWindow", "diskSizeLeft", nullptr));
        diskNameRight->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        diskSizeRight->setText(QCoreApplication::translate("MainWindow", "diskSizeLeft", nullptr));
        editBtn->setText(QCoreApplication::translate("MainWindow", "F4 Edit", nullptr));
        copyBtn->setText(QCoreApplication::translate("MainWindow", "F5 Copy", nullptr));
        moveBtn->setText(QCoreApplication::translate("MainWindow", "F6 Move", nullptr));
        folderBtn->setText(QCoreApplication::translate("MainWindow", "F7 New Folder", nullptr));
        deleteBtn->setText(QCoreApplication::translate("MainWindow", "F8 Delete", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuSelection->setTitle(QCoreApplication::translate("MainWindow", "Selection", nullptr));
        menuTabs->setTitle(QCoreApplication::translate("MainWindow", "Tabs", nullptr));
        menuTools->setTitle(QCoreApplication::translate("MainWindow", "Tools", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
