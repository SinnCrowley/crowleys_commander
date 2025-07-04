#ifndef MYSETTINGSDIALOG_H
#define MYSETTINGSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class MySettingsDialog;
}

class MySettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MySettingsDialog(QWidget *parent = nullptr);
    ~MySettingsDialog();

private slots:
    void on_cancelButton_clicked();
    void on_menuTabs_itemClicked(QListWidgetItem *item);

private:
    Ui::MySettingsDialog *ui;
};

#endif // MYSETTINGSDIALOG_H
