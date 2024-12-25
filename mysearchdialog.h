#ifndef MYSEARCHDIALOG_H
#define MYSEARCHDIALOG_H

#include <QDialog>
#include <QListWidget>

namespace Ui {
class MySearchDialog;
}

class MySearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MySearchDialog(QWidget *parent = nullptr);
    MySearchDialog(QString &path);
    ~MySearchDialog();
    QString getFileToShow();
    bool getSuccess();

private slots:
    void on_browseButton_clicked();
    void on_searchButton_clicked();
    void on_result_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MySearchDialog *ui;
    QString fileToShow;
    bool success;
    void findRecursively(const QString &path, const QString &pattern, QStringList *result);
};

#endif // MYSEARCHDIALOG_H
