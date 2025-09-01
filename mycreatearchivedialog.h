#ifndef MYCREATEARCHIVEDIALOG_H
#define MYCREATEARCHIVEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>

class MyCreateArchiveDialog : public QDialog {
    Q_OBJECT

public:
    MyCreateArchiveDialog(QWidget *parent, const QString &path);

private slots:
    void onFormatChanged(const QString& format);
    void onEncryptionToggled(bool checked);
    void onCreateClicked();

private:
    QLineEdit *nameEdit;
    QComboBox *formatCombo;
    QCheckBox *encryptCheck;
    QLabel *passwordLabel;
    QLineEdit *passwordEdit;
    QString m_path;
    QSize baseSize;
};

#endif // MYCREATEARCHIVEDIALOG_H
