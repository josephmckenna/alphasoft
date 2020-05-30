#!/bin/bash

if [ ! -d "./RunLogs" ]; then
    mkdir -p ./RunLogs
fi

RUNNO=1
while [ $RUNNO -le 50 ]; do
    for SEED in 10081985 18061985 3092016 26092019 28091956 18031956 20051985 23061990 17122013 30112015; do
	F="$DATADIR/outAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30_multirun${RUNNO}_seed${SEED}.root"
	ls -lh $F
	LOGNAME="./RunLogs/Rec"$(basename $F .root)".log"
	#echo ${LOGNAME}
	{ time ../g4ana.exe -f $F -a $AGRELEASE/ana/oldsim.hjson ; } &> ${LOGNAME} &
	echo "g4ana.exe -f $F -a $AGRELEASE/ana/oldsim.hjson"
	RUNNO=$((RUNNO+1))
    done
done

