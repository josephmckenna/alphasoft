#!/bin/bash

# simulation !!!
{ time AGTPC runHeedInterface.mac ; } &> HeedInterface_driftAval.log &
PID=$!

wait $PID

# analysis
cd $AGRELEASE/reco
g4ana.exe -f $MCDATA/outAgTPC_det_B1.00T_Q30_HeedInterface_driftAval.root
