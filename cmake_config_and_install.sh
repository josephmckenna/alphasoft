####################################################################################
# A2MC CMAKE CONFIGURATION AND INSTALLATION SCRIPT
####################################################################################


#///< SECTION 1 - ENVIRONMENTAL PATH DEFINITION 

#///< Working directory and packages (VGM, VMC and GEANT4_VMC) top directory
WORK_DIR=$(pwd)
INST_DIR=/home/user/local
#///< Library system suffix (either lib or lib64
LIB=lib     # in some systems this is required instead of LIB=lib64 (macosx?, ubuntu?)
#LIB=lib64  # in some systems this is required instead of LIB=lib (centos?)

#///< Where did you install VMC, VGM AND GEANT4_VMC?
# GEANT4_VMC
G4VMC=$INST_DIR/geant4_vmc.5.3/geant4_vmc_install
G4VMCVER=5.3.0

# VGM
VGM=$INST_DIR/vgm.4.8/vgm_install
VGMVER=4.8.0

# VMC
VMC=$INST_DIR/vmc.1.0.p3/vmc_install
VMCVER=1.0.3


#///< SECTION 2 - INSTALLATION PATH CHECKS

#VGM
if [[ ! -d $VGM/$LIB/VGM-$VGMVER ]]; then 
	echo -e "\t Loading VGM cmake files from        ==> " $VGM/$LIB/VGM-$VGMVER
	echo -e "\t *** could not find directory [PLEASE CHECK SECTION 1 of cmake_config_and_install.sh] ***"
	return
fi
#VMC
if [[ ! -d $VMC/$LIB/VMC-$VMCVER ]]; then 
	echo -e "\t Loading VMC cmake files from        ==> " $VMC/$LIB/VMC-$VMCVER
	echo -e "\t *** could not find directory [PLEASE CHECK SECTION 1 of cmake_config_and_install.sh] ***"
	return
fi
#GEANT4_VMC
if [[ ! -d $G4VMC/$LIB/Geant4VMC-$G4VMCVER ]]; then 
	echo -e "\t Loading GEANT4_VMC cmake files from ==> " $G4VMC/$LIB/Geant4VMC-$G4VMCVER
	echo -e "\t *** could not find directory [PLEASE CHECK SECTION 1 of cmake_config_and_install.sh] ***"
	return
fi

#///< SECTION 3 - BUILD AND INSTALLATION
if [[ "$1" == "clean" ]]; then
    echo "cleaning up"
    rm -rf build
fi

if [ -f "build/CMakeCache.txt" ]; then
	cd build
else
    echo "Configuring cmake for a2mc"
    ##    CREATE THE BUILD DIRECTORY  ##
    ## =================================
    mkdir build && cd build

    ##  SET ALL THE NECESSARY OPTIONS ##
    ## =================================
    cmake -DGeant4VMC_DIR:PATH=$G4VMC/$LIB/Geant4VMC-$G4VMCVER -DMTRoot_DIR:PATH=$G4VMC/$LIB/MTRoot-$G4VMCVER -DVGM_DIR:PATH=$VGM/$LIB/VGM-$VGMVER -DVMC_DIR:PATH=$VMC/$LIB/VMC-$VMCVER -DCMAKE_INSTALL_LIBDIR:PATH=$WORK_DIR/$LIB -DCMAKE_INSTALL_PREFIX:PATH=$WORK_DIR -DCMAKE_BUILD_TYPE:STRING=Release ..
fi

cmake --build . --target install

cd ${WORK_DIR}
