#!/bin/bash

if [[ -z ${AGRELEASE} ]]; then
    source /home/acapra/agnewtrack/agconfig.sh
else
    echo "Ready to test Fitter"
fi

cd $AGRELEASE

if [ ! -d "${AGRELEASE}/RunLogs" ]; then
    mkdir -p ${AGRELEASE}/RunLogs
fi

#Check if we need cmake3 command or cmake
if [ `command -v cmake3` ]; then
    #command cmake3 found... lets use it
    export CMAKE=cmake3
else
    #cmake3 not found. Default version of cmake is probably 3
    export CMAKE=cmake
fi

build_M1()
{
    if [[ "$1" == "clean" ]]; then
	echo "cleaning first"
	rm -rf $AGRELEASE/binM1 $AGRELEASE/buildM1
	mkdir -p $AGRELEASE/buildM1
	cd $AGRELEASE/buildM1
	${CMAKE} .. -DBUILD_AG_SIM=ON -DBUILD_A2=OFF -DBUILD_MANALYZER=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$AGRELEASE/binM1
    fi
    cd $AGRELEASE/buildM1
    #${CMAKE} .. -DBUILD_AG_SIM=ON -DBUILD_A2=OFF -DMINUIT2FIT=OFF -DBUILD_MANALYZER=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$AGRELEASE/binM1 |& tee ../BuildLogM1.txt
    ${CMAKE} --build . --target install --verbose -- -j`nproc --ignore=2` |& tee -a ../BuildLogM1.txt
}

build_M2()
{
    if [[ "$1" == "clean" ]]; then
	echo "cleaning first"
	rm -rf $AGRELEASE/binM2 $AGRELEASE/buildM2
	mkdir -p $AGRELEASE/buildM2
	cd $AGRELEASE/buildM2
	${CMAKE} .. -DBUILD_AG_SIM=ON -DBUILD_A2=OFF -DBUILD_MANALYZER=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$AGRELEASE/binM2 |& tee ../BuildLogM2.txt
    fi
    cd $AGRELEASE/buildM2
    ${CMAKE} --build . --target install --verbose -- -j`nproc --ignore=2` |& tee -a ../BuildLogM2.txt
}

build_run_debug()
{
    fin=$(printf "${AGMIDASDATA}/run%05dsub000.mid.lz4" $1)
    if [ ! -f ${fin} ]; then
	exit -1
    fi
    fout=$(printf "cosmics%dsub000M2dbg.root" $1)
    rm -rf $AGRELEASE/bin $AGRELEASE/build
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
    ${CMAKE} .. -DBUILD_AG_SIM=OFF -DBUILD_A2=ON -DBUILD_MANALYZER=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$AGRELEASE/bin
    time ${CMAKE} --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE
    rm -f gdb.txt
    gdb -ex="set logging on" -ex=r --args agana.exe -O"$DATADIR/test/$fout" --mt "${fin}" -- --diag --anasettings $AGRELEASE/ana/cosm.json
    mv $AGRELEASE/gdb.txt "$AGRELEASE/RunLogs/R$1sub000gdb.log"
}

run_M2()
{
    fin=$(printf "${AGMIDASDATA}/run%05dsub000.mid.lz4" $1)
    fout=$(printf "${DATADIR}/test/cosmics%dsub000M2.root" $1)
    log="${AGRELEASE}/RunLogs/R$1sub000M2.log"
    if [ ! -f ${fin} ]; then
	exit -1
    else
	echo "saving output to: $fout"
	echo "saving log to: $log"
    fi
    cd $AGRELEASE/binM2
    ./agana.exe -O"${fout}" "${fin}" -- --usetimerange 0.0 15.0 --diag --anasettings $AGRELEASE/ana/cosm.json &> ${log} &
}

runmt_M2()
{
    fin=$(printf "${AGMIDASDATA}/run%05dsub000.mid.lz4" $1)
    fout=$(printf "${DATADIR}/test/cosmics%dsub000M2mt.root" $1)
    log="${AGRELEASE}/RunLogs/R$1sub000M2mt.log"
    if [ ! -f ${fin} ]; then
	exit -1
    else
	echo "saving output to: $fout"
	echo "saving log to: $log"
    fi
    cd $AGRELEASE/binM2
    ./agana.exe --mt -O"${fout}" "${fin}" -- --diag --anasettings $AGRELEASE/ana/cosm.json &> ${log} &
}


run_M1()
{
    fin=$(printf "${AGMIDASDATA}/run%05dsub000.mid.lz4" $1)
    fout=$(printf "${DATADIR}/test/cosmics%dsub000M1.root" $1)
    log="${AGRELEASE}/RunLogs/R$1sub000M1.log"
    if [ ! -f ${fin} ]; then
	exit -1
    else
	echo "saving output to: $fout"
	echo "saving log to: $log"
    fi
    cd $AGRELEASE/binM1
    ./agana.exe -O"${fout}" "${fin}" -- --usetimerange 0.0 15.0 --diag --anasettings $AGRELEASE/ana/cosm.json &> $log &
 #   tail -f $log
}

runmt_M1()
{   
    fin=$(printf "${AGMIDASDATA}/run%05dsub000.mid.lz4" $1)
    fout=$(printf "${DATADIR}/test/cosmics%dsub000M1mt.root" $1)
    log="${AGRELEASE}/RunLogs/R$1sub000M1mt.log"
    if [ ! -f ${fin} ]; then
	exit -1
    else
	echo "saving output to: $fout"
	echo "saving log to: $log"
    fi
    cd $AGRELEASE/binM1
    ./agana.exe --mt -O"${fout}" "${fin}" -- --diag --anasettings $AGRELEASE/ana/cosm.json &> ${log} &
}


run=3863

echo "B=1T, trig: AW MLU1"
echo "Starting R${run}"

fin=$(printf "${AGMIDASDATA}/run%05dsub000.mid.lz4" $run)
ls -l "${fin}"


#set -x


#build_M1 clean
#build_M2 clean
##runmt_M1 $run
##runmt_M2 $run
#run_M1 $run
#run_M2 $run

fout=$(printf "${DATADIR}/test/cosmics%dsub000M2mt.root" $run)
echo ${fout}
MainEventTree.exe -f $fout -p 1 -s 1 &> RunLogs/Reco${run}M2.log
sleep 60

fout=$(printf "${DATADIR}/test/cosmics%dsub000M1mt.root" $run)
echo ${fout}
MainEventTree.exe -f $fout -p 1 -s 1 &> RunLogs/Reco${run}M1.log

