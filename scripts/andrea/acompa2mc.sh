#!/bin/bash

cd $AGRELEASE

#INST_DIR=$HOME/packages
INST_DIR=$AGRELEASE/simulation
LIB=lib64  # in some systems this is required instead of LIB=lib (ubuntu?)

VGM=$INST_DIR/vgm/install
VGMVER=4.9.0

VMC=$INST_DIR/vmc/install
VMCVER=1.0.3

G4VMC=$INST_DIR/geant4_vmc/install
G4VMCVER=5.3.0

prereq() {
    set -e
#############     VGM installation directory and version    #############
    cd $INST_DIR
    if [ ! -d "./vgm" ]; then
	git clone https://github.com/vmc-project/vgm.git
	cd vgm 
	git checkout v4-9
    fi
    cd $INST_DIR/vgm
    echo "Building: `basename $PWD`"
    
    rm -rf build $VGM && mkdir -p $VGM && mkdir -p build && cd build
    cmake -DCMAKE_INSTALL_PREFIX=$VGM ..
    cmake --build . --target install -- -j`nproc --ignore=2`

#############     VMC installation directory and version    #############
    cd $INST_DIR
    if [ ! -d "./vmc" ]; then
	git clone http://github.com/vmc-project/vmc.git 
	cd vmc
	git checkout v1-0-p3
    fi
    cd $INST_DIR/vmc
    echo "Building: `basename $PWD`"
 
    rm -rf build $VMC && mkdir -p $VMC &&  mkdir -p build && cd build
    cmake -DCMAKE_INSTALL_PREFIX="$VMC" ..
    cmake --build . --target install -- -j`nproc --ignore=2`

############# geant4_vmc installation directory and version  #############
    cd $INST_DIR
    if [ ! -d "./geant4_vmc" ]; then
	git clone http://github.com/vmc-project/geant4_vmc.git 
	cd geant4_vmc
	git checkout v5-3
    fi
    cd $INST_DIR/geant4_vmc
    echo "Building: `basename $PWD`"

    rm -rf build $G4VMC && mkdir -p $G4VMC && mkdir -p build && cd build
    cmake -DCMAKE_INSTALL_PREFIX=$G4VMC -DGeant4VMC_USE_VGM=ON -DVGM_DIR="$VGM/$LIB/VGM-$VGMVER" -DVMC_DIR="$VMC/$LIB/VMC-$VMCVER" -DGeant4VMC_BUILD_MTRoot=ON -DGarfield_INCLUDE_DIR="$GARFIELD_HOME/install/include/Garfield" ..
    cmake --build . --target install -- -j`nproc --ignore=2`

#########################################################################
    cd $AGRELEASE
}

#prereq

#DEP_LIST=($G4VMC/$LIB/Geant4VMC-$G4VMCVER $G4VMC/$LIB/MTRoot-$G4VMCVER $VGM/$LIB/VGM-$VGMVER $VMC/$LIB/VMC-$VMCVER)
#for DEP in ${DEP_LIST[*]}; do
#    printf "   %s\n" $DEP
#    ll $DEP
#done

rm -rf build
mkdir -p build
cd build

cmake --version | head -1
echo "Configuring CMake"

#cmake -DBUILD_AG_SIM=ON -DBUILD_A2_SIM=ON -DGeant4VMC_DIR=$G4VMC/$LIB/Geant4VMC-$G4VMCVER -DMTRoot_DIR=$G4VMC/$LIB/MTRoot-$G4VMCVER -DVGM_DIR=$VGM/$LIB/VGM-$VGMVER -DVMC_DIR=$VMC/$LIB/VMC-$VMCVER -DCMAKE_INSTALL_PREFIX=../bin ..
cmake -DBUILD_AG_SIM=ON -DBUILD_A2_SIM=ON -DGeant4VMC_DIR=$G4VMC/$LIB/Geant4VMC-$G4VMCVER -DVGM_DIR=$VGM/$LIB/VGM-$VGMVER -DVMC_DIR=$VMC/$LIB/VMC-$VMCVER -DCMAKE_INSTALL_PREFIX=../bin ..
#echo "cmake -DBUILD_AG_SIM=ON -DBUILD_A2_SIM=ON -DGeant4VMC_DIR=$G4VMC/$LIB/Geant4VMC-$G4VMCVER -DVGM_DIR=$VGM/$LIB/VGM-$VGMVER -DVMC_DIR=$VMC/$LIB/VMC-$VMCVER -DCMAKE_INSTALL_PREFIX=../bin .."

echo "Ready to build and install?"
echo "cmake --build . --target install -- -j`nproc --ignore=2`"
echo "Or build only?"
echo "cmake --build . --verbose -- -j`nproc --ignore=2`"

#echo "cmake --build . --target clean --verbose"
