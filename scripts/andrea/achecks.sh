#!/bin/bash

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


GITHASH=`git rev-parse --short HEAD`
#Fails when detached:
#BRANCH=`git branch | grep \* | cut -c 3-`
BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `


cd $AGRELEASE/
mkdir -p $AGRELEASE/RunLogs

export EOS_MGM_URL=root://eospublic.cern.ch

##########################################################################################

start=`date +%s`

if [ ! -f $AGMIDASDATA/run${RUNNO}sub000.mid.lz4  ]; then
  eos cp /eos/experiment/ALPHAg/midasdata_old/run${RUNNO}sub000.mid.lz4 $MIDASDATA/
  SOURCE1="${MIDASDATA}/run${RUNNO}sub000.mid.lz4"
else
  echo "${AGMIDASDATA}/run${RUNNO}sub000.mid.lz4 found locally"
  SOURCE1="${AGMIDASDATA}/run${RUNNO}sub000.mid.lz4"
fi

cd $AGRELEASE/bin

echo "Running from $PWD : ./agana.exe -O${AGOUTPUT}/output${RUNNO}.root ${SOURCE1} -- --usetimerange 0. 15.0 --time"
./agana.exe -O${AGOUTPUT}/output${RUNNO}.root ${SOURCE1} -- --usetimerange 0. 15.0 --time &> $AGRELEASE/RunLogs/agana_run_${RUNNO}_${GITHASH}.log

##########################################################################################

if [ ! -f ${AGMIDASDATA}/run02364sub000.mid.lz4  ]; then
  eos cp /eos/experiment/ALPHAg/midasdata_old/run02364sub000.mid.lz4 $MIDASDATA/
  SOURCE2="${MIDASDATA}/run02364sub000.mid.lz4"
else
  echo "${AGMIDASDATA}/run02364sub000.mid.lz4 found locally"
  SOURCE2="${AGMIDASDATA}/run02364sub000.mid.lz4"
fi
echo "Running from $PWD: ./agana.exe -O${AGOUTPUT}/output2364.root ${SOURCE2} -- --usetimerange 0. 5.0 --time"
./agana.exe -O${AGOUTPUT}/output2364.root ${SOURCE2} -- --usetimerange 0. 15.0 --time &> $AGRELEASE/RunLogs/agana_run_02364_${GITHASH}.log
echo "done"

if [ `ls $AGRELEASE/RunLogs/agana_run_02364_* | wc -l` -gt 1 ]; then
   echo "Making diff of analysis..."
   #Catch exit state (1 if there is a differnce) with ||
   diff -u `ls -tr $AGRELEASE/RunLogs/agana_run_02364_* | tail -n 2 ` > $AGRELEASE/RunLogs/AnalysisDiff.log || :
   if [ -f $AGRELEASE/RunLogs/AnalysisDiff.log ]; then
       cat $AGRELEASE/RunLogs/AnalysisDiff.log
   fi
fi

end_ana=`date +%s`

##########################################################################################

mtstart_ana=`date +%s`
cd $AGRELEASE/bin
echo "Running from $PWD
./agana.exe -O${AGOUTPUT}/output${RUNNO}mt.root --mt ${SOURCE1} -- --usetimerange 0. 15.0 --time &> $AGRELEASE/RunLogs/mt_agana_run_${RUNNO}_${GITHASH}.log
"
./agana.exe -O${AGOUTPUT}/output${RUNNO}mt.root --mt ${SOURCE1} -- --usetimerange 0. 15.0 --time &> $AGRELEASE/RunLogs/mt_agana_run_${RUNNO}_${GITHASH}.log

if [[ $? -ne 0 ]]; then
    echo "mt test exit with error $?, running gdb..."
    gdb -ex="set logging on" -ex=r --args ./agana.exe -O${AGOUTPUT}/output${RUNNO}mt.root --mt ${SOURCE1} -- --usetimerange 0. 15.0 --time
    mv gdb.txt $AGRELEASE/RunLogs/mt_agana_run_${RUNNO}_${GITHASH}.log
fi

mtend_ana=`date +%s`

##########################################################################################

cd $AGRELEASE

tail -n 50 $AGRELEASE/RunLogs/agana_run_${RUNNO}_${GITHASH}.log
tail -n 50 $AGRELEASE/RunLogs/mt_agana_run_${RUNNO}_${GITHASH}.log

