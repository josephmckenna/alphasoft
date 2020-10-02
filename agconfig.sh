

#Path is this file:
SOURCE="${BASH_SOURCE[0]}"
# resolve $SOURCE until the file is no longer a symlink
while [ -h "$SOURCE" ]; do
    DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
    SOURCE="$(readlink "$SOURCE")"
    # if $SOURCE was a relative symlink, we need to resolve it relative
    #to the path where the symlink file was located
    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
export AGRELEASE="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
#Over write data paths in 'localised profiles' based on host /domain names below
export AGMIDASDATA=${AGRELEASE}
export A2DATAPATH=${AGRELEASE}/alpha2

export AG_CFM=${AGRELEASE}/ana

# It can be used to tell the ROOTUTILS to fetch an output
# rootfile somewhere different from the default location
export AGOUTPUT=${AGRELEASE} # this is the default location

# This MUST be set in order to create the simulation output
if [[ -z "${MCDATA}" ]]; then
    export MCDATA=${AGRELEASE}/simulation
fi

sim_submodules_firsttimesetup()
{
  sim_submodules

  NCPU=`root-config --ncpu`

  #GEANT4
  #cd $AGRELEASE/simulation/submodules/geant4
  #mkdir build
  #mkdir install
  #cd build
  #cmake3 ../ -DGEANT4_INSTALL_DATA=ON -DGEANT4_USE_GDML=ON -DCMAKE_INSTALL_PREFIX=$AGRELEASE/simulation/submodules/geant4/install -DGEANT4_USE_XM=ON
  #make -j${NCPU}
  #make install 
  #cd ../install
  #. bin/geant4.sh
  
  #export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:$AGRELEASE/simulation/submodules/geant4/install/lib64/Geant4-`geant4-config --version`
  #export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AGRELEASE/simulation/submodules/geant4/install/lib64

  #CRY
  cd $AGRELEASE/simulation/submodules/
  wget https://nuclear.llnl.gov/simulation/cry_v1.7.tar.gz
  tar xvzf cry_v1.7.tar.gz 
  rm cry_v1.7.tar.gz 
  cd cry_v1.7
  #Skip test... the diff causes CI to quit?
  make setup lib

  #CADMESH
  cd ${CADMESH_HOME}
  mkdir build
  cd build
  cmake3 -DCMAKE_INSTALL_PREFIX=${CADMESH_HOME}/install ../
  make -j${NCPU}
  make install

  #GARFIELD
  #Git doesn't compile on Centos7... use svn for now...
  #svn co http://svn.cern.ch/guest/garfield/trunk $GARFIELD_HOME
  cd $AGRELEASE/simulation/submodules/
  git clone https://gitlab.cern.ch/garfield/garfieldpp.git 
  #$GARFIELD_HOME
  cd ${GARFIELD_HOME}
  make -j${NCPU}
  #git commands:
  #mkdir build
  #mkdir install
  #cd build
  #cmake3 -DCMAKE_INSTALL_PREFIX=${GARFIELD_HOME}/install -DROOT_CMAKE_DIR=`root-config --etcdir`/cmake  ../
  #make -j${NCPU}
  #make install

  #Finally... build the simulation
  cd $AGRELEASE/simulation
  cmake3 -DCMAKE_BUILD_TYPE=Release geant4
  make
  
}

sim_submodules()
{

  #ROOT
  export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:`root-config --etcdir`/cmake

  #GEANT4
  . geant4.sh

  #CRY
  export CRYHOME=$AGRELEASE/simulation/submodules/cry_v1.7
  export CRYDATAPATH=$CRYHOME/data
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CRYHOME/lib
  export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:$CRY_HOME

  #CADMESH
  export CADMESH_HOME=$AGRELEASE/simulation/submodules/CADMesh/
  export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:$CADMESH_HOME/install/
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CADMESH_HOME/install/lib
  
  
  #Garfield:
  export GARFIELD_HOME=${AGRELEASE}/simulation/submodules/garfieldpp
  export HEED_DATABASE=${GARFIELD_HOME}/Heed/heed++/database
  
  #export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:$GARFIELD_HOME/install/
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$GARFIELD_HOME/Library

}

#Computer profiles

