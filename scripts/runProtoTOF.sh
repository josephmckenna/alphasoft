#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x

ccomp.sh build nosim
ccomp.sh install

#protoTOF.exe -O$DATADIR/gareth/run904799.root --mt $MIDASDATA/run904799sub000.mid.lz4 -- --bscprint --bscpulser --bscProtoTOF --bscoffsetfile run799.calib &> RunLogs/R904799.log &
#protoTOF.exe -O$DATADIR/gareth/run904800.root --mt $MIDASDATA/run904800sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904800.log &
#protoTOF.exe -O$DATADIR/gareth/run904801.root --mt $MIDASDATA/run904801sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904801.log &
#protoTOF.exe -O$DATADIR/gareth/run904803.root --mt $MIDASDATA/run904803sub000.mid.lz4 -- --bscprint --bscpulser --bscProtoTOF --bscoffsetfile run803.calib &> RunLogs/R904803.log &
#protoTOF.exe -O$DATADIR/gareth/run904804.root --mt $MIDASDATA/run904804sub000.mid.lz4 -- --bscprint --bscpulser --bscProtoTOF --bscoffsetfile run804.calib &> RunLogs/R904804.log &
#protoTOF.exe -O$DATADIR/gareth/run904805.root --mt $MIDASDATA/run904805sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904805.log &
#protoTOF.exe -O$DATADIR/gareth/run904806.root --mt $MIDASDATA/run904806sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904806.log &
#protoTOF.exe -O$DATADIR/gareth/run904807.root --mt $MIDASDATA/run904807sub000.mid.lz4 -- --bscprint --bscpulser --bscProtoTOF --bscoffsetfile run807.calib &> RunLogs/R904807.log &
#protoTOF.exe -O$DATADIR/gareth/run904808.root --mt $MIDASDATA/run904808sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904808.log &
#protoTOF.exe -O$DATADIR/gareth/run904809.root --mt $MIDASDATA/run904809sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904809.log &
#protoTOF.exe -O$DATADIR/gareth/run904810.root --mt $MIDASDATA/run904810sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904810.log &
#protoTOF.exe -O$DATADIR/gareth/run904811.root --mt $MIDASDATA/run904811sub000.mid.lz4 -- --bscprint --bscProtoTOF &> RunLogs/R904811.log &
#protoTOF.exe -O$DATADIR/gareth/run904752.root --mt $MIDASDATA/run904752sub000.mid.lz4 -- --bscprint --bscpulser --bscProtoTOF --bscoffsetfile run752.calib &> RunLogs/R904752.log &

protoTOF.exe -O$DATADIR/gareth/run3873.root --mt $AGMIDASDATA/run03873sub000.mid.lz4 -- --bscprint &> RunLogs/R3873.log &
