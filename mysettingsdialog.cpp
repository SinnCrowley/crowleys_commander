#include "mysettingsdialog.h"
#include "ui_mysettingsdialog.h"
#include "styletweaks.h"

// class for menu tabs drawing
class ListItemDelegate : public QStyledItemDelegate {
public:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        Q_UNUSED(option);
        Q_UNUSED(index);
        return QSize(100, 50); // ignoring width
    }
};

MySettingsDialog::MySettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MySettingsDialog)
{
    ui->setupUi(this);

    ui->settingsPages->setCurrentIndex(0);
    setStyle(new StyleTweaks);
    ui->menuTabs->setItemDelegate(new ListItemDelegate());
    ui->menuTabs->selectionModel()->setCurrentIndex(ui->menuTabs->model()->index(0, 0, QModelIndex()), QItemSelectionModel::Select);
}

MySettingsDialog::~MySettingsDialog()
{
    delete ui;
}

void MySettingsDialog::on_cancelButton_clicked()
{
    close();
}


void MySettingsDialog::on_menuTabs_itemClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    int index = ui->menuTabs->currentRow();
    ui->settingsPages->setCurrentIndex(index);
}

