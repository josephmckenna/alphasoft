TOPDIR=~/packages/simulation

#root is assumed to be installed 

#geant4 is assumed to be installed


cd $TOPDIR
git clone https://github.com/vmc-project/vgm.git
cd vgm
git checkout checkout v4-8
cd ..
mkdir vgm_build
mkdir vgm_install
cd vgm_build
cmake -DCMAKE_INSTALL_PREFIX=$TOPDIR/vgm_install ../vgm
make -j32
make install

cd $TOPDIR
git clone https://github.com/vmc-project/vmc.git
cd vmc
git checkout v1-0-p3
cd ..
mkdir vmc_build
mkdir vmc_install
cd vmc_build
cmake -DCMAKE_INSTALL_PREFIX=$TOPDIR/vmc_install ../vmc
make -j32
make install

mkdir $TOPDIR
cd $TOPDIR
git clone https://github.com/vmc-project/geant4_vmc.git
cd geant4_vmc/
git checkout v5-3
cd ..
mkdir geant4_vmc_build
mkdir geant4_vmc_install
cd geant4_vmc_build/
cmake -DCMAKE_INSTALL_PREFIX=$TOPDIR/geant4_vmc_install -DVGM_DIR=$VGMINST/lib/VGM-$VGMVER ../geant4_vmc
make -j32
make install

