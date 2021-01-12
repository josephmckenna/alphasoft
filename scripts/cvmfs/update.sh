

THIS_SETUP=$1


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
      #I must leave cvmfs to publish the changes after git pull etc
      cd ${HOME}
      cvmfs_server publish alpha.cern.ch
      sleep 5
      sync
      sleep 10
      #Git pull done, now go ahead and rebuild all valid views (in views.list)
      cd ${THIS_PATH}
      for i in `cat views.list`; do
         source ${THIS_PATH}/update.sh ${i}
      done
      echo "All done"
      return
   else
      echo "Nothing to pull.. skipping rebuild"
      cd ${HOME}
      cvmfs_server publish alpha.cern.ch
      return
   fi

fi

if [ `echo "${THIS_SETUP}" | grep '.sh' | wc -l` -eq 1 ]; then
   #do stuff
   sleep 5
   cvmfs_server transaction alpha.cern.ch
   echo ${THIS_SETUP}
   source ${THIS_SETUP}
   cd ${THIS_PATH}
   source ../../agconfig.sh
   echo ${JUPYTER_PATH}
   LCG_VERSION=`echo ${JUPYTER_PATH} | awk -F: '{print $1}' | awk -F/ '{print $6"/"$7}' `
   LCG_VERSION_NAME=`echo ${JUPYTER_PATH} | awk -F: '{print $1}' | awk -F/ '{print $6"_"$7}' `
   mkdir -p ${AGRELEASE}/${LCG_VERSION}_build
   mkdir -p ${AGRELEASE}/alphasoft/${LCG_VERSION}
   cd ${AGRELEASE}/${LCG_VERSION}_build
   #Check if we need cmake3 command or cmake
   if [ `which cmake3` ]; then
      #command cmake3 found... lets use it
      export CMAKE=cmake3
   else
      #cmake3 not found. Default version of cmake is probably 3
      export CMAKE=cmake
   fi
   ${CMAKE} ${AGRELEASE} -DCMAKE_INSTALL_PREFIX=${AGRELEASE}/${LCG_VERSION} &> ${AGRELEASE}/../alphasoft_${LCG_VERSION_NAME}_build.log
   make &>> ${AGRELEASE}/../alphasoft_${LCG_VERSION_NAME}_build.log
   make install  &>> ${AGRELEASE}/../alphasoft_${LCG_VERSION_NAME}_build.log
   echo "source ${THIS_SETUP}" > ${AGRELEASE}/${LCG_VERSION}/setup.sh
   echo "source ${AGRELEASE}/agconfig.sh " ?> ${AGRELEASE}/${LCG_VERSION}/setup.sh
   cd ..
   rm -rf ${AGRELEASE}/${LCG_VERSION}_build
   cd ${HOME}
   cvmfs_server publish alpha.cern.ch
   sleep 10
else
   echo "Probably bad view: ${THIS_SETUP}"
fi