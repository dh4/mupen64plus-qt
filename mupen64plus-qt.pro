QT       += core

lessThan(QT_MAJOR_VERSION, 5) {
    QT   += gui
} else {
    QT   += widgets
}

TARGET = mupen64plus-qt
TEMPLATE = app


SOURCES += src/main.cpp \
    src/aboutdialog.cpp \
    src/mupen64plusqt.cpp \
    src/settingsdialog.cpp

HEADERS  += \
    src/aboutdialog.h \
    src/global.h \
    src/mupen64plusqt.h \
    src/settingsdialog.h

RESOURCES += \
    resources/mupen64plusqt.qrc
