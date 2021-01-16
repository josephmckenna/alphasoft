####################################################################################
# A2MC CMAKE CONFIGURATION AND INSTALLATION SCRIPT
####################################################################################

## ENVIRONMENTAL PATH DEFINITION (WHERE DID YOU INSTALL VMC, VGM AND GEANT4_VMC ? ##
## ================================
WORK_DIR=$(pwd)
INST_DIR=/Users/germano/local
############# geant4_vmc installation directory and version ########################
G4VMC=$INST_DIR/geant4_vmc.5.3/geant4_vmc_install
G4VMCVER=5.3.0
#############     VGM installation directory and version    ########################
VGM=$INST_DIR/vgm.4.8/vgm_install
VGMVER=4.8.0
#############     VMC installation directory and version    ########################
VMC=$INST_DIR/vmc.1.0.p3/vmc_install
VMCVER=1.0.3
#############     LIBRARY DIRECTORY SUFFIX (lib or lib64)   ########################
LIB=lib     # in some systems this is required instead of LIB=lib64 (macosx?)
#LIB=lib64  # in some systems this is required instead of LIB=lib (ubuntu?)

if [[ "$1" == "clean" ]]; then
    echo "cleaning up"
    rm -rf build
    #rm -rf build/CMakeFiles
    #rm -ff build/CMakeCache.txt 
    #rm -rf build/Makefile
fi

if [[ "$1" == "build" ]]; then
    if [ -f "build/CMakeCache.txt" ]; then
	echo "Building (and installing) a2mc only"
	cd build
    else
	echo "Configure cmake first"
	exit 1
    fi
else
    echo "Configuring cmake for a2mc"
    ##    CREATE THE BUILD DIRECTORY  ##
    ## =================================
    mkdir build && cd build

    ##  SET ALL THE NECESSARY OPTIONS ##
    ## =================================
    cmake -DGeant4VMC_DIR:PATH=$G4VMC/$LIB/Geant4VMC-$G4VMCVER -DMTRoot_DIR:PATH=$G4VMC/$LIB/MTRoot-$G4VMCVER -DVGM_DIR:PATH=$VGM/$LIB/VGM-$VGMVER -DVMC_DIR:PATH=$VMC/$LIB/VMC-$VMCVER -DCMAKE_INSTALL_LIBDIR:PATH=$WORK_DIR/$LIB -DCMAKE_INSTALL_PREFIX:PATH=$WORK_DIR -DCMAKE_BUILD_TYPE:STRING=Release ..
fi

##         BUILD AND INSTALL      ##
## =================================
cmake --build . --target install

cd ${WORK_DIR}
