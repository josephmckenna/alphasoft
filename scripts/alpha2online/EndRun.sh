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

RUNNO=${1}
if [ -z ${RUNNO} ]; then
   echo "No run number set"
   Run_number=`odbedit -c 'ls "/Runinfo/Run number"' | grep -v command | awk '{print $3}' | head -n1`
   echo "\n"
   echo ${Run_number}
   echo "\n"
   RUNNO=`echo $Run_number `
fi


sleep 3

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


echo "auto FRDs = Get_A2_Spills({RUNNO},{\"FastRampDown\"},{-1});
if (FRD.size()) {
TA2Plot a;
a.AddDumpGates(FRDs);
a.LoadData();
a.GetVertexEvent()->PrintPassCutEvents();
.q
" | root -l -b &> AutoSISPlots/R${RUNNO}_FRD_PassCutsEvents.log

echo "Cleaning up old runs"


cd $AGRELEASE/AutoSISPlots

for i in `seq $(( ${RUNNO} - 200 )) $(( ${RUNNO} - 100 ))`; do
rm -vrf $AGRELEASE/AutoSISPlots/${i}
done


echo `pwd`
