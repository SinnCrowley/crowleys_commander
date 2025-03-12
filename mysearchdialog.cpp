#include <QFileDialog>
#include <QDirIterator>
#include <QMessageBox>

#include "mysearchdialog.h"
#include "ui_mysearchdialog.h"


MySearchDialog::MySearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MySearchDialog)
{
    ui->setupUi(this);
}

MySearchDialog::MySearchDialog(QString &path) :
    QDialog(nullptr),
    ui(new Ui::MySearchDialog)
{
    ui->setupUi(this);

    ui->pathEdit->setText(path);
    ui->foundLabel->setText("");
}

MySearchDialog::~MySearchDialog()
{
    delete ui;
}


QString MySearchDialog::getFileToShow()
{
    return fileToShow;
}

bool MySearchDialog::getSuccess()
{
    return success;
}

void MySearchDialog::findRecursively(const QString &path, const QString &pattern, QStringList *result)
{
    QDir currentDir(path);
    QString prefix = path;

    if (path.at(path.length() - 1) != '/')
        prefix.append('/');

    foreach (const QString &match, currentDir.entryList(QStringList(pattern), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot))
        result->append(prefix + match);
    foreach (const QString &dir, currentDir.entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot))
        findRecursively(prefix + dir, pattern, result);
}

void MySearchDialog::on_browseButton_clicked()
{
    QFileDialog *fileDialog = new QFileDialog();

    fileDialog->setModal(true);
    fileDialog->setFileMode(QFileDialog::Directory);
    fileDialog->setOption(QFileDialog::ShowDirsOnly);

    if (QFileInfo::exists(ui->pathEdit->text()) && QFileInfo(ui->pathEdit->text()).isDir())
        fileDialog->setDirectory(ui->pathEdit->text());

    if (fileDialog->exec())
        ui->pathEdit->setText(fileDialog->selectedFiles().at(0));
}

void MySearchDialog::on_searchButton_clicked()
{
    ui->foundLabel->setText("");
    ui->result->clear();

    QStringList found;
    QString fileName = ui->nameEdit->text();
    QString path = ui->pathEdit->text();

    QDir currentDir = QDir(path);

    if (currentDir.exists()) {
        findRecursively(path, fileName.isEmpty() ? QStringLiteral("*") : fileName, &found);

        found.sort();

        foreach (const QString file, found)
            ui->result->addItem(file);

        if(found.size() == 0)
            ui->foundLabel->setText("Files were not found!");
        else
            ui->foundLabel->setText(QString::number(ui->result->count()) + " file(s) found. (Double click on a file to show it in filesystem)");

    } else {
        QMessageBox::warning(nullptr, "Error", "Invalid file path.");
    }
}

void MySearchDialog::on_result_itemDoubleClicked(QListWidgetItem *item)
{
    fileToShow = item->text();
    success = true;
    close();
}

