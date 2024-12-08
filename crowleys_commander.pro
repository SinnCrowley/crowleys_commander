QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    devicewatcher.cpp \
    main.cpp \
    mainwindow.cpp \
    myfilesystemmodel.cpp \
    mysearchdialog.cpp \
    mysortfilterproxymodel.cpp \
    mytreeview.cpp \
    styletweaks.cpp

HEADERS += \
    devicewatcher.h \
    mainwindow.h \
    myfilesystemmodel.h \
    mysearchdialog.h \
    mysortfilterproxymodel.h \
    mytreeview.h \
    styletweaks.h

FORMS += \
    mainwindow.ui \
    mysearchdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    config.ini \
    history.ini \
    templates/config.ini \
    templates/history_linux.ini \
    templates/history_windows.ini

# config files
linux:configtemplates.path = /etc/crowleys_commander/templates
linux:configtemplates.files = \
    config_templates/config.ini \
    config_templates/history_linux.ini

linux:icons.path = /opt/$${TARGET}/icons
linux:icons.files = icons/crowleys_commander.png

# Установка .desktop файла
linux:desktop.files = crowleys_commander.desktop
linux:desktop.path = /usr/share/applications

linux:INSTALLS += configtemplates icons desktop

linux:QMAKE_POST_LINK += \
    mkdir -p $$(HOME)/.config/crowleys_commander

windows:LIBS += -lole32 -luuid
linux:LIBS += -ludev
macx:LIBS += -framework IOKit -framework CoreFoundation

RESOURCES += \
    my_res.qrc
