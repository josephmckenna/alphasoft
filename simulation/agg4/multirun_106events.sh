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

yseed=(2352 3203 6114 19386 30958 32157 28149 11392 25180 22144 16019 30670 6132 25337 31059 22175 10042 3037 4798 19163 3243 32333 14406 14875 32057 26645 31399 6485 24738 28422 10185 9011 4779 23502 30812 7879 25127 6336 1527 16075 17516 16268 19819 20088 31285 28081 9279 24679 20957 12633 26760 29711 22449 32739 20679 28029 31716 30665 28924 19760 23472 19793 17241 25534 29695 17950 31181 19602 20854 26955 2068 2942 7643 16570 16055 4723 31322 21135 26809 5860 11691 4752 25299 17751 2506 23136 30799 20550 17911 5323 30580 32059 23211 16826 27718 2436 27837 9846 10888 1635)
RUNNO=11
for YS in "${yseed[@]}"; do
    for SEED in 10081985 18061985 3092016 26092019 28091956 18031956 20051985 23061990 17122013 30112015; do
	echo "/control/verbose 2" > run${RUNNO}_${SEED}.mac
	echo "/AGTPC/setRunName multirun${RUNNO}_seed${SEED}${NAME}" >> run${RUNNO}_${SEED}.mac
	echo "/random/setSeeds ${SEED} ${YS}" >> run${RUNNO}_${SEED}.mac
	echo "/run/beamOn 1000" >> run${RUNNO}_${SEED}.mac

	echo "@@@ SEED   ${SEED}   ${YS}" > RunLogs/multiAGTPCrun${RUNNO}_${SEED}.log
	echo "@@@ Run # ${RUNNO}" >> RunLogs/multiAGTPCrun${RUNNO}_${SEED}.log
	echo `hostname` >> RunLogs/multiAGTPCrun${RUNNO}_${SEED}.log
	echo `pwd` >> RunLogs/multiAGTPCrun${RUNNO}_${SEED}.log
	{ time agg4 run${RUNNO}_${SEED}.mac ; } >> RunLogs/multiAGTPCrun${RUNNO}_${SEED}.log 2>&1 &
	lPID=$!

	#gedit RunLogs/multiAGTPCrun${RUNNO}_${SEED}.log &> /dev/null &
	RUNNO=$((RUNNO+1))
    done
    wait $lPID
done

# 
