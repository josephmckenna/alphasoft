#!/bin/bash

if [ ! -d "$AGRELEASE/RunLogs" ]; then
    mkdir -p $AGRELEASE/RunLogs
fi

#agana.exe --mt run04488sub000.mid.lz4 -- --EOS --Bfield 0 --calib --diag --anasettings $AGRELEASE/ana/cosm.json |& tee $AGRELEASE/RunLogs/R4488.log

#agana.exe --mt run04491sub000.mid.lz4 -- --EOS --Bfield 0 --calib --diag --anasettings $AGRELEASE/ana/cosm.json &> $AGRELEASE/RunLogs/R4491.log &



#nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4513.root --mt run04513sub000.mid.lz4 -- --EOS --Bfield 0 --calib --diag --anasettings $AGRELEASE/ana/comm.json &> $AGRELEASE/RunLogs/R4513.log &

nohup agana.exe -O/mnt/fast_disk/andrea/cosmics4533.root --mt run04533sub000.mid.lz4 -- --EOS --Bfield 0 --calib --diag --anasettings $AGRELEASE/ana/comm.json &> $AGRELEASE/RunLogs/R4533.log &


