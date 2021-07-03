

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

# It can be used to tell the ROOTUTILS to fetch an output
# rootfile somewhere different from the default location
if [[ -z ${AGOUTPUT} ]]; then
    export AGOUTPUT=${AGRELEASE} # this is the default location
fi

#Use EOS PUBLIC if not already set
if [ -e ${EOS_MGM_URL} ]; then
  export EOS_MGM_URL=root://eospublic.cern.ch
fi


# This MUST be set in order to create the simulation output
if [[ -z "${MCDATA}" ]]; then
    export MCDATA=${AGRELEASE}/simulation
fi


#Computer profiles

alphaBeast()
{
  #. ~/packages/root_build/bin/thisroot.sh
    . /cvmfs/sft.cern.ch/lcg/views/LCG_99/x86_64-centos7-gcc8-opt/setup.sh
    echo -e " \e[34m `git status | head -1`\e[m"	
}

alphaCrunch()
{
  . /cvmfs/sft.cern.ch/lcg/releases/gcc/4.8.4/x86_64-centos7/setup.sh
  . /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.20.06/x86_64-centos7-gcc48-opt/bin/root/thisroot.sh

}

agana()
{
    . /cvmfs/sft.cern.ch/lcg/views/LCG_99/x86_64-centos7-gcc8-opt/setup.sh
    echo -e " \e[34m `git status | head -1`\e[m"
}

acapra()
{
    echo -e " \e[91m Hi Andrea! \e[m"
    
    # export GARFIELDPP="$AGRELEASE/build/simulation/garfieldpp"
    export PATH="$AGRELEASE/scripts/andrea":"$AGRELEASE/simulation/garfieldpp/scripts":$PATH

    echo -e " \e[32m `gcc --version | head -1`\e[m"
    echo -e " \e[34m `git status | head -1`\e[m"
}

lxplus()
{
  export AGMIDASDATA="/eos/experiment/ALPHAg/midasdata_old"
  export A2MIDASDATA="/eos/experiment/alpha/midasdata"
  echo "Setting (CentOS7) lxplus/batch environment variables"
  if [ -d "/cvmfs/sft.cern.ch/lcg/releases/gcc/4.8.4/x86_64-centos7/" ]; then
      if [[ -z "${ROOTSYS}" ]]; then
	  echo "Setting up ROOT 6.22.02 with gcc 4.8"
	  . /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.22.02/x86_64-centos7-gcc48-opt/bin/thisroot.sh
      fi
  else
    echo "cvmfs not found! Please install and mount cvmfs"
  fi
}







echo "############## ALPHA Software Setup ##############"
echo "Hostname: " `hostname`
echo "Username: " `whoami`
echo "##################################################"

if [ `echo "${GARFIELD_HOME}" | wc -c` -gt 1 ]; then
   echo "GARFIELD_HOME set to ${GARFIELD_HOME}, configuring... "
   #cat ${GARFIELD_HOME}/install/share/Garfield/setupGarfield.sh
   source ${GARFIELD_HOME}/install/share/Garfield/setupGarfield.sh
else
   echo "GARFIELD_HOME not set"
fi

#Setup LD_LIBRARY_PATH
for AG_LIB_PATH in ana/obj {,build/}analib {,build/}aged {,build/}recolib {,build/}a2lib {,build/}rootUtils; do
  if echo "${LD_LIBRARY_PATH}" | grep "${AGRELEASE}/${AG_LIB_PATH}/" > /dev/null; then
    NOTHING_TO_DO=1
  else
    #echo "Adding ${AG_LIB_PATH} to LD_LIBRARY_PATH"
    export  LD_LIBRARY_PATH=${AGRELEASE}/${AG_LIB_PATH}/:${LD_LIBRARY_PATH}
  fi
done
echo "Adding $AGRELEASE/bin/lib to LD_LIBRARY_PATH"
export LD_LIBRARY_PATH="$AGRELEASE/bin/lib:"$LD_LIBRARY_PATH


