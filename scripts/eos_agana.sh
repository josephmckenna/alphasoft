#!/bin/bash

#Tool to use EOS flag in agana... agana's main need the first midas files.
#This script get it, and tells agana to get the next 99 files

#example:
#$ ./eos_agana.sh [-hn] 1234 "--time --recoff"

aganacmd=${AGRELEASE}/agana.exe

while getopts ":hn" opt; do
  case ${opt} in
    h ) # process option h
	  echo "Tool to use EOS flag in agana... agana's main need the first midas files."
	  echo "This script gets it, and tells agana to get the next 99 files"
	  echo 'Usage: ./eos_agana.sh [-hn] 1234 "--time --recoff"'
	  echo "Options:"
	  echo "   -h   display this help and exit"
	  echo "   -n   run agana_noreco.exe instead of agana.exe"
	  exit
      ;;
    n ) # process option n
	  aganacmd=$AGRELEASE/agana_noreco.exe
	  echo "Using analyzer command $aganacmd"
      ;;
    \? ) echo "Usage: cmd [-h] [-t]"
      ;;
  esac
done

shift $((OPTIND -1))

RUNNO=${1}
ARGS=${2}
if [ `echo "${RUNNO}" | wc -c` -gt 3 ]; then
  echo "Running for RUNNO=${RUNNO}"
else
  echo "Give me a run number!"
  exit
fi

if [ `echo "${RUNNO}" | wc -c` -eq 5 ]; then
  RUNNO="0${RUNNO}"
fi

if [ `echo "${AGRELEASE}" | wc -c` -gt 3 ]; then
  echo "AGRELEASE set ok: $AGRELEASE"
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi


#Fetch the first file... agana's main requires it to start... after that,
#the other files are fetched from EOS and deleted when they are finished with
FETCHED=0
if [ -f ${AGRELEASE}/ana/run${RUNNO}sub000.mid.lz4 ]; then
   echo "First file found... not fetching from EOS"
else
   echo "eos cp /eos/experiment/ALPHAg/midasdata_old/run${RUNNO}sub000.mid.lz4 ${AGRELEASE}/ana/"
   eos cp /eos/experiment/ALPHAg/midasdata_old/run${RUNNO}sub000.mid.lz4 ${AGRELEASE}/ana/
   FETCHED=1
fi

cd ${AGRELEASE}/ana/
FILES=run${RUNNO}sub000.mid.lz4
for i in `seq 1 9`; do
   FILES="${FILES} run${RUNNO}sub00${i}.mid.lz4"
done
for i in `seq 10 99`; do
   FILES="${FILES} run${RUNNO}sub0${i}.mid.lz4"
done
echo "$aganacmd ${FILES} -- --EOS ${2} 2>&1 | gzip > R${RUNNO}.gz"
$aganacmd ${FILES} -- --EOS ${2} 2>&1 | gzip > R${RUNNO}.gz
#echo "$aganacmd ${FILES} -- --EOS ${2}"
#$aganacmd ${FILES} -- --EOS ${2}

#Clean up if we fetched the file from EOS
if [ ${FETCHED} -eq 1 ]; then
   rm -v ${AGRELEASE}/ana/run${RUNNO}sub000.mid.lz4
fi
