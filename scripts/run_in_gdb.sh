#/bin/bash

#Set default run number:

echo "Usage:"
echo "./run_in_gdb.sh -p agana.exe -r 45000 -e AG -l 1500 -m \"--argumentformodule a --otherarg b\""
echo "-r 12345      Set run number"
echo "-e [string]   Experiment (AG or A2)"
echo "-l 1500       MIDAS Event limit"
while getopts p:r:l:m:e: option
do
case "${option}"
in
p) PROG=${OPTARG};;
r) RUNNO=${OPTARG};;
l) LIMITEVENTS=${OPTARG};;
e) EXPERIMENT_TYPE=${OPTARG};;
m) MODULEFLAGS=${OPTARG};;
esac
done


if [ -h ${PROG} ]; then
    echo "FATAL: You must give me a program to run(-p agana.exe)"
    exit
fi
if [ -h ${RUNNO} ]; then
    echo "FATAL: You must give me a run number (-r 12345)"
    exit
fi
if [ -h ${EXPERIMENT_TYPE} ]; then
    echo "FATAL: You must give me a experiment name (-e A2 or -e AG)"
    exit
fi

if [ -h ${MODULEFLAGS} ]; then
   echo "No module flags set. OK"
else
   MODULEFLAGS="-- ${MODULEFLAGS}"
fi

FILENAME=""
if [ ${EXPERIMENT_TYPE} == "AG" ]; then
    FILENAME="run${RUNNO}sub000.mid.lz4"
    if [ ! -f ${AGRELEASE}/${FILENAME}  ]; then
        eos cp /eos/experiment/ALPHAg/midasdata_old/${FILENAME} ${AGRELEASE}/
    else
        echo "${FILENAME} found locally"
    fi
else
    if [ ${EXPERIMENT_TYPE} == "A2" ]; then
        FILENAME="run${RUNNO}sub00000.mid.gz"
        if [ ! -f ${AGRELEASE}/${FILENAME}  ]; then
            eos cp /eos/experiment/alpha/midasdata/${FILENAME} ${AGRELEASE}/
        else
            echo "${FILENAME} found locally"
        fi
    else
        echo "Invalid experiment type"
        exit
    fi
fi

gdb -ex run -ex where -batch  -return-child-result --args ${PROG} ${FILENAME} ${MODULEFLAGS}
