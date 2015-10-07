#!/bin/bash

cd `dirname $0`/../../
cp -r dist/debian .

VERSION=`git ls-remote --tags https://github.com/dh4/mupen64plus-qt | tail -n 1 | sed -e 's#/# #g' | awk '{print $(NF)}'`
debchange --create -v $VERSION-1 --package mupen64plus-qt "Version $VERSION"
debuild -i -us -uc -b
