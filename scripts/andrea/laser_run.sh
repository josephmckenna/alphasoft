#! /bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi

#

RUNS=(4391 4390 4389 4388 4387 4386 4385 4383 4382 4379 4402)

for idx in ${!RUNS[*]}; do
    run=${RUNS[$idx]}
    printf " Running %4d as number %4d\n" $run $idx
    set -x
    #gdb -ex="set logging on" -ex=r --args laser_calibration.exe --mt -O$DATADIR/laserCERN2019/laser${run}.root $AGMIDASDATA/run0${run}sub*.mid.lz4 -- --diag
    laser_calibration.exe --mt -O$DATADIR/laserCERN2019/laser${run}.root $AGMIDASDATA/run0${run}sub*.mid.lz4 -- --diag &> RunLogs/R${run}.log &
    set +x
done
