#!/bin/bash

[[ -z $WORKING_DIR ]] && WORKING_DIR=$(pwd)
[[ -z $VERSION ]] && VERSION=$(git log --oneline -n 1 | awk '{print $1}')


case "$1" in

    'setup_qt')
        cd $WORKING_DIR/../

        if [[ $BUILD_MXE_QT ]]; then
            # Build Qt with mxe
            sudo apt-get update -qq
            apt-get install \
                autoconf automake autopoint bash bison bzip2 cmake flex \
                gettext git g++ gperf intltool libffi-dev libtool \
                libltdl-dev libssl-dev libxml-parser-perl make openssl \
                p7zip-full patch perl pkg-config python ruby scons sed \
                unzip wget xz-utils
            git clone https://github.com/mxe/mxe.git
            cd mxe
            make qtbase
        else
            # Fetch pre-built mxe Qt
            wget https://www.dropbox.com/s/jr6l4lnixizqtln/travis-mxe-qt5.tar.gz
            tar -xvzf travis-mxe-qt5.tar.gz > /dev/null
        fi

        cd $WORKING_DIR/mupen64plus-qt
    ;;

    'get_quazip')
        wget http://downloads.sourceforge.net/quazip/quazip-0.7.1.tar.gz
        tar -xvzf quazip-0.7.1.tar.gz > /dev/null
        mv quazip-0.7.1/quazip .
    ;;

    'build')
        export PATH=$PATH:$WORKING_DIR/../mxe/usr/bin

        ./build-scripts/revision.sh
        i686-w64-mingw32.static-qmake-qt5
        make
    ;;

    'package')
        mkdir build

        mv release/mupen64plus-qt.exe resources/README.txt .
        zip build/mupen64plus-qt_win_$VERSION.zip mupen64plus-qt.exe README.txt
    ;;

esac
