cmake_minimum_required(VERSION 3.16)
project(mupen64plus-qt VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui Network Sql Widgets Xml)
find_package(QuaZip-Qt6 REQUIRED)

qt_standard_project_setup()

qt_add_executable(mupen64plus-qt WIN32 MACOSX_BUNDLE
    src/common.cpp src/common.h
    src/dialogs/aboutdialog.cpp src/dialogs/aboutdialog.h
    src/dialogs/configeditor.cpp src/dialogs/configeditor.h
    src/dialogs/controlinfodialog.cpp src/dialogs/controlinfodialog.h
    src/dialogs/downloaddialog.cpp src/dialogs/downloaddialog.h
    src/dialogs/gamesettingsdialog.cpp src/dialogs/gamesettingsdialog.h src/dialogs/gamesettingsdialog.ui
    src/dialogs/keycodesdialog.cpp src/dialogs/keycodesdialog.h
    src/dialogs/logdialog.cpp src/dialogs/logdialog.h
    src/dialogs/settingsdialog.cpp src/dialogs/settingsdialog.h src/dialogs/settingsdialog.ui
    src/emulation/emulatorhandler.cpp src/emulation/emulatorhandler.h
    src/global.h
    src/main.cpp
    src/mainwindow.cpp src/mainwindow.h
    src/roms/romcollection.cpp src/roms/romcollection.h
    src/roms/thegamesdbscraper.cpp src/roms/thegamesdbscraper.h
    src/views/gridview.cpp src/views/gridview.h
    src/views/listview.cpp src/views/listview.h
    src/views/tableview.cpp src/views/tableview.h
    src/views/widgets/clickablewidget.cpp src/views/widgets/clickablewidget.h
    src/views/widgets/treewidgetitem.cpp src/views/widgets/treewidgetitem.h
)
target_link_libraries(mupen64plus-qt PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Sql
    Qt::Widgets
    Qt::Xml
)


# Resources:
set(mupen64plusqt_resource_files
    "resources/images/keyboard.png"
    "resources/images/mupen64plus.png"
    "resources/images/not-found.png"
    "resources/locale/mupen64plus-qt_fr.qm"
    "resources/locale/mupen64plus-qt_ru.qm"
    "resources/other/LICENSE"
    "resources/other/VERSION"
)

qt_add_resources(mupen64plus-qt "mupen64plusqt"
    PREFIX
        "/"
    BASE
        "resources"
    FILES
        ${mupen64plusqt_resource_files}
)

if(MACOS OR WIN32 OR linux_quazip_static)
    target_sources(mupen64plus-qt PUBLIC
        quazip/*.c quazip/*.cpp quazip/*.h
    )

    target_compile_definitions(mupen64plus-qt PRIVATE
        QUAZIP_STATIC
    )

    target_link_libraries(mupen64plus-qt PRIVATE
        z
    )
endif()

if(UNIX AND NOT MACOS AND NOT linux_quazip_static)
    target_link_libraries(mupen64plus-qt PRIVATE
        QuaZip::QuaZip
    )
endif()

install(TARGETS mupen64plus-qt
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET mupen64plus-qt
    FILENAME_VARIABLE deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
