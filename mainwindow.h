#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCoreApplication>
#include <QtGui>
#include "ui_mainwindow.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void createView(QTabWidget *, QString);
    void directoryChange(QString path);

private slots:
    void diskList_currentTextChanged(const QString &arg1);
    void diskButton_clicked();
    void pathEdit_returnPressed();
    void view_activated(const QModelIndex &index);
    void tabBar_doubleClicked(int index);
    void tabBar_indexChanged(int index);
    //void contextMenu_requested(const QPoint &point);
    void contextMenu_requested(QPoint point);

    // menu actions
    void on_actionCreate_a_new_tab_triggered();

    void on_actionRename_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
