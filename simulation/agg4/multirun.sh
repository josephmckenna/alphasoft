#!/bin/bash

if [ ! -d "./RunLogs" ]; then
    mkdir -p ./RunLogs
fi

# DEFAULT SETTINGS
# pad time
echo "16" > settings.dat
# anode time
echo "16" >> settings.dat
# magnetic field
echo "1" >> settings.dat
# quencher fraction
echo "0.3" >> settings.dat
# material
echo "1" >> settings.dat
# prototype
echo "0" >> settings.dat

#gedit settings.dat &> /dev/null &

NAME=""

ySEED=28115
RUNNO=1
for SEED in 10081985 18061985 3092016 26092019 28091956 18031956 20051985 23061990 17122013 30112015; do
    echo "/control/verbose 2" > run${RUNNO}_${SEED}.mac
    echo "/AGTPC/setRunName multirun${RUNNO}_seed${SEED}${NAME}" >> run${RUNNO}_${SEED}.mac
    echo "/random/setSeeds ${SEED} ${ySEED}" >> run${RUNNO}_${SEED}.mac
    echo "/run/beamOn 1000" >> run${RUNNO}_${SEED}.mac

    echo "@@@ SEED   ${SEED}   ${ySEED}" > RunLogs/multiAGTPCrun${RUNNO}_${SEED}.log
    echo "@@@ Run # ${RUNNO}" >> RunLogs/multiAGTPCrun${RUNNO}_${SEED}.log
    echo `hostname` >> RunLogs/multiAGTPCrun${RUNNO}_${SEED}.log
    echo `pwd` >> RunLogs/multiAGTPCrun${RUNNO}_${SEED}.log
    { time agg4 run${RUNNO}_${SEED}.mac ; } >> RunLogs/multiAGTPCrun${RUNNO}_${SEED}.log 2>&1 &

    gedit RunLogs/multiAGTPCrun${RUNNO}_${SEED}.log &> /dev/null &
    RUNNO=$((RUNNO+1))
done

