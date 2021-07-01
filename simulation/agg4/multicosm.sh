#!/bin/bash

if [ ! -d "$AGRELEASE/RunLogs" ]; then
    mkdir -p $AGRELEASE/RunLogs
fi

# DEFAULT SETTINGS
# pad time
echo "16" > $AGRELEASE/simulation/agg4/settings.dat
# anode time
echo "16" >> $AGRELEASE/simulation/agg4/settings.dat
# magnetic field
echo "0" >> $AGRELEASE/simulation/agg4/settings.dat
# quencher fraction
echo "0.3" >> $AGRELEASE/simulation/agg4/settings.dat
# material
echo "1" >> $AGRELEASE/simulation/agg4/settings.dat
# prototype
echo "0" >> $AGRELEASE/simulation/agg4/settings.dat

#gedit settings.dat &> /dev/null &

NAME=""

#ySEED=28115
ySEED=95628
RUNNO=11
for SEED in 10081985 18061985 3092016 26092019 28091956 18031956 20051985 23061990 17122013 30112015; do
    echo "/control/verbose 2" > run${RUNNO}_${SEED}.mac
    echo "/AGTPC/setRunType 2" >> run${RUNNO}_${SEED}.mac
    echo "/AGTPC/setZcenter 1.2 m" >> run${RUNNO}_${SEED}.mac
    echo "/AGTPC/setRunName cosmrun${RUNNO}_seed${SEED}${NAME}" >> run${RUNNO}_${SEED}.mac
    echo "/random/setSeeds ${SEED} ${ySEED}" >> run${RUNNO}_${SEED}.mac
    echo "/run/beamOn 100000" >> run${RUNNO}_${SEED}.mac

    echo "@@@ SEED   ${SEED}   ${ySEED}" > $AGRELEASE/RunLogs/cosmAGTPCrun${RUNNO}_${SEED}.log
    echo "@@@ Run # ${RUNNO}" >> $AGRELEASE/RunLogs/cosmAGTPCrun${RUNNO}_${SEED}.log
    echo `hostname` >> $AGRELEASE/RunLogs/cosmAGTPCrun${RUNNO}_${SEED}.log
    echo `pwd` >> $AGRELEASE/RunLogs/cosmAGTPCrun${RUNNO}_${SEED}.log
    { time agg4 run${RUNNO}_${SEED}.mac ; } >> $AGRELEASE/RunLogs/cosmAGTPCrun${RUNNO}_${SEED}.log 2>&1 &

    #gedit $AGRELEASE/RunLogs/cosmAGTPCrun${RUNNO}_${SEED}.log &> /dev/null &
    RUNNO=$((RUNNO+1))
done

