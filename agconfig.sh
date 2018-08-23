

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




echo "############## agconfig.sh ##############" 
echo "Hostname: " `hostname`
echo "Username: " `whoami`
echo "#########################################"

#Setup LD_LIBRARY_PATH
for AG_LIB_PATH in ana analib aged; do
  if echo "${LD_LIBRARY_PATH}" | grep "${AGRELEASE}/${AG_LIB_PATH}/" > /dev/null; then
    NOTHING_TO_DO=1
  else
    echo "Adding ${AG_LIB_PATH} to LD_LIBRARY_PATH"
    export  LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${AGRELEASE}/${AG_LIB_PATH}/
  fi
done


#Quit if ROOT and ROOTANA are setup...
if [ "${1}" == "clean" ]; then
  echo "Clean setup of environment variables"
else
  if [ ${#ROOTANASYS} -gt 3 ]; then
    echo "ROOTANASYS set... not over writing"
    if [ ${#ROOTSYS} -lt 3 ]; then
      echo "Please setup root manually (or run . agconfig.sh clean)"
    else
      echo "ROOTSYS set... not over writing"
    fi
    return;
  fi
  if [ ${#ROOTSYS} -gt 3 ]; then
    echo "ROOTSYS set... not over writing"
    if [ ${#ROOTANASYS} -lt 3 ]; then
      echo "Please setup rootana manually (or run . agconfig.sh clean)"
    else
      echo "ROOTANASYS set... not over writing"
    fi
    return;
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
  ;;
* )
  echo "ROOTSYS and ROOTANASYS not set... Guessing settings for new computer..."
  ;;
esac


