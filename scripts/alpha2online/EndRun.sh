#!/bin/bash

case `hostname` in
alphasuperdaq*)
    echo "Good, we are on alphasuperdaq"
    ;;
*)
    echo "The start_daq script should be executed on alphadaq"
    exit 1
    ;;
esac

sleep 3

RUNNO=${1}
if [ -z ${RUNNO} ]; then
   echo "No run number set"
   Run_number=`odbedit -e ${MIDAS_EXPT_NAME} -c 'ls "/Runinfo/Run number"'`
   echo ${Run_number}
   RUNNO=`echo $Run_number | awk '{print $3}'`
fi

cd ~/alphasoft
. agconfig.sh

if [ ${RUNNO} -lt 55000 ]; then
   echo "Invalid run Number"
   exit
fi

if [ ${RUNNO} -gt 65000 ]; then
   echo "Invalid run Number"
   exit
fi

cd $AGRELEASE

echo "Generating SIS plots..."
echo ".L alpha2/macros/SaveAllDumps.cxx 
SaveAllDumps(${RUNNO})
SaveAllDumpsSVD(${RUNNO})
.q
" | root -l -b &> AutoSISPlots/R${RUNNO}.log

echo "done"


echo "Cleaning up old runs"


cd $AGRELEASE/AutoSISPlots

for i in `seq $(( ${RUNNO} - 200 )) $(( ${RUNNO} - 100 ))`; do
rm -vrf $AGRELEASE/AutoSISPlots/${i}
done


echo `pwd`
