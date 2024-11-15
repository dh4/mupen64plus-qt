#!/bin/bash

[[ -z $WORKING_DIR ]] && WORKING_DIR=$(pwd)
[[ -z $VERSION ]] && VERSION=$(git log --oneline -n 1 | awk '{print $1}')


case "$1" in

    'setup_qt')
        cd "$WORKING_DIR/../"

        # Build Qt with mxe
        sudo apt-get update -qq
        sudo apt-get install \
            autoconf automake autopoint bash bison bzip2 cmake flex \
            gettext git g++ gperf intltool libffi-dev libtool \
            libltdl-dev libssl-dev libxml-parser-perl make openssl \
            p7zip-full patch perl pkg-config python3 python-is-python3 ruby scons sed \
            unzip wget xz-utils libtool-bin lzip libgdk-pixbuf2.0-dev
        git clone https://github.com/mxe/mxe.git
        cd mxe
        make -j8 MXE_TARGETS='x86_64-w64-mingw32.static' qt6-qtbase qt6-qt5compat sdl2

        cd "$WORKING_DIR/mupen64plus-qt"
    ;;

    'get_quazip')
        wget https://github.com/stachenov/quazip/archive/refs/tags/v1.4.tar.gz
        tar -xvzf v1.4.tar.gz > /dev/null
        mv quazip-1.4/quazip quazip
    ;;

    'build')
        export PATH="$PATH:$WORKING_DIR/../mxe/usr/bin"

        ./build-scripts/revision.sh
        cp dist/windows/mupen64plus-qt.pro .
        x86_64-w64-mingw32.static-qt6-qmake
        make
    ;;

    'package')
        mkdir -p "build/"

        mv release/mupen64plus-qt.exe resources/README.txt .
        zip "build/mupen64plus-qt_win_$VERSION.zip" mupen64plus-qt.exe README.txt
    ;;

esac
