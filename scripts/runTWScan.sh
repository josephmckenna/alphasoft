#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x

ccomp.sh build nosim
ccomp.sh install

for twA in $(seq 0.0000000010 0.0000000001 0.0000000035)
do
      protoTOF.exe -O$DATADIR/gareth/scanA${twA}.root --mt $MIDASDATA/run904497sub000.mid.lz4 -- --bscprint --twA ${twA} &> RunLogs/R904497A${twA}.log & 
done
