#!/bin/bash

LAST=39
if [ $# -eq 1 ]; then
    LAST=$1
fi
    

echo "You are seeing the Analysis Report of the last ${LAST} runs" > $AGRELEASE/RunLogs/temptail.log

N=1
for LOG in `ls -rt $AGRELEASE/RunLogs/R*.log | tail -${LAST}`; do 

    echo $N $LOG >> $AGRELEASE/RunLogs/temptail.log
    tail -23 $LOG >> $AGRELEASE/RunLogs/temptail.log

    N=$((N+1))

done

more $AGRELEASE/RunLogs/temptail.log
