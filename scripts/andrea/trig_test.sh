#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

set -x


### vertical - AW: 3.2kV - Trig. AW MLU file 3
#agana.exe -O$DATADIR/agmini/cosmics904192.root --mt $MIDASDATA/run904192sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.json --Bfield 0 --calib --location TRIUMF | tee RunLogs/R904192.log


### vertical - Cosmic Run 904219 AW3.2kV, Trig. MLU0 scaledown '1'
#agana.exe -O$DATADIR/agmini/cosmics904219.root --mt $MIDASDATA/run904219sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l0.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904219.log&

### vertical - Cosmic Run 904245 AW3.1kV, Trig. MLU0, scaledown '1'
#agana.exe -O$DATADIR/agmini/cosmics904245.root --mt $MIDASDATA/run904245sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l0.json --Bfield 0 --calib --location TRIUMF | tee RunLogs/R904245.log


############################################################################################################################################################################################

### vertical w/ sleeve - AW3.2kV - Trig. MLU0 scaledown 2
#agana.exe -O$DATADIR/test/cosmics904507.root --mt $MIDASDATA/run904507sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l0.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904507.log&


############################################################################################################################################################################################


### vertical w/ sleeve - AW3.2kV - Trig. MLU3
#agana.exe -O$DATADIR/test/cosmics904548.root --mt $MIDASDATA/run904548sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904548.log&


############################################################################################################################################################################################


### vertical w/ sleeve - AW3.1kV - Trig. MLU3
#agana.exe -O$DATADIR/test/cosmics904550.root --mt $MIDASDATA/run904550sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904550.log&
