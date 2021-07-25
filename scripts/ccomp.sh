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
    time ${cmd} --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "noAg" ]]; then
    echo "Building alphaAnalysis and sim only"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    ${cmd} -DBUILD_A2_SIM=ON -DBUILD_AG=OFF ..
    time ${cmd} --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE

elif [[ "$1" == "nosim" ]]; then
    echo "Building alphasoft without Simulation components"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    ${cmd} ..
    if [[ "$2" == "verbose" ]]; then
	echo "verbose build without multiproc"
	time ${cmd} --build . --target install --verbose
    else
	time ${cmd} --build . --target install -- -j`nproc --ignore=2`
    fi
    cd $AGRELEASE

elif [[ "$1" == "ci" ]]; then
    echo "Building agsoft for CI"
    #Quit on error (so CI reports failure properly)
    set -e
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    ${cmd} -DCMAKE_BUILD_TYPE=Release ..
    ${cmd} --build . --target install
    ls -lh $AGRELEASE/bin
    cd $AGRELEASE

elif [[ "$1" == "debug" ]]; then
    echo "Building agsoft with Debug symbols"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    ${cmd} -DCMAKE_BUILD_TYPE=Debug ..
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
    elif [[ "$2" == "subm" ]]; then
	echo "with local manalyzer"
	${cmd} -DBUILD_MANALYZER=ON -DCMAKE_BUILD_TYPE=Release ..
    else
	echo "with ALPHA-g Simulation components"
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
    echo "- noA2 (without alphaAnalysis)"
    echo "- noAg (without agana)"
    echo "- nosim (build and install without simulation)"
    echo "- ci (build and install but quit on errors, no simulation)"
    echo "- debug (build with debug symbols and install)"
    echo "- build [nosim] (build optimized code with or without AG sim)"
    echo "- verbose (build optimized code with verbose output)"
    echo "- help: display the current output "
    echo " "
    echo "Default: build and install"

else
    echo "Building alphasoft"
    mkdir -p $AGRELEASE/build
    cd $AGRELEASE/build
#
    ${cmd} -DBUILD_AG_SIM=ON -DBUILD_A2_SIM=ON ..
    time ${cmd} --build . --target install -- -j`nproc --ignore=2`
    cd $AGRELEASE
fi



