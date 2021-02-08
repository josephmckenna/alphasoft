#!/bin/bash

THIS_SETUP=$1
echo "update.sh args: ${THIS_SETUP}"

#become shared user and then run this script
#if [ `whoami` != "cvalpha" ]; then
#   echo "I am not cvalpha... please run me as cvalpha"
#   echo "ssh cvmfs-alpha.cern.ch"
#   echo "sudo -i -u cvalpha"
#   return
#fi

#Path is this file:

if [ ${THIS_SETUP} == "update_git" ]; then
   cvmfs_server transaction alpha.cern.ch
   cd /cvmfs/alpha.cern.ch/alphasoft
   if [ `git pull | wc -l` -gt 1 ]; then
      sleep 5
      cp -v scripts/cvmfs/update.sh ~/
      cp -v scripts/cvmfs/views.list ~/
      git submodule update --remote
      #I must leave cvmfs to publish the changes after git pull etc
      cd ~/
      cvmfs_server publish alpha.cern.ch
      sleep 10
      #Git pull done, now go ahead and rebuild all valid views (in views.list)
      ./update.sh build
      echo "All done" 
      return
   else
      echo "Nothing to pull.. skipping rebuild"
      return
   fi
   cd ~/
   cvmfs_server publish alpha.cern.ch
elif [ ${THIS_SETUP} == "build" ]; then
   cd ~/
   for i in `cat views.list`; do
      # Run each version as a subprocess to avoid polluting the ENVVARs
      ./update.sh ${i}
   done
elif [ `echo "${THIS_SETUP}" | grep '.sh' | wc -l` -eq 1 ]; then
   #do stuff
   cd /cvmfs/alpha.cern.ch/alphasoft
   cvmfs_server transaction alpha.cern.ch
   echo ${THIS_SETUP}
   source ${THIS_SETUP}
   source agconfig.sh
   echo ${JUPYTER_PATH}
   LCG_VERSION_PATH=`echo ${JUPYTER_PATH} | awk -F: '{print $1}' | awk -F/ '{print $6"/"$7}' `
   LCG_VERSION_NAME=`echo ${JUPYTER_PATH} | awk -F: '{print $1}' | awk -F/ '{print $6"_"$7}' `
   #Insure a clean build dir
   rm -rf ${AGRELEASE}/${LCG_VERSION_PATH}_build
   mkdir -p ${AGRELEASE}/${LCG_VERSION_PATH}_build
   INSTALL_PATH="/cvmfs/alpha.cern.ch/alphasoft/${LCG_VERSION_PATH}"
   echo "INSTALL PATH: ${INSTALL_PATH}"
   mkdir -p ${INSTALL_PATH}
   cd ${AGRELEASE}/${LCG_VERSION_PATH}_build
   #Check if we need cmake3 command or cmake
   if [ `command -v cmake3` ]; then
      #command cmake3 found... lets use it
      export CMAKE=cmake3
   else
      #cmake3 not found. Default version of cmake is probably 3
      export CMAKE=cmake
   fi
   rm -vf ${INSTALL_PATH}/alphasoft_${LCG_VERSION_NAME}_build.log 
   ${CMAKE} ${AGRELEASE} -DCMAKE_INSTALL_PREFIX="${INSTALL_PATH}" \
                         -DPRE_CONFIG=${THIS_SETUP} \
                 &> ~/alphasoft_${LCG_VERSION_NAME}_build.log
   make          &>> ~/alphasoft_${LCG_VERSION_NAME}_build.log
   make install  &>> ~/alphasoft_${LCG_VERSION_NAME}_build.log  && \
   echo "Build OK! Date: `date` GitVersion: `git log -1 --format=%h`" >> ~/alphasoft_${LCG_VERSION_NAME}_build.log 
   cp ~/alphasoft_${LCG_VERSION_NAME}_build.log ${INSTALL_PATH}/build.log
   cd /cvmfs/alpha.cern.ch/alphasoft
   echo "Cleaning up build path: ${AGRELEASE}/${LCG_VERSION_PATH}_build"
   rm -rf ${AGRELEASE}/${LCG_VERSION_PATH}_build
   cd ${HOME}
   cvmfs_server publish alpha.cern.ch
   sleep 10
else
   echo "Probably bad view: ${THIS_SETUP}"
fi