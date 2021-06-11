#!/bin/bash

if [ ! -d "$AGRELEASE/RunLogs" ]; then
    mkdir -p $AGRELEASE/RunLogs
fi

#MainEventTree.exe --rootfile $AGRELEASE/output04488.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/reco4488plot.log &

#MainEventTree.exe --rootfile $AGRELEASE/output04491.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/reco4491plot.log &



#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4513.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/reco4513plot.log

#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4533.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/reco4533plot.log



#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4513calib.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/reco4513calibplot.log

#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4533calib.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/reco4533calibplot.log



#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4513newstr.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/reco4513newstrplot.log

#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4533newstr.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/reco4533newstrplot.log

#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4541.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/reco4541.log


#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4548.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/reco4548.log

#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4553.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/reco4553.log


#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4574.root --plot 0 --save 1 |& tee $AGRELEASE/RunLogs/reco4574.log&
#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4576.root --plot 0 --save 1 |& tee $AGRELEASE/RunLogs/reco4576.log&

#MainEventTree.exe --rootfile /mnt/fast_disk/andrea/cosmics4579.root --plot 0 --save 1 |& tee $AGRELEASE/RunLogs/reco4579.log&

#MainEventTree.exe --rootfile $DATADIR/test/cosmics4593.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/Reco4593.log
#MainEventTree.exe --rootfile $DATADIR/test/cosmics4593comm.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/Reco4593.log


#MainEventTree.exe --rootfile $DATADIR/test/cosmics4605.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/reco4605.log&

MainEventTree.exe --rootfile $DATADIR/test/cosmics4620_0.root --plot 1 --save 1 |& tee $AGRELEASE/RunLogs/reco4620.log