alphaBeast()
{
  export EOS_MGM_URL=root://eospublic.cern.ch
  . ~/packages/root_build/bin/thisroot.sh
  #. ~/packages/rootana/thisrootana.sh
  #. ~/joseph/agdaq/rootana/thisrootana.sh
  #. /cvmfs/sft.cern.ch/lcg/releases/gcc/4.9.3/x86_64-centos7/setup.sh
  #. /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.14.04/x86_64-centos7-gcc48-opt/root/bin/thisroot.sh
  #. ~/joseph/packages/root_build/bin/thisroot.sh

  #If geant4 is installed, set up simulation vars
  if [ `command -v geant4-config | wc -c` -gt 5 ]; then
      echo "Geant4 installation found..."
      sim_submodules
  fi
}
alphaCrunch()
{
  export EOS_MGM_URL=root://eospublic.cern.ch
  . ~/packages/rootana/thisrootana.sh
  #. ~/joseph/agdaq/rootana/thisrootana.sh
  . /cvmfs/sft.cern.ch/lcg/releases/gcc/4.8.4/x86_64-centos7/setup.sh
  . /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.20.06/x86_64-centos7-gcc48-opt/bin/root/thisroot.sh

  #If geant4 is installed, set up simulation vars
  if [ `command -v geant4-config | wc -c` -gt 5 ]; then
      echo "Geant4 installation found..."
      sim_submodules
  fi
}

agana()
{
  export EOS_MGM_URL=root://eospublic.cern.ch
  . ~/packages/root_v6.16.00_el74_64/bin/thisroot.sh
#  . /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.14.04/x86_64-centos7-gcc48-opt/root/bin/thisroot.sh
#  . ~/packages/rootana/thisrootana.sh
  echo -e " \e[34m `git status | head -1`\e[m"
}

acapra()
{
    echo -e " \e[91m Hi Andrea! \e[m"
    export EOS_MGM_URL=root://eospublic.cern.ch
    export AGMIDASDATA="/daq/alpha_data0/acapra/alphag/midasdata"
    export AGOUTPUT="/daq/alpha_data0/acapra/alphag/output"
    export GARFIELDPP="$AGRELEASE/build/simulation/garfieldpp"
    #export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
    echo -e " \e[32m `gcc --version | head -1`\e[m"
    echo -e " \e[34m `git status | head -1`\e[m"
}

lxplus()
{
  export EOS_MGM_URL=root://eospublic.cern.ch
  export AGMIDASDATA=${AGRELEASE}
  echo "Setting (CentOS7) lxplus/batch environment variables"
  if [ -d "/cvmfs/sft.cern.ch/lcg/releases/gcc/4.8.4/x86_64-centos7/" ]; then
      #. /cvmfs/sft.cern.ch/lcg/releases/gcc/4.8.4/x86_64-centos7/setup.sh
      #FUTURE:Use our own build of root (include xrootd,R, Python2.7 and minuit2)
      #. /cvmfs/alpha.cern.ch/CC7/packages/root/root_build/bin/thisroot.sh
      . /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.22.02/x86_64-centos7-gcc48-opt/bin/thisroot.sh
  else
    echo "cvmfs not found! Please install and mount cvmfs"
  fi
  
  #If geant4 is installed, set up simulation vars
  if [ `command -v geant4-config | wc -c` -gt 5 ]; then
      echo "Geant4 installation found..."
      sim_submodules
  fi
}







echo "############## agconfig.sh ##############"
echo "Hostname: " `hostname`
echo "Username: " `whoami`
echo "#########################################"

#Setup LD_LIBRARY_PATH
for AG_LIB_PATH in ana/obj {,build/}analib {,build/}aged {,build/}recolib {,build/}a2lib; do
  if echo "${LD_LIBRARY_PATH}" | grep "${AGRELEASE}/${AG_LIB_PATH}/" > /dev/null; then
    NOTHING_TO_DO=1
  else
    echo "Adding ${AG_LIB_PATH} to LD_LIBRARY_PATH"
    export  LD_LIBRARY_PATH=${AGRELEASE}/${AG_LIB_PATH}/:${LD_LIBRARY_PATH}
  fi
done

#Set up Root include path
for AG_ROOT_LIB_PATH in ana/include analib/include analib/RootUtils aged recolib/include a2lib/include a2lib/legacy; do
  if echo "${ROOT_INCLUDE_PATH}" | grep "${AGRELEASE}/${AG_ROOT_LIB_PATH}/" > /dev/null; then
    NOTHING_TO_DO=1
  else
    echo "Adding ${AG_ROOT_LIB_PATH} to ROOT_INCLUDE_PATH"
    export  ROOT_INCLUDE_PATH=${AGRELEASE}/${AG_ROOT_LIB_PATH}/:${ROOT_INCLUDE_PATH}
  fi
