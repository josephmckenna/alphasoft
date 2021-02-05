#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x

ccomp.sh build nosim
ccomp.sh install

#protoTOF.exe -O$DATADIR/gareth/run904497.root --mt $MIDASDATA/run904497sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904497.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904499.root --mt $MIDASDATA/run904499sub000.mid.lz4 -- --bscprint --bscpulser --bscoffsetfile bscoffsets1.calib &> RunLogs/R904499.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904510.root --mt $MIDASDATA/run904510sub000.mid.lz4 -- --bscprint --bscpulser --bscoffsetfile bscoffsets2.calib &> RunLogs/R904510.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904512.root --mt $MIDASDATA/run904512sub000.mid.lz4 -- --bscprint --bscpulser --bscoffsetfile bscoffsets3.calib &> RunLogs/R904512.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904522.root --mt $MIDASDATA/run904522sub000.mid.lz4 -- --bscprint --bscpulser --bscoffsetfile bscoffsets4.calib &> RunLogs/R904522.log & 
#protoTOF.exe -O$DATADIR/gareth/run904528.root --mt $MIDASDATA/run904528sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904528.log & 
#protoTOF.exe -O$DATADIR/gareth/tofrun904542.root --mt $MIDASDATA/run904542sub000.mid.lz4 -- --bscprint --bscpulser --bscProtoTOF --bscoffsetfile bscoffsets5.calib &> RunLogs/R904542.log & 

#protoTOF.exe -O$DATADIR/gareth/run904694.root --mt $MIDASDATA/run904694sub000.mid.lz4 -- --bscprint --bscProtoTOF --bscpulser --bscoffsetfile normal.calib &> RunLogs/R904694.log & 
#protoTOF.exe -O$DATADIR/gareth/run904695.root --mt $MIDASDATA/run904695sub000.mid.lz4 -- --bscprint --bscProtoTOF --bscpulser --bscoffsetfile disconnected.calib &> RunLogs/R904695.log & 
#protoTOF.exe -O$DATADIR/gareth/run904696.root --mt $MIDASDATA/run904696sub000.mid.lz4 -- --bscprint --bscProtoTOF --bscpulser --bscoffsetfile rtmswap.calib &> RunLogs/R904696.log & 
#protoTOF.exe -O$DATADIR/gareth/run904697.root --mt $MIDASDATA/run904697sub000.mid.lz4 -- --bscprint --bscProtoTOF --bscpulser --bscoffsetfile asdswap.calib &> RunLogs/R904697.log & 


#protoTOF.exe -O$DATADIR/gareth/run904723.root --mt $MIDASDATA/run904723sub000.mid.lz4 -- --bscprint --bscpulser --bscProtoTOF --bscoffsetfile new.calib &> RunLogs/R904723.log &
protoTOF.exe -O$DATADIR/gareth/run904725.root --mt $MIDASDATA/run904725sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904725.log &
#protoTOF.exe -O$DATADIR/gareth/run904727s000.root --mt $MIDASDATA/run904727sub000.mid.lz4 -- --bscprint --bscProtoTOF --bscpulser &> RunLogs/R904727s000.log &
#protoTOF.exe -O$DATADIR/gareth/run904727s001.root --mt $MIDASDATA/run904727sub001.mid.lz4 -- --bscprint --bscProtoTOF --bscpulser &> RunLogs/R904727s001.log &
#protoTOF.exe -O$DATADIR/gareth/run904727s002.root --mt $MIDASDATA/run904727sub002.mid.lz4 -- --bscprint --bscProtoTOF --bscpulser &> RunLogs/R904727s002.log &

