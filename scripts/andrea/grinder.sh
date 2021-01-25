#!/bin/bash

LIMITEVENTS=2000
RUNNO=904620
i=2

DIR="${AGRELEASE}/RunLogs"
if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

#BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `
BRANCH=$(git status | head -1 | awk '{print $4}')

LEAKTEST="$DIR/LeakTest${i}_${BRANCH}.log"
Event_Limit=" -e$LIMITEVENTS "
MODULESFLAGS="-- --diag --anasettings ${AGRELEASE}/ana/cyl_l1.5.json --Bfield 0 --calib --location TRIUMF"
if [ -f ${ROOTSYS}/etc/valgrind-root.supp ]; then
    SUPP="--suppressions=${ROOTSYS}/etc/valgrind-root.supp"
fi
ALPHATEST="$DIR/LeakTest_AnalysisOut_${i}_${BRANCH}.log"

set -x
valgrind --leak-check=full --error-limit=no ${SUPP} --log-file="${LEAKTEST}" agana.exe ${Event_Limit} --mt -O${AGRELEASE}/run${RUNNO}sub000leaktest.root ${MIDASDATA}/run${RUNNO}sub000.mid.lz4 ${MODULESFLAGS} &> ${ALPHATEST} &

tail -f ${ALPHATEST}
