#!/bin/bash

LIMITEVENTS=2000
RUNNO=904620
i=2

DIR="${AGRELEASE}/RunLogs"
if [ ! -d "${DIR}" ]; then
    mkdir -p ${DIR}
fi

BRANCH=$(git status | head -1 | awk '{print $4}')

SPEEDTEST="$DIR/SpeedTest${i}_${BRANCH}.log"
Event_Limit=" -e$LIMITEVENTS "
MODULESFLAGS="-- --diag --anasettings ${AGRELEASE}/ana/cyl_l1.5.json --Bfield 0 --calib --location TRIUMF"

ALPHATEST="$DIR/SpeedTest_AnalysisOut_${i}_${BRANCH}.log"

set -x
valgrind --tool=callgrind --callgrind-out-file="${SPEEDTEST}" agana.exe ${Event_Limit} -O${DATADIR}/dev/run${RUNNO}sub000speedtest.root ${MIDASDATA}/run${RUNNO}sub000.mid.lz4 ${MODULESFLAGS} |& tee ${ALPHATEST}

set +x
echo "done..."
echo "check:
  kcachegrind ${SPEEDTEST}
"
