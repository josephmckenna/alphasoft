#!/bin/bash
#

cmd=cmake
ver=$($cmd --version | grep 3.[0-9].)
if [[ -z "${ver}" ]]; then
    cmd=cmake3
fi


if [[ "$1" == "clean" ]]; then
    echo "Erasing alphasoft build"
    cd $AGRELEASE/build
    ${cmd} --build . --target clean
    cd $AGRELEASE
    if [[ "$2" == "all" ]]; then
	echo "removing build and bin folders"
	rm -rf $AGRELEASE/build $AGRELEASE/bin
	rm -rf $AGRELEASE/rootana/include $AGRELEASE/rootana/lib
    fi

elif [[ "$1" == "update" ]]; then
    echo "Recompiling alphasoft"
    cd $AGRELEASE/build
    time ${cmd} --build . -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "install" ]]; then
    echo "Installing alphasoft"
    cd $AGRELEASE/build
    time ${cmd} --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "noA2" ]]; then
    echo "Building agana and sim only"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    ${cmd} -DBUILD_AG_SIM=ON -DBUILD_A2=OFF ..
    #time ${cmd} --build . -- -j`nproc --ignore=2`
    time ${cmd} --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "noAg" ]]; then
    echo "Building alphaAnalysis and sim only"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    ${cmd} -DBUILD_A2_SIM=OFF -DBUILD_AG=OFF ..
    #time ${cmd} --build . -- -j`nproc --ignore=2`
    time ${cmd} --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "nosim" ]]; then
    echo "Building alphasoft without Simulation components"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    ${cmd} ..
    #time ${cmd} --build . -- -j`nproc --ignore=2`
    time ${cmd} --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "ci" ]]; then
    echo "Building agsoft for CI"
    #Quit on error (so CI reports failure properly)
    set -e
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    ${cmd} -DBUILD_AG_SIM=OFF -DBUILD_A2=ON -DCMAKE_BUILD_TYPE=Release ..
    ${cmd} --build . --target install
    ls -lh $AGRELEASE/bin
    cd $AGRELEASE

elif [[ "$1" == "debug" ]]; then
    echo "Building agsoft with Debug symbols"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    ${cmd} -DCMAKE_BUILD_TYPE=Debug ..
    #time ${cmd} --build . -- -j`nproc --ignore=2`
    time ${cmd} --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "build" ]]; then
    echo "Building alphasoft optimized"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    if [[ "$2" == "nosim" ]]; then
	echo "without Simulation components"
	${cmd} -DCMAKE_BUILD_TYPE=Release ..
    else
	echo "with Simulation components"
	#${cmd} -DBUILD_AG_SIM=ON -DBUILD_A2_SIM=ON -DCMAKE_BUILD_TYPE=Release ..
	${cmd} -DBUILD_AG_SIM=ON -DCMAKE_BUILD_TYPE=Release ..
    fi

    time ${cmd} --build . --target install -- -j`nproc --ignore=2`

    cd $AGRELEASE

elif [[ "$1" == "verbose" ]]; then
    echo "Building agsoft verbosily"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    ${cmd} -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=TRUE -DCMAKE_INSTALL_PREFIX=$AGRELEASE/bin ..
    ${cmd} --build . --target install --verbose -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "help" ]]; then
    echo "Options are:"
    echo "- clean [all]"
    echo "- update (build only)"
    echo "- install"
    echo "- nosim (build and install)"
    echo "- debug (build with debug symbols and install)"
    echo "- build [nosim] (build optimized code)"
    echo "- verbose (build optimized code with verbose output)"
    echo " "
    echo "Default: build and install"

else
    echo "Building alphasoft"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    ${cmd} -DBUILD_AG_SIM=ON -DBUILD_A2_SIM=OFF ..
    #time ${cmd} --build . -- -j`nproc --ignore=2`
    time ${cmd} --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE
fi



