#!/bin/bash

git show-ref --tags | grep "`git log -n 1 --pretty='%H'`"

if [[ $? == 1 ]]; then
    REVISION=$(git log -n 1 --pretty='%h')
    [[ $REVISION ]] && sed -i -e "s/Version = \"\([0-9.]*\)\"/Version = \"\1~git:$REVISION\"/g" src/global.h
fi

cat src/global.h | grep Version | awk '{print $(NF)}' | sed -e 's/[";~]//g' -e 's/:/-/g'
