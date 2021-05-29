#!/bin/bash

if [ ! -d "$AGRELEASE/RunLogs" ]; then
    mkdir -p $AGRELEASE/RunLogs
fi

#MainEventTree.exe --rootfile $AGRELEASE/output04488.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/R4488plot.log &

#MainEventTree.exe --rootfile $AGRELEASE/output04491.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/R4491plot.log &



#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4513.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/R4513plot.log

#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4533.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/R4533plot.log



#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4513calib.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/R4513calibplot.log

#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4533calib.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/R4533calibplot.log



#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4513newstr.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/R4513newstrplot.log

#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4533newstr.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/R4533newstrplot.log

#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4541.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/R4541.log


#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4548.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/R4548.log

#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4553.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/R4553.log


#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4574.root --plot 0 --save 1 |& tee $AGRELEASE/RunLogs/R4574.log&
#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4576.root --plot 0 --save 1 |& tee $AGRELEASE/RunLogs/R4576.log&

MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4579.root --plot 0 --save 1 |& tee $AGRELEASE/RunLogs/R4579.log&
