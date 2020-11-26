TOPDIR=/Users/germano/local
############# geant4_vmc installation directory and version ########################
G4VMCINST=$TOPDIR/geant4_vmc.5.2/geant4_vmc_install
G4VMCVER=5.2.0
#############     VGM installation directory and version    ########################
VGMINST=$TOPDIR/vgm.4.8/vgm_install
VGMVER=4.8.0
#############     VMC installation directory and version    ########################
VMCINST=$TOPDIR/vmc.1.0.p3/vmc_install
VMCVER=1.0.3

LIB=lib     # in some systems this is required instead of LIB=lib64
#LIB=lib64  # in some systems this is required instead of LIB=lib

rm -rf CMakeFiles
rm -ff CMakeCache.txt 
rm -rf Makefile

MTLIBPATH=$(ls $G4VMCINST/$LIB/ | grep MTRoot-)

cmake -Wno-dev -DGeant4VMC_DIR=$G4VMCINST/$LIB/Geant4VMC-$G4VMCVER -DMTRoot_DIR=$G4VMCINST/$LIB/$MTLIBPATH -DVMC_DIR=$VMCINST/$LIB/VMC-$VMCVER -DCMAKE_PREFIX_PATH=$VMCINST/$LIB/VMC-$VMCVER .