done

#Add scripts to BIN path
for AG_BIN_PATH in scripts bin; do
  if echo ${PATH} | grep "${AGRELEASE}/${AG_BIN_PATH}/" > /dev/null; then
    NOTHING_TO_DO=1
  else
    echo "Adding ${AG_BIN_PATH} to PATH"
    export  PATH=${AGRELEASE}/${AG_BIN_PATH}/:${PATH}
  fi
done



if [ "${1}" = "install_sim" ]; then
  echo "Installing all simulation submodules... go get a coffee..."
  sleep 1
  echo "No really... go get a coffee... this will take some time.."
  sleep 1
  git submodule update --init 
  sim_submodules_firsttimesetup
fi



#Quit if ROOT and ROOTANA are setup...
if [ "${1}" = "clean" ]; then
  echo "Clean setup of environment variables"
  echo "Now using rootana git submodule"
  export ROOTANASYS="${AGRELEASE}/rootana"
else
  if [ ${#ROOTANASYS} -gt 3 ]; then
    echo "ROOTANASYS set... not over writing: $ROOTANASYS"
  else
    echo "Using rootana git submodule"
    export ROOTANASYS="${AGRELEASE}/rootana"
  fi
  if [ ${#ROOTSYS} -lt 3 ]; then
    echo "Please setup root manually (or run . agconfig.sh clean)"
  else
    echo "ROOTSYS set... not over writing: $ROOTSYS"
  fi
fi

if [ "$ROOTANASYS" = "${AGRELEASE}/rootana" ]; then
    if [ -n "$(ls -A $ROOTANASYS)" ]; then
    	echo "ROOTANA submodule enabled"
    else
	echo "Enabling ROOTANA submodule..."
	git submodule update --init rootana
    fi
fi

#Setup ROOT and ROOTANA if we havn't quit yet...
case `hostname` in
alphavme*  )
  echo "alphavme detected..."
  echo "DO NOT RUN ANALYSIS ON A VME CRATE"
  exit
  ;;
alphagdaq* | alphadaq* )
  echo "DAQ computer detected..."
  echo "DO NOT RUN ANALYSIS ON DAQ!!!"
  return
  ;;
alphacpc04* | alphacpc09*  )
  echo -e " \e[33malphacpc04 or 09 detected...\033[0m"
  export AGMIDASDATA="/alpha/agdaq/data"
  if [ `whoami` = "agana" ] ; then
      echo -e " \e[33mUser agana\033[0m"
      agana
  else
      #If geant4 is installed, set up simulation vars
      if [ `command -v geant4-config | wc -c` -gt 5 ]; then
	  echo "Geant4 installation found..."
	  sim_submodules
      fi
  fi
  ;;
*.triumf.ca )
  echo -e " \e[33m alphaXXtriumf.ca or daqXX.triumf.ca  detected...\033[0m"
  export AGMIDASDATA="/daq/alpha_data0/acapra/alphag/midasdata/"
  if [ `whoami` = "acapra" ] ; then
      acapra
  fi
  ;;
alphabeast* )
  echo -e " \e[33malphabeast detected...\033[0m"
  alphaBeast
  ;;
alphacrunch* )
  echo -e " \e[33malphacrunch detected...\033[0m"
  alphaCrunch
  ;;
lxplus* )
  echo -e " \e[33mlxplus detected...\033[0m"
  lxplus
  ;;
* )
  if [ -n "${ROOTSYS}" ]; then
  echo "$ROOTSYS seems to be set ok"
  else
  echo "ROOTSYS not set... Guessing settings for new computer..."
  if [ -d "/cvmfs/sft.cern.ch/lcg/releases/gcc/4.8.4/x86_64-centos7/" ]; then
    echo "cvmfs found..."
    lxplus
  else
    echo "I don't know what to do yet"
  fi
  fi
  echo 'gcc       :' `which gcc`
  echo 'g++       :' `which g++`
  echo 'c++       :' `which c++`
  echo 'cc        :' `which cc`
  echo "ROOTSYS   : ${ROOTSYS}"
  echo "ROOTANASYS: ${ROOTANASYS}"

  ;;
esac
