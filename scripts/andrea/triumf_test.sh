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
#agana.exe -O$DATADIR/test/cosmics904468sub000.root --mt $MIDASDATA/run904468sub000.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904468sub000.log&


###########################################################################################################################################    #######################################################



### horizontal no sleeve - AW3.2kV - Trig. MLU1 - T14/15 up 
#agana.exe -O$DATADIR/test/cosmics904562.root --mt $MIDASDATA/run904562sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904562.log&


### horizontal no sleeve - AW3.2kV - Trig. MLU1 - T13/14 up 
#agana.exe -O$DATADIR/test/cosmics904565.root --mt $MIDASDATA/run904565sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904565.log&


### horizontal - AW3.2kV - Trig. MLU1 - T01 up 
#agana.exe -O$DATADIR/test/cosmics904569.root --mt $MIDASDATA/run904569sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904569.log&
#agana.exe -O$DATADIR/test/cosmics904569_10subs.root --mt $MIDASDATA/run904569sub00*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.5.json --Bfield 0 --calib --location TRIUMF | tee RunLogs/R904569.log


### horizontal - AW3.2kV - Trig. MLU1 - T00 up 
agana.exe -O$DATADIR/test/cosmics904577.root --mt $MIDASDATA/run904577sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904577.log&

