#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi
set -x


### horizontal w/ Sleeve - AW3.2kV - Trig. MLU1   ---> SHORT
#agana.exe -O$DATADIR/test/cosmics904472.root --mt $MIDASDATA/run904472sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904472.log&

### horizontal w/ Sleeve - AW3.2kV - Trig. MLU1 
#agana.exe -O$DATADIR/test/cosmics904474.root --mt $MIDASDATA/run904474sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904474.log&


#################################################################################################################################################################################################
##################################################################################################################################################################################################


### vertical w/ Sleeve - AW3.2kV - Trig. MLU1 -- short
#agana.exe -O$DATADIR/test/cosmics904501.root --mt $MIDASDATA/run904501sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904501.log&

### vertical w/ Sleeve - AW3.2kV - Trig. MLU1 -- long
#agana.exe -O$DATADIR/test/cosmics904503.root --mt $MIDASDATA/run904503sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904503.log&


### vertical w/ Sleeve - AW3.2kV - Trig. MLU1 -- long
#agana.exe -O$DATADIR/test/cosmics904547.root --mt $MIDASDATA/run904547sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904547.log&


############################################################################################################################################################################################



### vertical w/ Sleeve - AW3.1kV - Trig. MLU1 -- long
#agana.exe -O$DATADIR/test/cosmics904508.root --mt $MIDASDATA/run904508sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904508.log&

### vertical w/ Sleeve - AW3.1kV - Trig. MLU1 -- short
#agana.exe -O$DATADIR/test/cosmics904513.root --mt $MIDASDATA/run904513sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904513.log&

### vertical w/ Sleeve - AW3.1kV - Trig. MLU1 -- very long
#agana.exe -O$DATADIR/test/cosmics904549.root --mt $MIDASDATA/run904549sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904549.log&



############################################################################################################################################################################################


### horizontal w/ sleeve - AW3.1kV - Trig. MLU1 - T10 up - short
#agana.exe -O$DATADIR/test/cosmics904554.root --mt $MIDASDATA/run904554sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904554.log&


############################################################################################################################################################################################

### horizontal w/ sleeve - AW3.2kV - Trig. MLU1 - T10 up - long
#agana.exe -O$DATADIR/test/cosmics904555.root --mt $MIDASDATA/run904555sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l2.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904555.log&




############################################################################################################################################################################################

### horizontal w/ Sleeve - AW3.1kV - Trig. MLU1 
#agana.exe -O$DATADIR/test/cosmics904619.root --mt $MIDASDATA/run904619sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib --location TRIUMF &> RunLogs/R904619.log&
gdb -ex=r --args agana.exe -O$DATADIR/test/cosmics904619.root --mt $MIDASDATA/run904619sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l1.5.json --Bfield 0 --calib --location TRIUMF
