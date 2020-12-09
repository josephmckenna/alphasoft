#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x

ccomp.sh build nosim
ccomp.sh install

protoTOF.exe -O$DATADIR/gareth/tofrun904497.root --mt $MIDASDATA/run904497sub000.mid.lz4 -- --bscprint &> RunLogs/R904497.log & 
protoTOF.exe -O$DATADIR/gareth/tofrun904499.root --mt $MIDASDATA/run904499sub000.mid.lz4 -- --bscprint --bscpulser --bscoffsetfile bscoffsets1.calib &> RunLogs/R904499.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904510.root --mt $MIDASDATA/run904510sub000.mid.lz4 -- --bscprint --bscpulser --bscoffsetfile bscoffsets2.calib &> RunLogs/R904510.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904512.root --mt $MIDASDATA/run904512sub000.mid.lz4 -- --bscprint --bscpulser --bscoffsetfile bscoffsets3.calib &> RunLogs/R904512.log & 
