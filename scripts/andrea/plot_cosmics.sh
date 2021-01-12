#!/bin/bash

set -x

#hadd -ff $DATADIR/agmini/cosmics38739_newcyl.root $DATADIR/output/cosmics3873_newcyl.root $DATADIR/output/cosmics3879_newcyl.root
#time $AGRELEASE/build/reco/MainEventTree.exe -f $DATADIR/agmini/cosmics38739_newcyl.root -p 1

#time $AGRELEASE/build/reco/MainEventTree.exe -f $DATADIR/test/cosmics3873_10subs.root -p 1 -s 0
#rm -f ./statR3873.txt ./plots_R3873.root

#hadd -ff $DATADIR/test/cosmics38739.root $DATADIR/test/cosmics3873.root $DATADIR/test/cosmics3879.root
time $AGRELEASE/build/reco/MainEventTree.exe -f $DATADIR/agmini/cosmics38739.root -p 1
