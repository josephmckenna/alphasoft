#!/bin/bash

if [ ! -d "$AGRELEASE/RunLogs" ]; then
    mkdir -p $AGRELEASE/RunLogs
fi



ana()
{
    IDX=$1
    for SEED in 10081985 18061985 3092016 26092019 28091956 18031956 20051985 23061990 17122013 30112015; do
	#F="$DATADIR/MCdata_oldsim/outAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30_multirun${IDX}_seed${SEED}.root"
	F="$MCDATA/outAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30_multirun${IDX}_seed${SEED}_v2021.root"
	ls -lh $F
	if [[ -f "$F" ]]; then
	    s=$(wc -c <"$F")
	    sMb=$(echo "scale=2; $s/1024/1024" | bc -l)
	    echo "`basename $F` size is $sMb Mb"
	else
	    continue
	fi
	
	LOGNAME="$AGRELEASE/RunLogs/Rec"$(basename $F .root)".log"
	echo "logfile: ${LOGNAME}"

	cmd="g4ana.exe -f $F -a $AGRELEASE/ana/oldsim.hjson"

	{ time $cmd; } &> ${LOGNAME} &
	CURRENT_PID=$!
	echo "exectuing: ${cmd} with PID: ${CURRENT_PID}"
	IDX=$((IDX+1))
	echo ""
    done
}

RUNNO=1
#while [ $RUNNO -le 1000 ]; do
#ana $RUNNO
#done

merge_rootfiels(){
    FILE_LIST=""
    RUNNO=1
    for SEED in 10081985 18061985 3092016 26092019 28091956 18031956 20051985 23061990 17122013 30112015; do
	FILE_LIST="anaoutAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30_multirun${RUNNO}_seed${SEED}_v2021.root $FILE_LIST"
	RUNNO=$((RUNNO+1))
    done
    echo $FILE_LIST
    hadd -ff anaoutAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30_multirun_v2021.root $FILE_LIST
}

merge_rootfiels
