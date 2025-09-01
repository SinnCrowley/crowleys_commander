#include "mycreatearchivedialog.h"

#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

#include "archivemanager.h"

MyCreateArchiveDialog::MyCreateArchiveDialog(QWidget *parent, const QString &path)
    : QDialog(parent), m_path(path)
{
    setWindowTitle("Create Archive");
    setModal(true);

    baseSize = QSize(400, 130);
    setFixedSize(baseSize);

    QHBoxLayout *labelsLayout = new QHBoxLayout;
    QLabel *nameLabel = new QLabel("Archive name:");
    QLabel *typeLabel = new QLabel("Type:");
    labelsLayout->addWidget(nameLabel);
    labelsLayout->addStretch();
    labelsLayout->addWidget(typeLabel);

    QHBoxLayout *editsLayout = new QHBoxLayout;
    nameEdit = new QLineEdit;
    formatCombo = new QComboBox;

    QStringList formats = {
        "zip", "7z",
        "tar", "tar.gz", "tar.xz", "tar.bz2", "tar.zst",
        "gz", "xz", "bz2", "zst",
        "lzma", "lzip",
        "iso", "xar", "cpio", "ar", "warc", "mtree"
    };
    formatCombo->addItems(formats);
    formatCombo->setCurrentIndex(0);
    formatCombo->setFixedSize(80, 25);

    editsLayout->addWidget(nameEdit);
    editsLayout->addWidget(formatCombo);

    // encryption
    encryptCheck = new QCheckBox("Enable encryption");

    passwordLabel = new QLabel("Password:");
    passwordLabel->setVisible(false);
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setVisible(false);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    QPushButton *createButton = new QPushButton("Create");
    buttonBox->addButton(createButton, QDialogButtonBox::AcceptRole);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(labelsLayout);
    mainLayout->addLayout(editsLayout);
    mainLayout->addWidget(encryptCheck);
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(passwordEdit);
    mainLayout->addStretch();
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    connect(formatCombo, &QComboBox::currentTextChanged, this, &MyCreateArchiveDialog::onFormatChanged);
    connect(encryptCheck, &QCheckBox::toggled, this, &MyCreateArchiveDialog::onEncryptionToggled);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(createButton, &QPushButton::clicked, this, &MyCreateArchiveDialog::onCreateClicked);
}

void MyCreateArchiveDialog::onFormatChanged(const QString& format) {
    if (format == "zip" || format == "7z") {
        encryptCheck->setVisible(true);
    } else {
        encryptCheck->setVisible(false);
        passwordLabel->setVisible(false);
        passwordEdit->setVisible(false);
        encryptCheck->setChecked(false);

        setFixedSize(baseSize);
    }
}

void MyCreateArchiveDialog::onEncryptionToggled(bool checked) {
    passwordLabel->setVisible(checked);
    passwordEdit->setVisible(checked);
    adjustSize();

    if (checked) {
        QSize newSize = baseSize;
        newSize.setHeight(baseSize.height() + 50);
        setFixedSize(newSize);
    } else {
        setFixedSize(baseSize);
    }
}

void MyCreateArchiveDialog::onCreateClicked() {
    QString name = nameEdit->text();
    QString format = formatCombo->currentText();
    QString password;

    if (encryptCheck->isChecked()) {
        password = passwordEdit->text();
    }

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Error", "Archive name cannot be empty.");
        return;
    }

    ArchiveManager manager;
    QString fullPath =  m_path + '/' + name + '.' + format;
    manager.createArchive(fullPath, QStringList(), password, format);

    accept();
}
