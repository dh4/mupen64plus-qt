#!/bin/bash

[[ -z $WORKING_DIR ]] && WORKING_DIR=$(pwd)
[[ -z $VERSION ]] && VERSION=$(git log --oneline -n 1 | awk '{print $1}')

[[ -z $ARCH ]] && ARCH=".$(uname -m)"


case "$1" in

    'setup_qt')
        sudo apt-get update -qq
        sudo apt-get -y install cmake qt6-base-dev libquazip1-qt6-dev
    ;;

    'get_quazip')
        wget https://github.com/stachenov/quazip/archive/refs/tags/v1.4.tar.gz
        tar -xvzf v1.4.tar.gz > /dev/null
        mv quazip-1.4/quazip quazip
    ;;

    'build')
        ./build-scripts/revision.sh
        cmake -DLINUX_QUAZIP_STATIC=ON .
        make
    ;;

    'package')
        mkdir -p "build/"

        mv resources/README.txt .
        tar -cvzpf "build/mupen64plus-qt_linux_$VERSION.tar.gz" mupen64plus-qt README.txt
    ;;

esac
