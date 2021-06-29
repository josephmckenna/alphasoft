#!/bin/bash

if [ ! -d "$AGRELEASE/RunLogs" ]; then
    mkdir -p $AGRELEASE/RunLogs
fi

#MainEventTree.exe --rootfile $AGRELEASE/output04488.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/R4488plot.log &

#MainEventTree.exe --rootfile $AGRELEASE/output04491.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/R4491plot.log &



#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4513.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/R4513plot.log

MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4533.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/R4533plot.log

