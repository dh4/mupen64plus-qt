#!/bin/bash

git log --oneline -n 1 | grep 'Bump version'

if [[ $? == 1 ]]; then
    REVISION=$(git log --oneline -n 1 | awk '{print $1}')
    sed -i -e "s/Version = \"\([0-9.]*\)\"/Version = \"\1~git:$REVISION\"/g" src/global.h
fi

cat src/global.h | grep Version | awk '{print $(NF)}' | sed -e 's/[";~]//g' -e 's/:/-/g'
