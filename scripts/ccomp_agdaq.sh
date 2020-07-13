#!/bin/bash
#
if [[ "$1" == "clean" ]]; then
    echo "Erasing agdaq build"
    cd $AGRELEASE/build
    cmake3 --build . --target clean
    cd $AGRELEASE
    rm -rf $AGRELEASE/build $AGRELEASE/bin
elif [[ "$1" == "update" ]]; then
    echo "Recompiling agdaq"
    cd $AGRELEASE/build
    time cmake3 --build . -- -j
    cd $AGRELEASE
elif [[ "$1" == "install" ]]; then
    echo "Install agdaq"
    cd $AGRELEASE/build
    time cmake3 --build . --target install -- -j
    cd $AGRELEASE
elif [[ "$1" == "wA2" ]]; then
    echo "Building agdaq and alphaAnalysis"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    cmake3 .. -DBUILD_AG_SIM=ON
    time cmake3 --build . -- -j
    time cmake3 --build . --target install -- -j
    cd $AGRELEASE
elif [[ "$1" == "nosim" ]]; then
    echo "Building agdaq without Simulation components"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    cmake3 .. -DBUILD_AG_SIM=OFF -DBUILD_A2=OFF 
    time cmake3 --build . -- -j
    time cmake3 --build . --target install -- -j
    cd $AGRELEASE

elif [[ "$1" == "help" ]]; then

    echo "Options are:"
    echo "- clean"
    echo "- update (build only)"
    echo "- install"
    echo "- wA2"
    echo "- nosim (build and install)"
    echo "Default: build and install"

else
    echo "Building agdaq"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    cmake3 .. -DBUILD_AG_SIM=ON -DBUILD_A2=OFF 
    time cmake3 --build . -- -j
    time cmake3 --build . --target install -- -j
    cd $AGRELEASE
fi



