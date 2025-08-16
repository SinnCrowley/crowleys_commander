#include "myopenwithdialog.h"

#include <QSettings>
#include <QDebug>
#include <QPainter>

MyOpenWithDialog::MyOpenWithDialog(const QString &filePath, QWidget *parent)
    : QDialog(parent), m_filePath(filePath)
{
    setWindowTitle(tr("Open with..."));
    setModal(true);
    resize(500, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QFileInfo fileInfo(m_filePath);
    QLabel *fileLabel = new QLabel(tr("Choose application to open the file:\n%1")
                                       .arg(fileInfo.fileName()));
    fileLabel->setWordWrap(true);
    mainLayout->addWidget(fileLabel);

    m_appListWidget = new QListWidget;
    m_appListWidget->setIconSize(QSize(32, 32));
    m_appListWidget->setUniformItemSizes(true);
    mainLayout->addWidget(m_appListWidget);

    QHBoxLayout *customLayout = new QHBoxLayout;
    QLabel *customLabel = new QLabel(tr("Other applications:"));
    m_customAppEdit = new QLineEdit;
    m_browseButton = new QPushButton(tr("Browse..."));

    customLayout->addWidget(customLabel);
    customLayout->addWidget(m_customAppEdit);
    customLayout->addWidget(m_browseButton);
    mainLayout->addLayout(customLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_openButton = new QPushButton(tr("Open"));
    m_cancelButton = new QPushButton(tr("Cancel"));

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_openButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_openButton, &QPushButton::clicked, this, &MyOpenWithDialog::onOpenClicked);
    connect(m_browseButton, &QPushButton::clicked, this, &MyOpenWithDialog::onBrowseClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_appListWidget, &QListWidget::itemDoubleClicked,
            this, &MyOpenWithDialog::onItemDoubleClicked);

    m_openButton->setDefault(true);


    populateApplications();
}

void MyOpenWithDialog::populateApplications()
{
    QList<AppInfo> apps = getAvailableApplications();

    foreach (const AppInfo &app, apps) {
        QPixmap originalPixmap = app.icon.pixmap(QSize(128,128));
        QPixmap scaledPixmap = originalPixmap.scaled(QSize(32, 32), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QListWidgetItem *item = new QListWidgetItem(QIcon(scaledPixmap), app.name);
        item->setData(Qt::UserRole, app.path);
        item->setData(Qt::UserRole + 1, app.command);

        // Create tooltip
        QString tooltip = app.name;
        if (!app.comment.isEmpty()) {
            tooltip += "\n" + app.comment;
        }
        tooltip += "\n" + tr("Path: ") + app.path;

        item->setToolTip(tooltip);

        if (app.isDefault) {
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            item->setText(app.name + tr(" (default)"));
        }

        m_appListWidget->addItem(item);
    }

    if (m_appListWidget->count() > 0) {
        m_appListWidget->setCurrentRow(0);
    }
}

QList<AppInfo> MyOpenWithDialog::getAvailableApplications()
{
    QList<AppInfo> apps;
    QMimeDatabase mimeDb;
    QFileInfo fileInfo(m_filePath);
    QMimeType mimeType = mimeDb.mimeTypeForFile(fileInfo);

    // get applications for current MIME type
    apps = getApplicationsForMimeType(mimeType.name());

#ifdef Q_OS_MACOS
    // On macOS look for apps in /Applications and ~/Applications
    QStringList appDirs = {
        "/Applications",                           // Пользовательские приложения
        QDir::homePath() + "/Applications",        // Локальные пользовательские приложения
        "/System/Applications",                    // Системные приложения (macOS 10.15+)
        "/System/Applications/Utilities",          // Системные утилиты
        "/System/Library/CoreServices/Applications", // Основные системные приложения
        "/Library/CoreServices",                   // Службы системы (Finder, etc.)
        "/usr/local/Applications",                 // Приложения установленные через Homebrew и др.
        "/opt/homebrew/Applications"               // Homebrew на Apple Silicon
    };

    QSet<QString> processedApps; // избегаем дубликатов

    foreach (const QString &appsDir, appDirs) {
        QDir dir(appsDir);
        if (!dir.exists()) {
            qDebug() << "Directory doesn't exist:" << appsDir;
            continue;
        }

        qDebug() << "Searching in:" << appsDir;
        QStringList appBundles = dir.entryList(QStringList() << "*.app", QDir::Dirs);

        foreach (const QString &bundle, appBundles) {
            QString fullPath = QString("%1/%2").arg(appsDir, bundle);

            // Избегаем дубликатов по имени приложения
            QString appName = bundle.left(bundle.length() - 4); // убираем .app
            if (processedApps.contains(appName.toLower())) {
                continue;
            }

            // Проверяем, что это действительно приложение (есть исполняемый файл)
            QString executablePath = fullPath + "/Contents/MacOS/" + appName;
            if (!QFile::exists(executablePath)) {
                // Пробуем найти исполняемый файл по Info.plist
                QString plistPath = fullPath + "/Contents/Info.plist";
                if (QFile::exists(plistPath)) {
                    QProcess plistBuddy;
                    plistBuddy.start("defaults", QStringList() << "read" << plistPath << "CFBundleExecutable");
                    plistBuddy.waitForFinished();
                    QString execName = plistBuddy.readAllStandardOutput().trimmed();

                    if (!execName.isEmpty()) {
                        executablePath = fullPath + "/Contents/MacOS/" + execName;
                    }
                }

                // Если исполняемый файл не найден, пропускаем
                if (!QFile::exists(executablePath)) {
                    qDebug() << "Executable not found for:" << fullPath;
                    continue;
                }
            }

            // Читаем Info.plist для получения правильного имени
            QString plistPath = fullPath + "/Contents/Info.plist";
            QString displayName = appName;

            if (QFile::exists(plistPath)) {
                // CFBundleDisplayName
                QProcess plistBuddy;
                plistBuddy.start("defaults", QStringList() << "read" << plistPath << "CFBundleDisplayName");
                plistBuddy.waitForFinished();
                QString bundleDisplayName = plistBuddy.readAllStandardOutput().trimmed();

                if (bundleDisplayName.isEmpty() || bundleDisplayName.contains("does not exist")) {
                    // CFBundleName
                    plistBuddy.start("defaults", QStringList() << "read" << plistPath << "CFBundleName");
                    plistBuddy.waitForFinished();
                    bundleDisplayName = plistBuddy.readAllStandardOutput().trimmed();
                }

                if (!bundleDisplayName.isEmpty() && !bundleDisplayName.contains("does not exist")) {
                    displayName = bundleDisplayName;
                }

                // Проверяем, что это не скрытое приложение
                plistBuddy.start("defaults", QStringList() << "read" << plistPath << "LSUIElement");
                plistBuddy.waitForFinished();
                QString isUIElement = plistBuddy.readAllStandardOutput().trimmed();

                if (isUIElement == "1") {
                    qDebug() << "Skipping UI element:" << displayName;
                    continue; // Пропускаем фоновые приложения
                }
            }

            processedApps.insert(appName.toLower());

            AppInfo appInfo;
            appInfo.name = displayName;
            appInfo.path = fullPath;
            appInfo.command = QString("open -a \"%1\"").arg(fullPath);
            appInfo.icon = getApplicationIcon(fullPath);
            appInfo.isDefault = false;

            qDebug() << "Added app:" << displayName << "at" << fullPath;

            apps.append(appInfo);
        }
    }

    // Дополнительно добавляем некоторые важные системные приложения вручную
    QStringList importantSystemApps = {
        "TextEdit", "Preview", "QuickTime Player", "Calculator",
        "Terminal", "Console", "Activity Monitor", "Disk Utility",
        "System Information", "Archive Utility"
    };

    foreach (const QString &appName, importantSystemApps) {
        if (!processedApps.contains(appName.toLower())) {
            // Пытаемся найти приложение через системный поиск
            QProcess findProcess;
            findProcess.start("mdfind", QStringList() << QString("kMDItemDisplayName == '%1' && kMDItemContentType == 'com.apple.application-bundle'").arg(appName));
            findProcess.waitForFinished(3000);

            QString foundPaths = findProcess.readAllStandardOutput().trimmed();
            QStringList paths = foundPaths.split('\n', Qt::SkipEmptyParts);

            if (!paths.isEmpty()) {
                QString appPath = paths.first();
                if (QFile::exists(appPath) && !processedApps.contains(appName.toLower())) {
                    AppInfo appInfo;
                    appInfo.name = appName;
                    appInfo.path = appPath;
                    appInfo.command = QString("open -a \"%1\"").arg(appPath);
                    appInfo.icon = getApplicationIcon(appPath);
                    appInfo.isDefault = false;

                    apps.append(appInfo);
                    processedApps.insert(appName.toLower());
                    qDebug() << "Found system app via mdfind:" << appName << "at" << appPath;
                }
            }
        }
    }

    // Сортируем по имени
    std::sort(apps.begin(), apps.end(), [](const AppInfo &a, const AppInfo &b) {
        return a.name < b.name;
    });

#else // Linux
    QStringList desktopDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    desktopDirs << "/usr/share/applications"
                << "/usr/local/share/applications"
                << QDir::homePath() + "/.local/share/applications";

    QSet<QString> processedApps; // avoid duplicities

    foreach (const QString &dir, desktopDirs) {
        QDir desktopDir(dir);
        if (!desktopDir.exists()) continue;

        QStringList desktopFiles = desktopDir.entryList(QStringList() << "*.desktop", QDir::Files);

        foreach (const QString &desktopFile, desktopFiles) {
            QString fullPath = desktopDir.absoluteFilePath(desktopFile);

            QSettings desktop(fullPath, QSettings::IniFormat);
            desktop.beginGroup("Desktop Entry");

            QString name = desktop.value("Name").toString();
            QString genericName = desktop.value("GenericName").toString();
            QString comment = desktop.value("Comment").toString();
            QString exec = desktop.value("Exec").toString();
            QString icon = desktop.value("Icon").toString();
            QString type = desktop.value("Type").toString();
            bool noDisplay = desktop.value("NoDisplay", false).toBool();
            bool hidden = desktop.value("Hidden", false).toBool();

            if (name.isEmpty() || exec.isEmpty() || noDisplay || hidden || type != "Application") {
                continue;
            }

            // Remove parameters from start command
            QString cleanExec = exec;
            cleanExec.remove("%[fFuU]"); // remove %f, %F, %u, %U
            cleanExec = cleanExec.split(' ').first().trimmed();

            // Checking that executable file exists
            QString execPath = cleanExec;
            if (!execPath.startsWith('/')) {
                // Looking in PATH
                execPath = QStandardPaths::findExecutable(cleanExec);
                if (execPath.isEmpty()) continue;
            }

            // remove duplicities
            if (processedApps.contains(execPath)) continue;
            processedApps.insert(execPath);

            AppInfo appInfo;
            appInfo.name = name;
            if (!genericName.isEmpty() && genericName != name) {
                appInfo.name += QString(" (%1)").arg(genericName);
            }
            appInfo.path = fullPath;
            appInfo.command = cleanExec;
            appInfo.icon = getApplicationIcon(icon);
            appInfo.isDefault = false;

            // additional info in tooltip
            appInfo.comment = comment;

            apps.append(appInfo);
        }
    }

    // sort by name alphabetically
    std::sort(apps.begin(), apps.end(), [](const AppInfo &a, const AppInfo &b) {
        return a.name < b.name;
    });
#endif

    return apps;
}

QList<AppInfo> MyOpenWithDialog::getApplicationsForMimeType(const QString &mimeType)
{
    QList<AppInfo> apps;

    // xdg-mime on Linux
    QProcess process;
    process.start("xdg-mime", QStringList() << "query" << "default" << mimeType);
    process.waitForFinished();

    QString defaultApp = process.readAllStandardOutput().trimmed();
    if (!defaultApp.isEmpty()) {
        QString desktopPath = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, defaultApp);
        if (!desktopPath.isEmpty()) {
            QSettings desktop(desktopPath, QSettings::IniFormat);
            desktop.beginGroup("Desktop Entry");

            AppInfo appInfo;
            appInfo.name = desktop.value("Name").toString();
            appInfo.path = desktopPath;
            appInfo.command = desktop.value("Exec").toString().split(' ').first();
            appInfo.icon = getApplicationIcon(desktop.value("Icon").toString());
            appInfo.isDefault = true;
            apps.append(appInfo);
        }
    }

    return apps;
}

QIcon MyOpenWithDialog::getApplicationIcon(const QString &iconPath)
{
    QIcon icon;

    if (iconPath.isEmpty()) {
        icon = QIcon(":/icons/application.png");
        return icon;
    }

#ifdef Q_OS_MACOS
    // Get icon on macOS via NSWorkspace
    if (iconPath.endsWith(".app")) {
        // ИUse system API for getting app icon
        QString scriptPath = QString("osascript -e 'tell application \"Finder\" to get POSIX path of (application file id (id of application \"%1\"))'").arg(iconPath);
        QProcess process;
        process.start("sh", QStringList() << "-c" << scriptPath);
        process.waitForFinished();

        // Alternative - get icon via system command
        QString iconScript = QString("sips -s format png /Applications/%1/Contents/Resources/*.icns --out /tmp/app_icon.png 2>/dev/null")
                                 .arg(QFileInfo(iconPath).baseName() + ".app");
        QProcess iconProcess;
        iconProcess.start("sh", QStringList() << "-c" << iconScript);
        iconProcess.waitForFinished();

        if (QFile::exists("/tmp/app_icon.png")) {
            icon = QIcon("/tmp/app_icon.png");
            QFile::remove("/tmp/app_icon.png");
        }
    }

    // In case of fail, trying to get icon in application resources
    if (icon.isNull() && iconPath.endsWith(".app")) {
        QString resourcesPath = iconPath + "/Contents/Resources";
        QDir resourcesDir(resourcesPath);
        QStringList iconFiles = resourcesDir.entryList(QStringList() << "*.icns" << "*.png" << "*.jpg", QDir::Files);

        if (!iconFiles.isEmpty()) {
            QString iconFile = resourcesPath + "/" + iconFiles.first();
            icon = QIcon(iconFile);
        }
    }

#else // Linux
    // 1. direct file path
    if (QFile::exists(iconPath)) {
        icon = QIcon(iconPath);
    }

    // 2. use QIcon::fromTheme()
    if (icon.isNull() && !iconPath.contains('/')) {
        icon = QIcon::fromTheme(iconPath);
        if (!icon.isNull())
            return icon;
    }

    // 5. trying to find via "find" command
    /*if (icon.isNull() && !iconPath.contains('/')) {
        QProcess findProcess;
        QString findCmd = QString("find /usr/share/icons /usr/share/pixmaps ~/.local/share/icons ~/.icons -name '%1.*' -type f 2>/dev/null | head -1").arg(iconPath);
        findProcess.start("sh", QStringList() << "-c" << findCmd);
        findProcess.waitForFinished(2000); // timeout 2 seconds

        QString foundPath = findProcess.readAllStandardOutput().trimmed();
        if (!foundPath.isEmpty() && QFile::exists(foundPath)) {
            icon = QIcon(foundPath);
        }
    }

    // 6. trying to search via executable file name
    if (icon.isNull() && iconPath.contains('/')) {
        QString execName = iconPath.split('/').last();
        if (execName != iconPath) { // if it was path
            icon = getApplicationIcon(execName); // recursive call with name
        }
    }*/

#endif

    // Fallback icon
    if (icon.isNull())
        icon = QIcon(":/icons/icons/application.png");

    return icon;
}

void MyOpenWithDialog::onOpenClicked()
{
    QString appPath;

    if (!m_customAppEdit->text().isEmpty()) {
        appPath = m_customAppEdit->text();
    } else if (m_appListWidget->currentItem()) {
        appPath = m_appListWidget->currentItem()->data(Qt::UserRole + 1).toString();
    }

    if (appPath.isEmpty()) {
        return;
    }

    if (openWith(m_filePath, appPath)) {
        accept();
    }
}

void MyOpenWithDialog::onBrowseClicked()
{
    QString filter = tr("All files (*)");

    QString appPath = QFileDialog::getOpenFileName(this,
                                                   tr("Choose application"), QDir::homePath(), filter);

    if (!appPath.isEmpty()) {
        m_customAppEdit->setText(appPath);
    }
}

void MyOpenWithDialog::onItemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    onOpenClicked();
}

bool MyOpenWithDialog::openWith(const QString &filePath, const QString &appPath)
{
#ifdef Q_OS_MACOS
    // Use "open" command on macOS
    QProcess process;
    if (appPath.endsWith(".app")) {
        process.start("open", QStringList() << "-a" << appPath << filePath);
    } else {
        process.start(appPath, QStringList() << filePath);
    }
    return process.waitForStarted();

#else
    // On Linux start application directly
    QProcess process;
    process.startDetached(appPath, QStringList() << filePath);
    return true;
#endif
}

void MyOpenWithDialog::show(const QString &filePath, QWidget *parent)
{
    MyOpenWithDialog dialog(filePath, parent);
    dialog.exec();
}
