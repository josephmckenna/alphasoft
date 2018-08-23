

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




echo "############## myconfig.sh ##############" 
echo "Hostname: " `hostname`
echo "Username: " `whoami`
echo "#########################################"

if [ ${1} == "clean" ]; then
  echo "overwriting any settings"
else
  if [ ${#ROOTANASYS} -gt 3 ]; then
    echo "ROOTANASYS set... not over writing"
    return;
  fi
  if [ ${#ROOTSYS} -gt 3 ]; then
    echo "ROOTSYS set... not over writing"
    return;
  fi
fi
 
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

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
