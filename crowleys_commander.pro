QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 console

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
    history_linux.ini

windows:LIBS += -lole32 -luuid
linux:LIBS += -ludev

RESOURCES += \
    my_res.qrc
