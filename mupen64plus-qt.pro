QT       += core network xml sql

lessThan(QT_MAJOR_VERSION, 5) {
    QT   += gui
} else {
    QT   += widgets
}

macx {
    TARGET = Mupen64Plus-Qt
} else {
    TARGET = mupen64plus-qt
}

TEMPLATE = app
macx:ICON = macosx/mupen64plus.icns
win32:RC_FILE = windows/icon.rc


SOURCES += src/main.cpp \
    src/common.cpp \
    src/mainwindow.cpp \
    src/aboutdialog.cpp \
    src/settingsdialog.cpp \
    src/treewidgetitem.cpp \
    src/clickablewidget.cpp \
    src/configeditor.cpp \
    src/downloaddialog.cpp \
    src/logdialog.cpp \
    src/emulatorhandler.cpp \
    src/romcollection.cpp \
    src/thegamesdbscrapper.cpp

HEADERS += src/global.h \
    src/common.h \
    src/mainwindow.h \
    src/aboutdialog.h \
    src/settingsdialog.h \
    src/treewidgetitem.h \
    src/clickablewidget.h \
    src/configeditor.h \
    src/downloaddialog.h \
    src/logdialog.h \
    src/emulatorhandler.h \
    src/romcollection.h \
    src/thegamesdbscrapper.h

RESOURCES += resources/mupen64plusqt.qrc

FORMS += src/settingsdialog.ui

win32|macx {
    CONFIG += staticlib
    DEFINES += QUAZIP_STATIC

    #Download quazip source and copy the quazip directory to project
    SOURCES += quazip/*.cpp
    SOURCES += quazip/*.c
    HEADERS += quazip/*.h
} else {
    lessThan(QT_MAJOR_VERSION, 5) {
        LIBS += -lquazip
    } else {
        # Debian distributions use a different library name for Qt5 quazip
        system("uname -a | grep -E 'Debian|Ubuntu' > /dev/null") {
            LIBS += -lquazip-qt5
        } else {
            LIBS += -lquazip5
        }
    }
}
