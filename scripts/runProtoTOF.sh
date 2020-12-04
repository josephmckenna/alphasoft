#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x

time protoTOF.exe -O$DATADIR/gareth/tofrun904497.root --mt $MIDASDATA/run904497sub000.mid.lz4 -- --print &> RunLogs/R904497.log & 
