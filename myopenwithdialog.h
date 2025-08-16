#ifndef MYOPENWITHDIALOG_H
#define MYOPENWITHDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QUrl>

#ifdef Q_OS_MACOS
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

struct AppInfo {
    QString name;
    QString path;
    QString command;
    QString comment; // app description
    QIcon icon;
    bool isDefault;
};

class MyOpenWithDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MyOpenWithDialog(const QString &filePath, QWidget *parent = nullptr);

    static bool openWith(const QString &filePath, const QString &appPath);
    static void show(const QString &filePath, QWidget *parent = nullptr);

private slots:
    void onOpenClicked();
    void onBrowseClicked();
    void onItemDoubleClicked(QListWidgetItem *item);

private:
    void populateApplications();
    QList<AppInfo> getAvailableApplications();
    QList<AppInfo> getApplicationsForMimeType(const QString &mimeType);
    QIcon getApplicationIcon(const QString &appPath);

    QString m_filePath;
    QListWidget *m_appListWidget;
    QPushButton *m_openButton;
    QPushButton *m_browseButton;
    QPushButton *m_cancelButton;
    QLineEdit *m_customAppEdit;
};

#endif // MYOPENWITHDIALOG_H
