#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

#set -x

### KO run - horizontal - AW: 3.2kV - all PWBs
echo "Starting Run 904014"
{ time agana.exe -O$DATADIR/agmini/cosmics904014.root --mt $MIDASDATA/run904014sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l3.json --Bfield 0 --calib ;} &> RunLogs/R904014.log &


### horizontal - AW: 3.2kV - 63+0.75 boards running
run_list=(903963 903965 903967 903990 903991)
comments=("T09up" "T13up" "T14up w/Supp" "T10up w/Supp" "T10up w/Supp")

for idx in ${!run_list[*]}; do
    run=${run_list[$idx]}
    printf "Starting Run   %s\t%s\n" ${run} "${comments[$idx]}"
    { time agana.exe -O$DATADIR/agmini/cosmics${run}.root --mt $MIDASDATA/run${run}sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l3.json --Bfield 0 --calib ;} &> RunLogs/R${run}.log 
done






### vertical - AW: 3.2kV - whatever gas
#{ time agana.exe -O$DATADIR/agmini/cosmics904026.root --mt $MIDASDATA/run904026sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l3.json --Bfield 0 --calib ;} &> RunLogs/R904026.log &
#root -q -b $DATADIR/agmini/cosmics904026.root ana/macros/ReadEventTree.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904026.root ana/macros/plotTPCdeformation.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904026.root ana/macros/plotPadSigma.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904026.root ana/macros/padplot.C+ &> /dev/null &


### vertical - AW: 3.2kV - whatever gas - sca gain 1-0
#{ time agana.exe -O$DATADIR/agmini/cosmics904042.root --mt $MIDASDATA/run904042sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l3.json --Bfield 0 --calib ;} &> RunLogs/R904042.log &
#root -q -b $DATADIR/agmini/cosmics904042.root ana/macros/ReadEventTree.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904042.root ana/macros/plotTPCdeformation.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904042.root ana/macros/plotPadSigma.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904042.root ana/macros/padplot.C+ &> /dev/null &

### vertical - AW: 3.2kV - whatever gas - sca gain 2-1
#{ time agana.exe -O$DATADIR/agmini/cosmics904047.root --mt $MIDASDATA/run904047sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l3.json --Bfield 0 --calib ;} &> RunLogs/R904047.log &
#root -q -b $DATADIR/agmini/cosmics904047.root ana/macros/ReadEventTree.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904047.root ana/macros/plotTPCdeformation.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904047.root ana/macros/plotPadSigma.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904047.root ana/macros/padplot.C+ &> /dev/null &

### vertical - AW: 3.2kV - whatever gas - sca gain 3-2
#{ time agana.exe -O$DATADIR/agmini/cosmics904132.root --mt $MIDASDATA/run904132sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl_l3.json --Bfield 0 --calib ;} &> RunLogs/R904132.log &
#root -q -b $DATADIR/agmini/cosmics904132.root ana/macros/ReadEventTree.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904132.root ana/macros/plotTPCdeformation.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904132.root ana/macros/plotPadSigma.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904132.root ana/macros/padplot.C+ &> /dev/null &



### vertical - AW: 3.1kV - whatever gas 
#{ time agana.exe -O$DATADIR/agmini/cosmics904133.root --mt $MIDASDATA/run904133sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl.json --Bfield 0 --calib ;} &> RunLogs/R904133.log &
#root -q -b $DATADIR/agmini/cosmics904133.root ana/macros/ReadEventTree.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904133.root ana/macros/plotTPCdeformation.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904133.root ana/macros/plotPadSigma.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904133.root ana/macros/padplot.C+ &> /dev/null &

### vertical - AW: 3.1kV - whatever gas - sca gain 3-2
#{ time agana.exe -O$DATADIR/agmini/cosmics904134.root --mt $MIDASDATA/run904134sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl.json --Bfield 0 --calib ;} &> RunLogs/R904134.log &
#root -q -b $DATADIR/agmini/cosmics904134.root ana/macros/ReadEventTree.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904134.root ana/macros/plotTPCdeformation.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904134.root ana/macros/plotPadSigma.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904134.root ana/macros/padplot.C+ &> /dev/null &

### vertical - AW: 3.1kV - whatever gas - sca gain 2-1
#{ time agana.exe -O$DATADIR/agmini/cosmics904135.root --mt $MIDASDATA/run904135sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl.json --Bfield 0 --calib ;} &> RunLogs/R904135.log &
#root -q -b $DATADIR/agmini/cosmics904135.root ana/macros/ReadEventTree.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904135.root ana/macros/plotTPCdeformation.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904135.root ana/macros/plotPadSigma.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904135.root ana/macros/padplot.C+ &> /dev/null &

### vertical - AW: 3.1kV - whatever gas - sca gain 1-2
#{ time agana.exe -O$DATADIR/agmini/cosmics904136.root --mt $MIDASDATA/run904136sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl.json --Bfield 0 --calib ;} &> RunLogs/R904136.log &
#root -q -b $DATADIR/agmini/cosmics904136.root ana/macros/ReadEventTree.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904136.root ana/macros/plotTPCdeformation.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904136.root ana/macros/plotPadSigma.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904136.root ana/macros/padplot.C+ &> /dev/null &


### vertical - AW: 3.2kV - Ar-CO2 70:30
#{ time agana.exe -O$DATADIR/agmini/cosmics904139.root --mt $MIDASDATA/run904139sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl.json --Bfield 0 --calib ;} &> RunLogs/R904139.log &
#root -q -b $DATADIR/agmini/cosmics904139.root ana/macros/ReadEventTree.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904139.root ana/macros/plotTPCdeformation.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904139.root ana/macros/plotPadSigma.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904139.root ana/macros/padplot.C+ &> /dev/null &
#root $DATADIR/agmini/cosmics904139.root ana/macros/padplot.C+


### vertical - AW: 3.2kV - Ar-CO2 70:30 - sca gain 1-2
#{ time agana.exe -O$DATADIR/agmini/cosmics904147.root --mt $MIDASDATA/run904147sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl.json --Bfield 0 --calib ;} &> RunLogs/R904147.log &
#root -q -b $DATADIR/agmini/cosmics904147.root ana/macros/ReadEventTree.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904147.root ana/macros/plotTPCdeformation.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904147.root ana/macros/plotPadSigma.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904147.root ana/macros/padplot.C+ &> /dev/null &


### vertical - AW: 3.2kV - Ar-CO2 70:30 - sca gain 2-3
#{ time agana.exe -O$DATADIR/agmini/cosmics904148.root --mt $MIDASDATA/run904148sub*.mid.lz4 -- --diag --anasettings $AGRELEASE/ana/cyl.json --Bfield 0 --calib ;} &> RunLogs/R904148.log &
#root -q -b $DATADIR/agmini/cosmics904148.root ana/macros/ReadEventTree.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904148.root ana/macros/plotTPCdeformation.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904148.root ana/macros/plotPadSigma.C+ &> /dev/null &
#root -q -b $DATADIR/agmini/cosmics904148.root ana/macros/padplot.C+ &> /dev/null &
