#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x

ccomp.sh build nosim
ccomp.sh install

protoTOF.exe -O$DATADIR/gareth/tofrun904497.root --mt $MIDASDATA/run904497sub000.mid.lz4 -- --bscprint --bscRunType 0 &> RunLogs/R904497.log & 
protoTOF.exe -O$DATADIR/gareth/tofrun904499.root --mt $MIDASDATA/run904499sub000.mid.lz4 -- --bscprint --bscRunType 1 &> RunLogs/R904499.log & 
