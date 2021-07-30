#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x

ccomp.sh build nosim
ccomp.sh install

for twA in $(seq 0.0000000012 0.0000000001 0.0000000022)
do
      protoTOF.exe -O$DATADIR/gareth/run904810A${twA}.root --mt $MIDASDATA/run904810sub000.mid.lz4 -- --bscprint --bscProtoTOF --twA ${twA} &> RunLogs/R904810A${twA}.log &
done
