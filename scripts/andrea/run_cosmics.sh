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
#agana.exe -O$DATADIR/test/cosmics3873.root --mt $AGMIDASDATA/run03873sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib &> RunLogs/R3873.log &
#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics3873comm.root --mt run03873sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --Bfield 0 &> $AGRELEASE/RunLogs/R3873comm.log &
#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics3873cyl.root --mt run03873sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 &> $AGRELEASE/RunLogs/R3873cyl.log &
#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics3873test.root --mt run03873sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/test.json --Bfield 0 &> $AGRELEASE/RunLogs/R3873test.log &
#agana.exe -O$DATADIR/test/cosmics3873.root --mt $AGMIDASDATA/run03873sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_0.json --Bfield 0 --calib &> RunLogs/R3873.log &
agana.exe -O$DATADIR/test/cosmics3873.root --mt $AGMIDASDATA/run03873sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_1.json --Bfield 0 --calib &> RunLogs/R3873.log &

# Run 3879
echo "Starting Run 3879"
#{ time agana.exe -O$DATADIR/output/cosmics3879_newcyl.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib ; } &> RunLogs/R3879.log &
#{ time agana.exe -O$DATADIR/test/cosmics3879.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l0.json --Bfield 0 --calib ; } &> RunLogs/R3879.log &
#{ time agana.exe -O$DATADIR/test/cosmics3879.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l0.json --Bfield 0 --calib ; } &> RunLogs/R3879.log &
#{ time agana.exe -O$DATADIR/test/cosmics3879.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cosm.json --Bfield 0 --calib ; } &> RunLogs/R3879.log &
#agana.exe -O$DATADIR/test/cosmics3879.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib &> RunLogs/R3879.log &
#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics3879comm.root --mt run03879sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --Bfield 0 &> $AGRELEASE/RunLogs/R3879comm.log &
#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics3879cyl.root --mt run03879sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 &> $AGRELEASE/RunLogs/R3873cyl.log &
#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics3879test.root --mt run03879sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/test.json --Bfield 0 &> $AGRELEASE/RunLogs/R3879test.log &
#agana.exe -O$DATADIR/test/cosmics3879.root --mt $AGMIDASDATA/run03879sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_0.json --Bfield 0 --calib &> RunLogs/R3879.log &
