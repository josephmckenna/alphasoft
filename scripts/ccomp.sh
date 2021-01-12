#!/bin/bash
#
if [[ "$1" == "clean" ]]; then
    echo "Erasing agsoft build"
    cd $AGRELEASE/build
    cmake3 --build . --target clean
    cd $AGRELEASE
    if [[ "$2" == "all" ]]; then
	echo "removing build and bin folders"
	rm -rf $AGRELEASE/build $AGRELEASE/bin
	rm -rf $AGRELEASE/rootana/include $AGRELEASE/rootana/lib
    fi

elif [[ "$1" == "update" ]]; then
    echo "Recompiling agsoft"
    cd $AGRELEASE/build
    time cmake3 --build . -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "install" ]]; then
    echo "Install agsoft"
    cd $AGRELEASE/build
    time cmake3 --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "noA2" ]]; then
    echo "Building agsoft and alphaAnalysis"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    cmake3 .. -DBUILD_AG_SIM=ON -DBUILD_A2=OFF
    time cmake3 --build . -- -j`nproc --ignore=2`
    time cmake3 --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "nosim" ]]; then
    echo "Building agsoft without Simulation components"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    cmake3 .. -DBUILD_AG_SIM=OFF 
    time cmake3 --build . -- -j`nproc --ignore=2`
    time cmake3 --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "ci" ]]; then
    echo "Building agsoft for CI"
    #Quit on error (so CI reports failure properly)
    set -e
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    cmake3 .. -DBUILD_AG_SIM=OFF -DBUILD_A2=ON -DCMAKE_BUILD_TYPE=Release
    cmake3 --build . 
    cmake3 --build . --target install
    ls -lh $AGRELEASE/bin
    cd $AGRELEASE

elif [[ "$1" == "debug" ]]; then
    echo "Building agsoft with Debug symbols"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    cmake3 .. -DBUILD_AG_SIM=OFF -DBUILD_A2=OFF -DCMAKE_BUILD_TYPE=Debug
    time cmake3 --build . -- -j`nproc --ignore=2`
    time cmake3 --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "build" ]]; then
    echo "Building agsoft optimized"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    if [[ "$2" == "nosim" ]]; then
	echo "without Simulation components"
	cmake3 .. -DBUILD_AG_SIM=OFF -DBUILD_A2=ON -DCMAKE_BUILD_TYPE=Release
    else
	echo "with Simulation components"
	cmake3 .. -DBUILD_AG_SIM=ON -DBUILD_A2=ON -DCMAKE_BUILD_TYPE=Release
    fi

    time cmake3 --build . -- -j`nproc --ignore=2`

    cd $AGRELEASE

elif [[ "$1" == "verbose" ]]; then
    echo "Building agsoft verbosily"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    cmake3 .. -DBUILD_AG_SIM=OFF -DBUILD_A2=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=TRUE
    cmake3 --build  . --verbose
    cmake3 --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "help" ]]; then
    echo "Options are:"
    echo "- clean [all]"
    echo "- update (build only)"
    echo "- install"
    echo "- wA2"
    echo "- nosim (build and install)"
    echo "- debug (build with debug symbols and install)"
    echo "- build [nosim] (build optimized code)"
    echo "- verbose (build optimized code with verbose output)"
    echo " "
    echo "Default: build and install"

else
    echo "Building agsoft"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    cmake3 .. -DBUILD_AG_SIM=ON -DBUILD_A2=ON
    time cmake3 --build . -- -j`nproc --ignore=2`
    time cmake3 --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE
fi



