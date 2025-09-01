#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QProgressDialog>
#include "devicewatcher.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QVector<QStack<QString>> historyBackLeft;
    QVector<QStack<QString>> historyForwardLeft;
    QVector<QStack<QString>> historyBackRight;
    QVector<QStack<QString>> historyForwardRight;
    bool isNavTriggered = false;
    bool isCutted = false;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    bool isValidFileName(QString filename);
    QStringList getFileList();
    void createView(QTabWidget *tabBar, QString path);
    void directoryChange(QString path, const QString &position);
    void tabsUpdate(QTabWidget *tabWidget);
    void diskStatusUpdate(QTabWidget *tabWidget);
    void clearLayout(QLayout *layout);
    QString makeUniqueCopyName(const QString &dirPath, const QString &originalName);
    void deviceUpdate();
    void addToHistory(QString path, int currentTab, QString panel);
    void initialize();

private slots:
    // Slots for UI actions
    void diskList_textActivated(const QString &text);
    void diskButton_clicked();
    void pathEdit_returnPressed();
    void view_activated(const QModelIndex &index);
    void tabBar_doubleClicked(int index);
    void tabBar_indexChanged(int index);
    void contextMenu_requested(const QPoint &point);
    void viewHeader_clicked(int localIndex);
    void focusPathEdit();

    // File menu actions
    void on_actionNew_File_triggered();
    void on_actionOpen_selected_file_triggered();
    void on_actionOpen_with_triggered();
    void on_actionRename_triggered();
    void on_actionRemove_triggered();
    void on_actionRemove_permanently_triggered();
    void on_actionCreate_Folder_triggered();
    void on_actionCreate_Shortcut_triggered();
    void on_actionCreate_Archive_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionCopy_as_path_triggered();
    void properties_triggered();

    // Selection menu actions
    void on_actionSelect_file_triggered();
    void on_actionSelect_all_triggered();
    void on_actionRemove_selection_triggered();

    // Tab menu actions
    void on_actionCreate_a_new_tab_triggered();
    void on_actionClose_this_tab_triggered();
    void on_actionClose_all_tabs_triggered();
    void on_actionSwitch_to_the_next_tab_triggered();
    void on_actionSwitch_to_the_previous_tab_triggered();
    void on_actionOpen_the_folder_in_the_new_tab_triggered();
    void on_actionOpen_the_folder_in_the_new_tab_in_another_bar_triggered();

    // Tools menu actions
    void on_actionFile_search_triggered();
    void on_actionShow_Hide_hidden_files_triggered();
    void open_Terminal_triggered();
    void on_actionSettings_triggered();

    // Bottom button actions
    void on_editBtn_clicked();
    void on_copyBtn_clicked();
    void on_moveBtn_clicked();
    void on_folderBtn_clicked();
    void on_deleteBtn_clicked();

    // navigation buttons actions
    void navigate(QString position, const QString direction);
    void navigationButton_clicked();

    // update UI after external folder deletion
    void onRootPathChanged(const QString &newPath, const QString &position);

private:
    Ui::MainWindow *ui;
    DeviceWatcher *deviceWatcher;
};

#endif // MAINWINDOW_H
