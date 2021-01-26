#!/bin/bash

LIMITEVENTS=2000
#RUNNO=904620
RUNNO=904139
i=17

DIR="${AGRELEASE}/RunLogs"
if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

#BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `
BRANCH=$(git status | head -1 | awk '{print $4}')

LEAKTEST="$DIR/LeakTest${i}_${BRANCH}.log"
Event_Limit=" -e$LIMITEVENTS "
MODULESFLAGS="-- --diag --Bfield 0 --calib --location TRIUMF"

SUPP="--suppressions=${AGRELEASE}/scripts/UnitTest/valgrind-cling.supp"
if [ -f ${ROOTSYS}/etc/valgrind-root.supp ]; then
    SUPP=${SUPP}" --suppressions=${ROOTSYS}/etc/valgrind-root.supp"
fi

ALPHATEST="$DIR/LeakTest_AnalysisOut_${i}_${BRANCH}.log"

set -x
valgrind --leak-check=full --error-limit=no ${SUPP} --log-file="${LEAKTEST}" agana.exe ${Event_Limit} --mt -O${DATADIR}/dbg/run${RUNNO}sub000leaktestmt.root ${MIDASDATA}/run${RUNNO}sub000.mid.lz4 ${MODULESFLAGS} |& tee ${ALPHATEST}
