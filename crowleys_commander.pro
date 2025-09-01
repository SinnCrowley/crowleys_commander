QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    archivemanager.cpp \
    devicewatcher.cpp \
    main.cpp \
    mainwindow.cpp \
    mycreatearchivedialog.cpp \
    myfilesystemmodel.cpp \
    myopenwithdialog.cpp \
    mysearchdialog.cpp \
    mysettingsdialog.cpp \
    mysortfilterproxymodel.cpp \
    mytreeview.cpp \
    styletweaks.cpp

HEADERS += \
    archivemanager.h \
    devicewatcher.h \
    mainwindow.h \
    mycreatearchivedialog.h \
    myfilesystemmodel.h \
    myopenwithdialog.h \
    mysearchdialog.h \
    mysettingsdialog.h \
    mysortfilterproxymodel.h \
    mytreeview.h \
    styletweaks.h

FORMS += \
    mainwindow.ui \
    mysearchdialog.ui \
    mysettingsdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    config.ini \
    history.ini \
    config_templates/config.ini \
    config_templates/history_linux.ini \
    config_templates/history_windows.ini


# config files
linux:configtemplates.path = /etc/crowleys_commander/templates
linux:configtemplates.files = \
    config_templates/config.ini \
    config_templates/history_linux.ini

linux:icons.path = /opt/$${TARGET}/icons
linux:icons.files = icons/crowleys_commander.png

# .desktop file installing (Linux)
linux:desktop.files = crowleys_commander.desktop
linux:desktop.path = /usr/share/applications

linux:INSTALLS += configtemplates icons desktop

linux:QMAKE_POST_LINK += \
    mkdir -p $$(HOME)/.config/crowleys_commander


macx:configtemplates.path = Contents/Resources/config_templates
macx:configtemplates.files = \
    config_templates/config.ini \
    config_templates/history_linux.ini

macx:ICON = icons/crowleys_commander.icns
macx:CONFIG += app_bundle
macx:QMAKE_BUNDLE_DATA += configtemplates


windows:LIBS += -lole32 -luuid #-loleaut32 # Bit7z
linux:LIBS += -ludev
macx:LIBS += -framework IOKit -framework CoreFoundation

RESOURCES += \
    my_res.qrc


# archive support for linux
linux {
    # try with pkg-config
    CONFIG += link_pkgconfig
    PKGCONFIG += libarchive

    # if pkg-config doesn't work, use default paths
    !packagesExist(libarchive) {
        INCLUDEPATH += /usr/include
        LIBS += -larchive
    }
}

# archive support for macOS (install via Homebrew: brew install libarchive)
macx {
    INCLUDEPATH += $$PWD/libs/libarchive/macos/include
    LIBS += -L$$PWD/libs/libarchive/macos/lib -larchive -lzlib -lbz2 -llzma -llz4 -lzstd -lcharset -lcrypto -liconv -lssl -lxml2
}

# archive support for Windows
windows {
    INCLUDEPATH += $$PWD/libs/libarchive/windows/include
    LIBS += -L$$PWD/libs/libarchive/windows/lib -larchive -lzlib -lbz2 -llzma -llz4 -lzstd -lcharset -lcrypto -liconv -lssl -lxml2 -lcrypt32 -lws2_32
}


# # Bit7z
# # archive support for linux
# linux {
#     INCLUDEPATH += $$PWD/libs/bit7z/linux/include
#     LIBS += -L$$PWD/libs/bit7z/linux/lib -lbit7z64 -ldl
# }

# # archive support for macOS
# macx {
#     INCLUDEPATH += $$PWD/libs/bit7z/macos/include
#     LIBS += -L$$PWD/libs/bit7z/macos/lib -lbit7z64 -ldl
# }

# # archive support for Windows
# windows {
#     INCLUDEPATH += $$PWD/libs/bit7z/windows/include
#     LIBS += -L$$PWD/libs/bit7z/windows/lib -lbit7z64
# }

# DEFINES += BIT7Z_AUTO_FORMAT=ON BIT7Z_LINK_LIBCPP=OFF
