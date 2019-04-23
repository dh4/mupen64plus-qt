#!/bin/bash

cd `dirname $0`

VERSION=`cat ../../VERSION`
sed "s/@@VERSION@@/$VERSION/g" mupen64plus-qt.spec.in > mupen64plus-qt.spec
rpmdev-setuptree
spectool -g -R mupen64plus-qt.spec
rpmbuild -ba mupen64plus-qt.spec