#Set up Root include path
for AG_ROOT_LIB_PATH in ana/include analib/include rootUtils/include aged recolib/include a2lib/include a2lib/legacy; do
  if echo "${ROOT_INCLUDE_PATH}" | grep "${AGRELEASE}/${AG_ROOT_LIB_PATH}/" > /dev/null; then
    NOTHING_TO_DO=1
  else
#    echo "Adding ${AG_ROOT_LIB_PATH} to ROOT_INCLUDE_PATH"
    export  ROOT_INCLUDE_PATH=${AGRELEASE}/${AG_ROOT_LIB_PATH}/:${ROOT_INCLUDE_PATH}
  fi
done
echo "Adding $AGRELEASE/bin/include to ROOT_INCLUDE_PATH"
export ROOT_INCLUDE_PATH=${AGRELEASE}/bin/include:${ROOT_INCLUDE_PATH}

#Add scripts to BIN path
for AG_BIN_PATH in scripts bin bin/simulation; do
  if echo ${PATH} | grep "${AGRELEASE}/${AG_BIN_PATH}/" > /dev/null; then
    NOTHING_TO_DO=1
  else
    echo "Adding ${AGRELEASE}/${AG_BIN_PATH} to PATH"
    export  PATH=${AGRELEASE}/${AG_BIN_PATH}/:${PATH}
  fi
done



#Quit if ROOT is setup...
if [ ${#ROOTSYS} -lt 3 ]; then
    echo "Please setup root manually (or run . agconfig.sh"
else
    echo "ROOTSYS set... not over writing: $ROOTSYS"
fi

if [ -z ${MIDASSYS+x} ]; then
    echo "MIDASSYS not set, continuing without MIDAS"
else
    echo "MIDASSYS set: $MIDASSYS"
fi

#Setup ROOT if we havn't quit yet...
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
      #agana
  fi
  ;;
*.triumf.ca )
  echo -e " \e[33m alphaXXtriumf.ca or daqXX.triumf.ca  detected...\033[0m"
  export AGMIDASDATA="/daq/alpha_data0/acapra/alphag/midasdata"
  export MIDASDATA="/daq/alpha_agmini/data"
  if [ `whoami` = "acapra" ] ; then
      export DATADIR=/daq/alpha_data0/acapra/alphag
      export MCDATA=${DATADIR}/MCdata
      export GARFIELDPP=${DATADIR}/GPPdata
      export AGOUTPUT="/daq/alpha_data0/acapra/alphag/output"
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
  if [ `whoami` = "acapra" ] ; then
      acapra
  fi
  ;;
* )
  if [ -n "${ROOTSYS}" ]; then
    echo "$ROOTSYS seems to be set ok"
  else
    if [ `which root-config | wc -c` -gt 5 ]; then
      echo "ROOTSYS not set but root-config found... ok"
      ROOTLIBPATH=`root-config --libdir`
      echo "Adding ${ROOTLIBPATH} to LD_LIBRARY_PATH"
      export LD_LIBRARY_PATH="${ROOTLIBPATH}:${LD_LIBRARY_PATH}"
      
    else
      echo "ROOTSYS not set... Guessing settings for new computer..."
      if [ -d "/cvmfs/sft.cern.ch/lcg/releases/gcc/4.8.4/x86_64-centos7/" ]; then
        echo "cvmfs found..."
        lxplus
      else
        echo "\tFAILED TO FIND ROOT! I don't know what to do"
      fi
    fi
  fi
  echo 'gcc       :' `which gcc`
  echo 'g++       :' `which g++`
  echo 'c++       :' `which c++`
  echo 'cc        :' `which cc`
  echo "ROOTSYS   : ${ROOTSYS}"
  if [ `which root-config | wc -c` -gt 5 ]; then
  echo 'root      :' `root-config --version`
  fi
  echo "AGRELEASE : ${AGRELEASE}"
  ;;
esac
