#!/bin/bash

#PHI=`deg2rad 60`
#THETA=`deg2rad 30`


cd $AGRELEASE/bin/simulation
#TrackElectronAvalanche.exe 1 0 0 0.5 |& tee $AGRELEASE/RunLogs/TrackGarf_0B.log

time TrackElectronAvalanche.exe 1 0 0 -0.5 |& tee $AGRELEASE/RunLogs/TrackGarf_0B.log
