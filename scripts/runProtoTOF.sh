#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x

ccomp.sh build nosim
ccomp.sh install

protoTOF.exe -O$DATADIR/gareth/tofrun904497.root --mt $MIDASDATA/run904497sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904497.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904499.root --mt $MIDASDATA/run904499sub000.mid.lz4 -- --bscprint --bscpulser --bscoffsetfile bscoffsets1.calib &> RunLogs/R904499.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904510.root --mt $MIDASDATA/run904510sub000.mid.lz4 -- --bscprint --bscpulser --bscoffsetfile bscoffsets2.calib &> RunLogs/R904510.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904512.root --mt $MIDASDATA/run904512sub000.mid.lz4 -- --bscprint --bscpulser --bscoffsetfile bscoffsets3.calib &> RunLogs/R904512.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904522.root --mt $MIDASDATA/run904522sub000.mid.lz4 -- --bscprint --bscpulser --bscoffsetfile bscoffsets4.calib &> RunLogs/R904522.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904528.root --mt $MIDASDATA/run904528sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904528.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904542.root --mt $MIDASDATA/run904542sub000.mid.lz4 -- --bscprint --bscpulser --bscProtoTOF --bscoffsetfile bscoffsets5.calib &> RunLogs/R904542.log & 
