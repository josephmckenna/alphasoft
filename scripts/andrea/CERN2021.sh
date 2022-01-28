#!/bin/bash

if [ ! -d "$AGRELEASE/RunLogs" ]; then
    mkdir -p $AGRELEASE/RunLogs
fi

#agana.exe --mt run04488sub000.mid.lz4 -- --EOS --Bfield 0 --calib --diag --anasettings $AGRELEASE/ana/cosm.json |& tee $AGRELEASE/RunLogs/R4488.log

#agana.exe --mt run04491sub000.mid.lz4 -- --EOS --Bfield 0 --calib --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R4491.log &



#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4513.root --mt run04513sub000.mid.lz4 -- --EOS --Bfield 0 --calib --diag --anasettings $AGRELEASE/ana/comm.json &> $AGRELEASE/RunLogs/R4513.log &
#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4513calib.root --mt run04513sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --loadcalib &> $AGRELEASE/RunLogs/R4513calib.log &
#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4513newstr.root --mt run04513sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --Bfield 0 &> $AGRELEASE/RunLogs/R4513newstr.log &

#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4533.root --mt run04533sub000.mid.lz4 -- --EOS --Bfield 0 --calib --diag --anasettings $AGRELEASE/ana/comm.json &> $AGRELEASE/RunLogs/R4533.log &
#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4533calib.root --mt run04533sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --loadcalib &> $AGRELEASE/RunLogs/R4533calib.log &
#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4533newstr.root --mt run04533sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --Bfield 0 &> $AGRELEASE/RunLogs/R4533newstr.log &


#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4541.root --mt run04541sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --Bfield 0 &> $AGRELEASE/RunLogs/R4541.log &
#agana.exe -O/mnt/fast_disk/andrea/cosmics4541.root --mt run04541sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --Bfield 0 |& tee $AGRELEASE/RunLogs/R4541.log


#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4548.root --mt run04548sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --Bfield 0 &> $AGRELEASE/RunLogs/R4548.log &

#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4553.root --mt run04553sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --Bfield 0 &> $AGRELEASE/RunLogs/R4553.log &

#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4574.root --mt run04574sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --Bfield 0 &> $AGRELEASE/RunLogs/R4574.log &

#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4576.root --mt run04576sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --Bfield 0 &> $AGRELEASE/RunLogs/R4576.log &

#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4579.root --mt run04579sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm.json --Bfield 0 &> $AGRELEASE/RunLogs/R4579.log &
#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4579.root --mt run04579sub000.mid.lz4 -- --EOS --calib --diag --anasettings $AGRELEASE/ana/comm2.json --Bfield 0 &> $AGRELEASE/RunLogs/R4579.log &


#agana.exe -O$DATADIR/test/cosmics4593.root --mt $AGMIDASDATA/run04593sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib &> $AGRELEASE/RunLogs/R4593.log &
#agana.exe -O$DATADIR/test/cosmics4593comm.root --mt $AGMIDASDATA/run04593sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/comm.json --Bfield 0 --calib &> $AGRELEASE/RunLogs/R4593comm.log &
#agana.exe -O$DATADIR/test/cosmics4593.root --mt $AGMIDASDATA/run04593sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_0.json --Bfield 0 --calib &> $AGRELEASE/RunLogs/R4593.log &
#agana.exe -O$DATADIR/test/cosmics4593.root --mt $AGMIDASDATA/run04593sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_1.json --Bfield 0 --calib &> $AGRELEASE/RunLogs/R4593.log &


#agana.exe -O$DATADIR/test/cosmics4605.root --mt $AGMIDASDATA/run04605sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_1.json --Bfield 0 --calib |& tee $AGRELEASE/RunLogs/R4605.log

#agana.exe -O$DATADIR/test/cosmics4620_0.root --mt $AGMIDASDATA/run04620sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_0.json --Bfield 0 --calib &> $AGRELEASE/RunLogs/R4620.log&
#agana.exe -O$DATADIR/test/cosmics4620_2.root --mt $AGMIDASDATA/run04620sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_2.json --Bfield 0 --calib |& tee $AGRELEASE/RunLogs/R4620.log

#agana.exe -O$DATADIR/test/cosmics4815sub000.root --mt $AGMIDASDATA/run04815sub000.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_2.json --Bfield 0 --calib |& tee $AGRELEASE/RunLogs/R4815.log
#agana.exe -O$DATADIR/test/cosmics4815.root --mt $AGMIDASDATA/run04815sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_2.json --Bfield 0 --calib |& tee $AGRELEASE/RunLogs/R4815.log


#agana.exe -O$DATADIR/CERN2021/cosmics5151.root --mt $AGMIDASDATA/run05151sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_2.json |& tee $AGRELEASE/RunLogs/R5151.log
agana.exe -O$DATADIR/CERN2021/cosmics5151.root --mt $AGMIDASDATA/run05151sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_3.json &> tee $AGRELEASE/RunLogs/R5151.log&

agana.exe -O$DATADIR/CERN2021/cosmics5157.root --mt $AGMIDASDATA/run05157sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cern2021_3.json &> tee $AGRELEASE/RunLogs/R5157.log&
