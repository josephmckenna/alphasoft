#!/bin/bash

OUTPUT_PATH=${1}
echo "Logging git info into ${OUTPUT_PATH}/GitInfo.h"
echo "#define GIT_DATE            "  $( git log -n 1 --date=raw | grep Date | cut -b 8-19 ) > ${OUTPUT_PATH}/GitInfo.h
echo "#define GIT_REVISION      \" $( git rev-parse --short HEAD ) \"" >> ${OUTPUT_PATH}/GitInfo.h
echo "#define GIT_REVISION_FULL \" $( git log -n 1 | grep commit | cut -b 8-99) \"" >> ${OUTPUT_PATH}/GitInfo.h
echo "#define GIT_BRANCH        \" $( git branch --remote --no-abbrev --contains  | tr '\n' ' ' ) \"" >> ${OUTPUT_PATH}/GitInfo.h
echo "#define GIT_DIFF_SHORT_STAT \" $( git branch --no-abbrev --contains | tr '\n' ' ') : $( git diff --shortstat) \"" >> ${OUTPUT_PATH}/GitInfo.h
echo "#define COMPILATION_DATE    " $( date +%s) >> ${OUTPUT_PATH}/GitInfo.h
