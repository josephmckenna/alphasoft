#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x



# Run 3873
echo "Starting Run 3873"
#{ time agana.exe -O$DATADIR/output/cosmics3873_newcyl.root --mt $AGMIDASDATA/run03873sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib ; } &> RunLogs/R3873.log &
#{ time agana.exe -O$DATADIR/test/cosmics3873.root --mt $AGMIDASDATA/run03873sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l0.json --Bfield 0 --calib ; } &> RunLogs/R3873.log &
#agana.exe -O$DATADIR/test/cosmics3873.root --mt $AGMIDASDATA/run03873sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l0.json --Bfield 0 --calib &> RunLogs/R3873.log &
#{ time agana.exe -O$DATADIR/test/cosmics3873_10subs.root --mt $AGMIDASDATA/run03873sub00*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l0.json --Bfield 0 --calib ; } &> RunLogs/R3873.log &
agana.exe -O$DATADIR/test/cosmics3873.root --mt $AGMIDASDATA/run03873sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib &> RunLogs/R3873.log &


# Run 3879
echo "Starting Run 3879"
#{ time agana.exe -O$DATADIR/output/cosmics3879_newcyl.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib ; } &> RunLogs/R3879.log &
#{ time agana.exe -O$DATADIR/test/cosmics3879.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l0.json --Bfield 0 --calib ; } &> RunLogs/R3879.log &
#{ time agana.exe -O$DATADIR/test/cosmics3879.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l0.json --Bfield 0 --calib ; } &> RunLogs/R3879.log &
#{ time agana.exe -O$DATADIR/test/cosmics3879.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cosm.json --Bfield 0 --calib ; } &> RunLogs/R3879.log &
agana.exe -O$DATADIR/test/cosmics3879.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib &> RunLogs/R3879.log &



