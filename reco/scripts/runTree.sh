#!/bin/bash


#ccomp.sh update

set -x
#time $AGRELEASE/build/reco/MainEventTree.exe -f $DATADIR/test/cosmics904139.root -p 1
#time $AGRELEASE/build/reco/MainEventTree.exe -f $DATADIR/test/cosmics903838.root -p 1
time $AGRELEASE/build/reco/MainEventTree.exe -f $DATADIR/test/cosmics904026.root -p 1
#gdb -ex=r --args $AGRELEASE/build/reco/MainEventTree.exe -f $DATADIR/test/cosmics904139.root -p 1
