#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x

### horizontal - run 903838. 61 PWB minus 3 SCAs. 396 subruns, about 1M events. K.O.
#agana.exe -O$DATADIR/agmini/cosmics903838.root --mt $MIDASDATA/run903838sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R903838.log &

##################################################################################################################################################################################################


### vertical - AW: 3.2kV - Ar-CO2 70:30
#agana.exe -O$DATADIR/test/cosmics904139.root --mt $MIDASDATA/run904139sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904139.log&
#agana.exe -O$DATADIR/test/cosmics904139sub000.root --mt $MIDASDATA/run904139sub000.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904139.log&

### vertical - Cosmic Run 904214 AW3.2kV, Trig. MLU1 - fastest?
#agana.exe -O$DATADIR/test/cosmics904214.root --mt $MIDASDATA/run904214sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904214.log&


### vertical - Cosmic Run 904234 AW3.1kV, Trig. MLU1
#agana.exe -O$DATADIR/test/cosmics904234.root --mt $MIDASDATA/run904234sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904234.log&

### vertical - Cosmic Run 904241 AW3.1kV, Trig. MLU1 
#agana.exe -O$DATADIR/test/cosmics904241.root --mt $MIDASDATA/run904241sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904241.log&


##################################################################################################################################################################################################
##################################################################################################################################################################################################


### horizontal - AW3.2kV - Trig. MLU1
#agana.exe -O$DATADIR/test/cosmics904468.root --mt $MIDASDATA/run904468sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904468.log&


##################################################################################################################################################################################################
