#!/bin/bash
set -e

RUNNO=${1}
if [ `echo "${RUNNO}" | wc -c` -gt 3 ]; then
  echo "Running for RUNNO=${RUNNO}"
else
  RUNNO=03586 #Has magnetic field, Has Bars
  echo "Using default RUNNO of ${RUNNO}"
fi

if [ `echo "${AGRELEASE}" | wc -c` -gt 3 ]; then
  echo "AGRELEASE set ok: $AGRELEASE"
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi


start=`date +%s`

mkdir -p $AGRELEASE/testlogs

cd $AGRELEASE

export EOS_MGM_URL=root://eospublic.cern.ch

if [ ! -f $AGMIDASDATA/run${RUNNO}sub000.mid.lz4  ]; then
  eos cp /eos/experiment/ALPHAg/midasdata_old/run${RUNNO}sub000.mid.lz4 $AGMIDASDATA/
else
  echo "${AGMIDASDATA}/run${RUNNO}sub000.mid.lz4 found locally"
fi


GITHASH=`git rev-parse --short HEAD`
#Fails when detached:
#BRANCH=`git branch | grep \* | cut -c 3-`
BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `


echo "Running from $PWD : ./agana.exe -O${AGRELEASE}/bin/output${RUNNO}.root ${AGMIDASDATA}/run${RUNNO}sub000.mid.lz4 -- --usetimerange 0. 15.0 --time"
./agana.exe -O${AGRELEASE}/bin/output${RUNNO}.root ${AGMIDASDATA}/run${RUNNO}sub000.mid.lz4 -- --usetimerange 0. 15.0 --time &> $AGRELEASE/testlogs/agana_run_${RUNNO}_${GITHASH}.log



if [ ! -f ${AGMIDASDATA}/run02364sub000.mid.lz4  ]; then
  eos cp /eos/experiment/ALPHAg/midasdata_old/run02364sub000.mid.lz4 ${AGMIDASDATA}
else
  echo "${AGMIDASDATA}/run02364sub000.mid.lz4 found locally"
fi
echo "Running from $PWD: ./agana.exe -O${AGRELEASE}/bin/output2364.root ${AGMIDASDATA}/run02364sub000.mid.lz4 -- --usetimerange 0. 5.0 --time"
./agana.exe -O${AGRELEASE}/bin/output2364.root ${AGMIDASDATA}/run02364sub000.mid.lz4 -- --usetimerange 0. 15.0 --time &> $AGRELEASE/testlogs/agana_run_02364_${GITHASH}.log
echo "done"

if [ `ls $AGRELEASE/testlogs/agana_run_02364_* | wc -l` -gt 1 ]; then
   echo "Making diff of analysis..."
   #Catch exit state (1 if there is a differnce) with ||
   diff -u `ls -tr $AGRELEASE/testlogs/agana_run_02364_* | tail -n 2 ` > $AGRELEASE/testlogs/AnalysisDiff.log || :
   if [ -f $AGRELEASE/testlogs/AnalysisDiff.log ]; then
       cat $AGRELEASE/testlogs/AnalysisDiff.log
   fi
fi

end_ana=`date +%s`

mtstart_ana=`date +%s`
cd $AGRELEASE/bin
echo "Running from $PWD
./agana.exe -O${AGRELEASE}/bin/output${RUNNO}mt.root --mt ${AGMIDASDATA}/run${RUNNO}sub000.mid.lz4 -- --usetimerange 0. 15.0 --time &> $AGRELEASE/testlogs/mt_agana_run_${RUNNO}_${GITHASH}.log
"
./agana.exe -O${AGRELEASE}/bin/output${RUNNO}mt.root --mt ${AGMIDASDATA}/run${RUNNO}sub000.mid.lz4 -- --usetimerange 0. 15.0 --time &> $AGRELEASE/testlogs/mt_agana_run_${RUNNO}_${GITHASH}.log

if [[ $? -ne 0 ]]; then
    echo "mt test exit with error $?, running gdb..."
    gdb -ex="set logging on" -ex=r --args ./agana.exe -O/home/acapra/agsoft/bin/output03586mt.root --mt /daq/alpha_data0/acapra/alphag/midasdata/run03586sub000.mid.lz4 -- --usetimerange 0. 15.0 --time
    mv gdb.txt $AGRELEASE/testlogs/mt_agana_run_${RUNNO}_${GITHASH}.log
fi

mtend_ana=`date +%s`

cd $AGRELEASE

tail -n 50 $AGRELEASE/testlogs/agana_run_${RUNNO}_${GITHASH}.log
tail -n 50 $AGRELEASE/testlogs/mt_agana_run_${RUNNO}_${GITHASH}.log

