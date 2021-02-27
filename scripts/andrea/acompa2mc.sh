#!/bin/bash

cd $AGRELEASE

INST_DIR=$HOME/packages
LIB=lib64  # in some systems this is required instead of LIB=lib (ubuntu?)

VGM=$INST_DIR/vgm/install
VGMVER=4.9.0

VMC=$INST_DIR/vmc/install
VMCVER=1.0.3

G4VMC=$INST_DIR/geant4_vmc/install
G4VMCVER=5.3.0

DEP_LIST=($G4VMC/$LIB/Geant4VMC-$G4VMCVER $G4VMC/$LIB/MTRoot-$G4VMCVER $VGM/$LIB/VGM-$VGMVER $VMC/$LIB/VMC-$VMCVER)
for DEP in ${DEP_LIST[*]}; do
    printf "   %s\n" $DEP
    ll $DEP
done

rm -rf build
mkdir -p build
cd build

cmake -DBUILD_AG_SIM=ON -DBUILD_A2_SIM=ON -DGeant4VMC_DIR=$G4VMC/$LIB/Geant4VMC-$G4VMCVER -DMTRoot_DIR=$G4VMC/$LIB/MTRoot-$G4VMCVER -DVGM_DIR=$VGM/$LIB/VGM-$VGMVER -DVMC_DIR=$VMC/$LIB/VMC-$VMCVER -DCMAKE_INSTALL_PREFIX=../bin ..
