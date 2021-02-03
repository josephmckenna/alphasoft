#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x


#agana.exe -O$DATADIR/test/cosmics904501sub000.root --mt $MIDASDATA/run904501sub000.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --location TRIUMF --persistency 0 | tee RunLogs/R904501.log
#gdb -ex=r --args agana.exe -O$DATADIR/test/cosmics904501sub000.root --mt $MIDASDATA/run904501sub000.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --location TRIUMF --persistency 0


#agana.exe -O$DATADIR/test/cosmics904514sub000.root --mt $MIDASDATA/run904514sub000.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --location TRIUMF --persistency 0 | tee RunLogs/R904501.log


# agana.exe -O$DATADIR/test/pulser904519sub000.root --mt $MIDASDATA/run904519sub000.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF --persistency 0 | tee RunLogs/R904519.log
#gdb -ex=r --args agana.exe -O$DATADIR/test/pulser904519sub000.root --mt $MIDASDATA/run904519sub000.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF --persistency 0

#agana.exe -O$DATADIR/test/pulser904531sub000.root --mt $MIDASDATA/run904531sub000.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF --persistency 0 | tee RunLogs/R904531.log


#agana.exe -O$DATADIR/test/pulser904559sub000.root --mt $MIDASDATA/run904559sub000.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF --persistency 0 | tee RunLogs/R904559.log


#agana.exe -O$DATADIR/test/pulser904582.root --mt $MIDASDATA/run904582sub000.mid.lz4 -- --diag --persistency 0 &> RunLogs/R904582.log &
#agana.exe -O$DATADIR/test/pulser904583.root --mt $MIDASDATA/run904583sub000.mid.lz4 -- --diag --persistency 0 &> RunLogs/R904583.log &
#agana.exe -O$DATADIR/test/pulser904585.root --mt $MIDASDATA/run904585sub000.mid.lz4 -- --diag --persistency 0 &> RunLogs/R904585.log &
#agana.exe -O$DATADIR/test/pulser904586.root --mt $MIDASDATA/run904586sub000.mid.lz4 -- --diag --persistency 0 &> RunLogs/R904586.log &
#agana.exe -O$DATADIR/test/pulser904588.root --mt $MIDASDATA/run904588sub000.mid.lz4 -- --diag --persistency 0 &> RunLogs/R904588.log &
#agana.exe -O$DATADIR/test/pulser904589.root --mt $MIDASDATA/run904589sub000.mid.lz4 -- --diag --persistency 0 &> RunLogs/R904589.log &
#agana.exe -O$DATADIR/test/pulser904590.root --mt $MIDASDATA/run904590sub000.mid.lz4 -- --diag --persistency 0 &> RunLogs/R904590.log &
#agana.exe -O$DATADIR/test/pulser904591.root --mt $MIDASDATA/run904591sub00*.mid.lz4 -- --diag --persistency 0 &> RunLogs/R904591.log 

#root -q -b $DATADIR/test/pulser904582.root ana/macros/andrea/persistency_plot.C+ &
#root -q -b $DATADIR/test/pulser904583.root ana/macros/andrea/persistency_plot.C+ &
#root -q -b $DATADIR/test/pulser904585.root ana/macros/andrea/persistency_plot.C+ &
#root -q -b $DATADIR/test/pulser904586.root ana/macros/andrea/persistency_plot.C+ &
#root -q -b $DATADIR/test/pulser904588.root ana/macros/andrea/persistency_plot.C+ &
#root -q -b $DATADIR/test/pulser904589.root ana/macros/andrea/persistency_plot.C+ &
#root -q -b $DATADIR/test/pulser904590.root ana/macros/andrea/persistency_plot.C+ &
#root -q -b $DATADIR/test/pulser904591.root ana/macros/andrea/persistency_plot.C+ &

#agana.exe -O$DATADIR/test/pulser904593.root --mt $MIDASDATA/run904593sub00*.mid.lz4 -- --diag --persistency 0 &> RunLogs/R904593.log 
#root -q -b $DATADIR/test/pulser904593.root ana/macros/andrea/persistency_plot.C+

#agana.exe -O$DATADIR/test/pulser904596.root --mt $MIDASDATA/run904596sub00*.mid.lz4 -- --diag --persistency 0 &> RunLogs/R904596.log 
#root -q -b $DATADIR/test/pulser904596.root ana/macros/andrea/persistency_plot.C+

agana.exe -O$DATADIR/test/pulser904618.root --mt $MIDASDATA/run904618sub00*.mid.lz4 -- --diag --persistency 0 &> RunLogs/R904618.log 
root -q -b $DATADIR/test/pulser904618.root ana/macros/andrea/persistency_plot.C+
