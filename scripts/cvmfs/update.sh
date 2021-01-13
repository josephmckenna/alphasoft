#!/bin/bash

THIS_SETUP=$1
THIS_RELEASE=$2
echo "update.sh args: ${THIS_SETUP} ${THIS_RELEASE}"

#become shared user and then run this script
#if [ `whoami` != "cvalpha" ]; then
#   echo "I am not cvalpha... please run me as cvalpha"
#   echo "ssh cvmfs-alpha.cern.ch"
#   echo "sudo -i -u cvalpha"
#   return
#fi

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
export THIS_PATH="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

if [ ${THIS_SETUP} == "update_git" ]; then
   cd ${THIS_PATH}
   cvmfs_server transaction alpha.cern.ch
   if [ `git pull | wc -l` -gt 1 ]; then
      cd ../../
      git submodule update --remote
      cd ${THIS_PATH}
      cp update.sh views.list ${HOME}/
      #I must leave cvmfs to publish the changes after git pull etc
      cd ${HOME}
      cvmfs_server publish alpha.cern.ch
      sleep 5
      #Git pull done, now go ahead and rebuild all valid views (in views.list), note how this is now a copy in ~/
      ./update.sh build ${THIS_PATH}../../
      echo "All done" 
      return
   else
      echo "Nothing to pull.. skipping rebuild"
      cd ${HOME}
      cvmfs_server publish alpha.cern.ch
      return
   fi
elif [ ${THIS_SETUP} == "build" ]; then
   cd ${HOME}
   for i in `cat views.list`; do
      # Run each version as a subprocess to avoid polluting the ENVVARs
      ./update.sh ${i} ${THIS_RELEASE}
   done
elif [ `echo "${THIS_SETUP}" | grep '.sh' | wc -l` -eq 1 ]; then
   #do stuff
   sleep 5
   cvmfs_server transaction alpha.cern.ch
   echo ${THIS_SETUP}
   source ${THIS_SETUP}
   cd ${THIS_RELEASE}
   source agconfig.sh
   echo ${JUPYTER_PATH}
   LCG_VERSION=`echo ${JUPYTER_PATH} | awk -F: '{print $1}' | awk -F/ '{print $6"/"$7}' `
   LCG_VERSION_NAME=`echo ${JUPYTER_PATH} | awk -F: '{print $1}' | awk -F/ '{print $6"_"$7}' `
   mkdir -p ${AGRELEASE}/${LCG_VERSION}_build
   mkdir -p ${AGRELEASE}/${LCG_VERSION}
   cd ${AGRELEASE}/${LCG_VERSION}_build
   #Check if we need cmake3 command or cmake
   if [ `command -v cmake3` ]; then
      #command cmake3 found... lets use it
      export CMAKE=cmake3
   else
      #cmake3 not found. Default version of cmake is probably 3
      export CMAKE=cmake
   fi
   ${CMAKE} ${AGRELEASE} -DCMAKE_INSTALL_PREFIX=${AGRELEASE}/${LCG_VERSION} \
                         -DPRE_CONFIG=${THIS_SETUP} \
                 &> ${AGRELEASE}/../alphasoft_${LCG_VERSION_NAME}_build.log
   make          &>> ${AGRELEASE}/../alphasoft_${LCG_VERSION_NAME}_build.log
   make install  &>> ${AGRELEASE}/../alphasoft_${LCG_VERSION_NAME}_build.log
   cd ..
   rm -rf ${AGRELEASE}/${LCG_VERSION}_build
   cd ${HOME}
   cvmfs_server publish alpha.cern.ch
   sleep 10
else
   echo "Probably bad view: ${THIS_SETUP}"
fi