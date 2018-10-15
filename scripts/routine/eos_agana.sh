#!/bin/bash

#Tool to use EOS flag in agana... agana's main need the first midas files.
#This script get it, and tells agana to get the next 99 files

#example:
#$ ./eos_agana.sh 1234 "--time --recoff"
RUNNO=${1}
ARGS=${2}
if [ `echo "${RUNNO}" | wc -c` -gt 3 ]; then
  echo "Running for RUNNO=${RUNNO}"
else
  echo "Give me a run number!"
  exit
fi

if [ `echo "${AGRELEASE}" | wc -c` -gt 3 ]; then
  echo "AGRELEASE set ok: $AGRELEASE"
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi
echo "eos cp /eos/experiment/ALPHAg/midasdata_old/run${RUNNO}sub000.mid.lz4 ${AGRELEASE}/ana/"
#eos cp /eos/experiment/ALPHAg/midasdata_old/run${RUNNO}sub000.mid.lz4 ${AGRELEASE}/ana/

cd ${AGRELEASE}/ana/
FILES=run${RUNNO}sub000.mid.lz4
for i in `seq 1 9`; do
   FILES="${FILES} run${RUNNO}sub00${i}.mid.lz4"
done
for i in `seq 10 99`; do
   FILES="${FILES} run${RUNNO}sub0${i}.mid.lz4"
done
echo "./agana.exe ${FILES} -- --EOS ${2} &> R${RUNNO}"
./agana.exe ${FILES} -- --EOS ${2} &> R${RUNNO}
