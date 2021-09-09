#!/bin/bash

RUNNO=${1}

if [ -z ${RUNNO} ]; then
   echo "No run number set"
   exit
fi

cd ~/alphasoft
. agconfig.sh

if [ ${RUNNO} -lt 50000 ]; then
   echo "Invalid run Number"
   exit
fi

if [ ${RUNNO} -gt 60000 ]; then
   echo "Invalid run Number"
   exit
fi

cd $AGRELEASE

echo "Generating SIS plots..."
echo ".L alpha2/macros/SaveAllDumps.cxx 
SaveAllDumps(${RUNNO})
.q
" | root -l -b &> AutoSISPlots/R${RUNNO}.log

echo "done"


echo "Cleaning up old runs"


cd $AGRELEASE/AutoSISPlots

for i in `seq $(( ${RUNNO} - 200 )) $(( ${RUNNO} - 100 ))`; do
rm -vrf $AGRELEASE/AutoSISPlots/${i}
done


echo `pwd`
