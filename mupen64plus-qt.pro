QT       += core

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
    src/aboutdialog.cpp \
    src/mupen64plusqt.cpp \
    src/settingsdialog.cpp \
    src/treewidgetitem.cpp

HEADERS += src/global.h \
    src/aboutdialog.h \
    src/mupen64plusqt.h \
    src/settingsdialog.h \
    src/treewidgetitem.h

RESOURCES += resources/mupen64plusqt.qrc

FORMS += src/settingsdialog.ui
