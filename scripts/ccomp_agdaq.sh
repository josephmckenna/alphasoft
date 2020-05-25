#!/bin/bash
#
if [[ "$1" == "clean" ]]; then
    echo "Erasing agdaq build"
    cd $AGRELEASE/build
    cmake3 --build . --target clean
    cd $AGRELEASE
    rm -rf $AGRELEASE/build $AGRELEASE/bin
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



