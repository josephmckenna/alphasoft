#!/bin/bash
set -e

RUNNO=${1}
if [ `echo "${RUNNO}" | wc -c` -gt 3 ]; then
  echo "Running for RUNNO=${RUNNO}"
else
  RUNNO=02364
  echo "Using default RUNNO of ${RUNNO}"
fi

if [ `echo "${AGRELEASE}" | wc -c` -gt 3 ]; then
  echo "AGRELEASE set ok: $AGRELEASE"
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi



cd $AGRELEASE/ana
make
export EOS_MGM_URL=root://eospublic.cern.ch

if [ ! -f run${RUNNO}sub000.mid.lz4  ]; then
  eos cp /eos/experiment/alpha/run${RUNNO}sub000.mid.lz4 .
else
  echo "run${RUNNO}sub000.mid.lz4 found locally"
fi

#Calling -h returns with a non-zero exit code
#./agana.exe -h
#Calling with a fake input file and --help finishes with a exit code 0 (not fail)
mkdir -p test-results
./agana.exe run02364sub000.mid.lz4 -- --usetimerange 0. 1.0 | tee test-results/agana_run_${RUNNO}.txt
#./agana.exe run02364sub000.mid.lz4 -- ---useeventrange  0 2 | tee test-results/agana_run_${RUNNO}.txt


tail -n 50 test-results/agana_run_${RUNNO}.txt

echo ".L macros/ReadEventTree.C 
ReadEventTree()
.q
" | root -l -b *${RUNNO}*.root

echo "Leak test:"
cd $AGRELEASE/scripts/UnitTest/
./LeakCheck.sh 2364 NOBUILD 20



#./agana.exe fakefile -- --help
echo "Add more here"


