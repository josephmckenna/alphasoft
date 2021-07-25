#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x

# ============================================================================================================================================================

#echo "B=0T, trig: AW MLU1"
#echo "Starting Run 3873"
#agana.exe -O$DATADIR/test/cosmics3873.root --mt $AGMIDASDATA/run03873sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l0.json --Bfield 0 --calib &> RunLogs/R3873.log &
#agana.exe -O$DATADIR/test/cosmics3873.root --mt $AGMIDASDATA/run03873sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib &> RunLogs/R3873.log &
#agana.exe -O$DATADIR/test/cosmics3873.root --mt $AGMIDASDATA/run03873sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_0.json --Bfield 0 --calib &> RunLogs/R3873.log &
#agana.exe -O$DATADIR/test/cosmics3873.root --mt $AGMIDASDATA/run03873sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_1.json --Bfield 0 --calib &> RunLogs/R3873.log &

# ============================================================================================================================================================

#echo "Starting Run 3879"
#agana.exe -O$DATADIR/test/cosmics3879.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib &> RunLogs/R3879.log &
#agana.exe -O$DATADIR/test/cosmics3879.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_0.json --Bfield 0 --calib &> RunLogs/R3879.log &

# ============================================================================================================================================================
# ============================================================================================================================================================

echo "B=1T, trig: AW MLU1"
echo "Starting Run 3863"
agana.exe -O$DATADIR/output/cosmics3863.root --mt $AGMIDASDATA/run03863sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3863cosm.log &
agana.exe -O$DATADIR/cosmics/cosmics3863.root --mt $AGMIDASDATA/run03863sub*.mid.lz4 -- --diag &> $AGRELEASE/RunLogs/R3863ana.log &

# ============================================================================================================================================================

echo "Starting Run 3864"
agana.exe -O$DATADIR/output/cosmics3864.root --mt $AGMIDASDATA/run03864sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3864cosm.log &
agana.exe -O$DATADIR/cosmics/cosmics3864.root --mt $AGMIDASDATA/run03864sub*.mid.lz4 -- --diag &> $AGRELEASE/RunLogs/R3864ana.log &

# ============================================================================================================================================================

echo "Starting Run 3865"
agana.exe -O$DATADIR/output/cosmics3865.root --mt $AGMIDASDATA/run03865sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R3865cosm.log &
agana.exe -O$DATADIR/cosmics/cosmics3865.root --mt $AGMIDASDATA/run03865sub*.mid.lz4 -- --diag &> $AGRELEASE/RunLogs/R3865ana.log &

# ============================================================================================================================================================
