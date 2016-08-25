#!/bin/bash

[[ -z $WORKING_DIR ]] && WORKING_DIR=$(pwd)
[[ -z $VERSION ]] && VERSION=$(git log --oneline -n 1 | awk '{print $1}')


case "$1" in

    'setup_qt')
        sudo apt-get update -qq
        sudo apt-get -y install qt4-qmake libqt4-dev libqt4-sql-sqlite
    ;;

    'get_quazip')
        wget http://downloads.sourceforge.net/quazip/quazip-0.7.1.tar.gz
        tar -xvzf quazip-0.7.1.tar.gz > /dev/null
        mv quazip-0.7.1/quazip .
    ;;

    'build')
        ./build-scripts/revision.sh
        qmake-qt4 CONFIG+=linux_quazip_static
        make
    ;;

    'package')
        mkdir -p "build/$TRAVIS_BRANCH"

        mv resources/README.txt .
        tar -cvzpf "build/$TRAVIS_BRANCH/mupen64plus-qt_linux_$VERSION.tar.gz" mupen64plus-qt README.txt
    ;;

esac
