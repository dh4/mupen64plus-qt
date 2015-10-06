#!/bin/bash

cd `dirname $0`

VERSION=`git ls-remote --tags https://github.com/dh4/mupen64plus-qt | tail -n 1 | sed -e 's#/# #g' | awk '{print $(NF)}'`
sed "s/@@VERSION@@/$VERSION/g" mupen64plus-qt.spec.in > mupen64plus-qt.spec
spectool -g -R mupen64plus-qt.spec
rpmbuild -ba mupen64plus-qt.spec
