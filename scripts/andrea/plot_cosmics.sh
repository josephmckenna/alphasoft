#!/bin/bash

set -x

#hadd -ff $DATADIR/test/cosmics38739.root $DATADIR/test/cosmics3873.root $DATADIR/test/cosmics3879.root
#time $AGRELEASE/build/reco/MainEventTree.exe -f $DATADIR/agmini/cosmics38739.root -p 1

#MainEventTree.exe --rootfile $DATADIR/test/cosmics3873.root --plot 1 --save 1 &> $AGRELEASE/RunLogs/recoR3873.log&
#MainEventTree.exe --rootfile $DATADIR/test/cosmics3879.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3879.log&

#MainEventTree.exe --rootfile $DATADIR/output/cosmics3863.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3863cosm.log&
#MainEventTree.exe --rootfile $DATADIR/output/cosmics3864.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3864cosm.log&
# cosm 1
MainEventTree.exe --rootfile $DATADIR/output/cosmics3865.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3865cosm.log&
MainEventTree.exe --rootfile $DATADIR/output/pbar3781.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3725cosm.log&
MainEventTree.exe --rootfile $DATADIR/output/pbar3780.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3780cosm.log&
MainEventTree.exe --rootfile $DATADIR/output/pbar3725.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3781cosm.log&

sleep 60

#MainEventTree.exe --rootfile $DATADIR/cosmics/cosmics3863.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3863ana.log&
#MainEventTree.exe --rootfile $DATADIR/cosmics/cosmics3864.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3864ana.log&
# cosm 2
MainEventTree.exe --rootfile $DATADIR/cosmics/cosmics3865.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3865ana.log&
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3781.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3725ana.log&
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3780.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3780ana.log&
MainEventTree.exe --rootfile $DATADIR/cosmics/pbar3725.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3781ana.log&

