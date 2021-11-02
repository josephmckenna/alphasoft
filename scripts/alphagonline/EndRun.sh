#!/bin/bash

case `hostname` in
alphagdaq*)
    echo "Good, we are on alphagdaq"
    ;;
*)
    echo "The start_daq script should be executed on alphagdaq"
    exit 1
    ;;
esac


/zssd/home1/agdaq/pythonMidas/endrun.py


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

if [ ${RUNNO} -lt 3000 ]; then
   echo "Invalid run Number"
   exit
fi

if [ ${RUNNO} -gt 15000 ]; then
   echo "Invalid run Number"
   exit
fi

cd $AGRELEASE

echo "Waiting for analysis to finish and close the root file"
for i in `seq 1 100`; do
   root -l -q root_output_files/output0${RUNNO}.root 1> /dev/null 2> errout
   ERRORS=`cat errout | wc -l`
   if [ ${ERRORS} -eq 0 ]; then
      break
   fi
   echo "Failed to open"
   sleep 1
done



echo "Generating Chrono plots..."
echo ".L ana/macros/SaveAllDumps.C
SaveAllDumps(${RUNNO})
.q
" | root -l -b &> AutoChronoPlots/R${RUNNO}.log

echo "done"


echo "Cleaning up old runs"


cd $AGRELEASE/AutoChronoPlots

for i in `seq $(( ${RUNNO} - 200 )) $(( ${RUNNO} - 100 ))`; do
rm -vrf $AGRELEASE/AutoChronoPlots/${i}
done


echo `pwd`
