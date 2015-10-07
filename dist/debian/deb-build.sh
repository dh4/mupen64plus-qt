#!/bin/bash

cd `dirname $0`/../../

VERSION=`cat VERSION`

mkdir deb-build
cd deb-build

wget https://github.com/dh4/mupen64plus-qt/archive/$VERSION.tar.gz

if [[ $? == 0 ]]; then
    tar -xvzf $VERSION.tar.gz
    cp -r ../dist/debian mupen64plus-qt-$VERSION
    cd mupen64plus-qt-$VERSION
    debchange --create -v $VERSION-1 --package mupen64plus-qt "Version $VERSION"
    debuild -i -us -uc -b
    mv ../mupen64plus-qt_$VERSION*.deb ../..
fi