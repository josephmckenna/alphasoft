#!/bin/bash

set -x

#hadd -ff $DATADIR/test/cosmics38739.root $DATADIR/test/cosmics3873.root $DATADIR/test/cosmics3879.root
#time $AGRELEASE/build/reco/MainEventTree.exe -f $DATADIR/agmini/cosmics38739.root -p 1

MainEventTree.exe --rootfile $DATADIR/test/cosmics3873.root --plot 1 --save 1 &> $AGRELEASE/RunLogs/recoR3873.log&
#MainEventTree.exe --rootfile $DATADIR/test/cosmics3879.root --plot 0 --save 1 &> $AGRELEASE/RunLogs/recoR3879.log&
