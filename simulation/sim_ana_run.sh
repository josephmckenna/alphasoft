#!/bin/bash

# simulation !!!
{ time AGTPC runHeedInterface.mac ; } &> HeedInterface_driftAval.log &
PID=$!

# analysis
cd g4ana
wait $PID
g4ana.exe $MCDATA/outAgTPC_det_B1.00T_Q30_HeedInterface_driftAval.root 1 1 5000 5000
