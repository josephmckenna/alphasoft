#!/bin/bash

if [ ! -d "${AGRELEASE}/RunLogs" ]; then
    mkdir -p $AGRELEASE/RunLogs
fi


cd $AGRELEASE/bin/simulation
time TrackElectronAvalanche.exe 1 0 0 0.5 |& tee $AGRELEASE/RunLogs/TrackGarf_0B_theta0.5.log

#time TrackElectronAvalanche.exe 1 0 0 -0.5 |& tee $AGRELEASE/RunLogs/TrackGarf_0B_theta-0.5.log
