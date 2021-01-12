#!/bin/bash

if [ ! -d "./RunLogs" ]; then
    mkdir -p ./RunLogs
fi

RUNNO=1
while [ $RUNNO -le 1000 ]; do
    for SEED in 10081985 18061985 3092016 26092019 28091956 18031956 20051985 23061990 17122013 30112015; do
	F="$DATADIR/MCdata_oldsim/outAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30_multirun${RUNNO}_seed${SEED}.root"
	if [[ -f "$F" ]]; then
	    s=$(wc -c <"$F")
	    sMb=$(echo "scale=2; $s/1024/1024" | bc -l)
	    echo "`basename $F` size is $sMb Mb"
	else
	    continue
	fi
	LOGNAME="./RunLogs/Rec"$(basename $F .root)".log"
	#echo ${LOGNAME}
	cmd="$AGRELEASE/build/reco/g4ana.exe -f $F -a $AGRELEASE/ana/oldsim.hjson -l 1"
	{ time $cmd; } &> ${LOGNAME} &
	echo $cmd
	RUNNO=$((RUNNO+1))
	echo ""
    done
done

