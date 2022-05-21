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



cd ~/alphasoft
. agconfig.sh

if [ ${RUNNO} -lt 55000 ]; then
   echo "Invalid run Number"
   exit
fi

if [ ${RUNNO} -gt 75000 ]; then
   echo "Invalid run Number"
   exit
fi

cd $AGRELEASE

#Wait for file to open (60 second timeout)
ROOT_FILE_NAME=${AGOUTPUT}/output${RUNNO}.root
for i in `seq 1 600`; do
   # Check timeout ...
   if [ ${i} -gt 60 ]; then
      echo "Could not open file after ${i} seconds. Did the analyzer crash?"
      exit 1
   fi

   root.exe -b -l -q -e "TFile *f = TFile::Open(\"${ROOT_FILE_NAME}\"); if ((!f) || f->IsZombie() || f->TestBit(TFile::kRecovered)) { cout << \"There is a problem with the file: $filename_to_check\n\"; exit(1); }" > /dev/null
   if [ $? -ne 0 ]; then
      echo "${ROOT_FILE_NAME} has error, analysis is probably still running"
      sleep 1
      continue;
   else
      break;
   fi

 done

echo "Generating SIS plots..."
echo ".L alpha2/macros/SaveAllDumps.cxx 
SaveAllDumps(${RUNNO})
SaveAllDumpsSVD(${RUNNO})
.q
" | root -l -b &> AutoSISPlots/R${RUNNO}.log

echo "done"


echo "auto FRDs = Get_A2_Spills(${RUNNO},{\"FastRampDown\"},{-1});
if (FRDs.size()) {
TA2Plot a;
a.AddDumpGates(FRDs);
a.LoadData();
a.GetVertexEvents()->PrintPassCutEvents();
}
.q
" | root -l -b &> AutoSISPlots/R${RUNNO}_FRD_PassCutsEvents.log

echo "Cleaning up old runs"


cd $AGRELEASE/AutoSISPlots

for i in `seq $(( ${RUNNO} - 200 )) $(( ${RUNNO} - 100 ))`; do
rm -vrf $AGRELEASE/AutoSISPlots/${i}
done


echo `pwd`
