#!/bin/bash

if [ ! -d "$AGRELEASE/RunLogs" ]; then
    mkdir -p $AGRELEASE/RunLogs
fi

set -x

#agana.exe -O$DATADIR/output/pbar3725.root --mt $AGMIDASDATA/run03725sub*.mid.lz4 -- --usetimerange 264.316 567.379 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3725cosm.log &
agana.exe -O$DATADIR/cosmics/pbar3725.root --mt $AGMIDASDATA/run03725sub*.mid.lz4 -- --usetimerange 264.316 567.379 --diag --anasettings $AGRELEASE/ana/cosm2.json  &> $AGRELEASE/RunLogs/R3725cosm2.log &


#agana.exe -O$DATADIR/output/pbar3780.root --mt $AGMIDASDATA/run03780sub*.mid.lz4 -- --usetimerange 372.023 577.757 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3780cosm.log &
agana.exe -O$DATADIR/cosmics/pbar3780.root --mt $AGMIDASDATA/run03780sub*.mid.lz4 -- --usetimerange 372.023 577.757 --diag --anasettings $AGRELEASE/ana/cosm2.json &> $AGRELEASE/RunLogs/R3780cosm2.log &


#agana.exe -O$DATADIR/output/pbar3781.root --mt $AGMIDASDATA/run03781sub*.mid.lz4 -- --usetimerange 182.243 386.789 --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3781cosm.log &
agana.exe -O$DATADIR/cosmics/pbar3781.root --mt $AGMIDASDATA/run03781sub*.mid.lz4 -- --usetimerange 182.243 386.789 --diag --anasettings $AGRELEASE/ana/cosm2.json&> $AGRELEASE/RunLogs/R3781cosm2.log &
