#!/bin/bash

echo "#define GIT_DATE            "  $( git log -n 1 --date=raw | grep Date | cut -b 8-19) > include/GitInfo.h
echo "#define GIT_REVISION      \" $( git rev-parse --short HEAD ) \"" >> include/GitInfo.h
echo "#define GIT_REVISION_FULL \" $( git log -n 1 | grep commit | cut -b 8-99) \"" >> include/GitInfo.h
echo "#define GIT_BRANCH        \" $( git branch --remote --no-abbrev --contains) \"" >> include/GitInfo.h
echo "#define GIT_DIFF_SHORT_STAT \" $( git branch --no-abbrev --contains) : $( git diff --shortstat) \"" >> include/GitInfo.h
echo "#define COMPILATION_DATE    " $( date +%s) >> include/GitInfo.h
