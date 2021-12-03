

#!/bin/bash
PARAMS=""
while (( "$#" )); do
  case "$1" in
    -a|--my-boolean-flag)
      MY_FLAG=0
      shift
      ;;
    -e|--executable)
      if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
        EXECUTABLE=$2
        shift 2
      else
        echo "Error: Argument for $1 is missing" >&2
        exit 1
      fi
      ;;
    -j|--jobname)
      if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
        JOBNAME=$2
        shift 2
      else
        echo "Error: Argument for $1 is missing" >&2
        exit 1
      fi
      ;;
    -r|--runno)
      if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
        RUNNO=$2
        shift 2
      else
        echo "Error: Argument for $1 is missing" >&2
        exit 1
      fi
      ;;
    -o|--outputpath)
      if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
        OUTPUT_PATH=$2
        shift 2
      else
        echo "Error: Argument for $1 is missing" >&2
        exit 1
      fi
      ;;
    -l|--logpath)
      if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
        LOG_PATH=$2
        shift 2
      else
        echo "Error: Argument for $1 is missing" >&2
        exit 1
      fi
      ;;
    -*|--*=) # unsupported flags
      echo "Error: Unsupported flag $1" >&2
      exit 1
      ;;
    *) # preserve positional arguments
      PARAMS="$PARAMS $1"
      shift
      ;;
  esac
done
# set positional arguments in their proper place
eval set -- "$PARAMS"


SETTINGS_GOOD=1
RED=`tput setaf 1`
#Check settings
if [ -z "${RUNNO}" ]; then
   echo "${RED}Please set run number with \"-r 12345\""
   SETTINGS_GOOD=0
fi

if [ -z "${EXECUTABLE}" ]; then
   echo "${RED}Please specify the executable with \"-e myexe.exe\""
   SETTINGS_GOOD=0
fi

if [ -z "${JOBNAME}" ]; then
   echo "${RED}Please specify the jobname with \"-j MyJob\" (please do not use spaces)"
   SETTINGS_GOOD=0
fi

if [ -z "${OUTPUT_PATH}" ]; then
   echo "${RED}Please specify an output path (for root file) \"-o /my/user/path\" (please do not use spaces, and please use full path)"
   SETTINGS_GOOD=0
fi

if [ -z "${LOG_PATH}" ]; then
   echo "${RED}Please specify an output path (for log file) \"-l /my/user/path\" (please do not use spaces, and please use full path, do not use EOS folder)"
   SETTINGS_GOOD=0
fi


#Reset terminal text back to black
echo `tput sgr0`

if [ ${SETTINGS_GOOD} -eq 0 ]; then
   echo "Errors in arguements, not submitting jobs"
   exit
fi



MIDAS_DATA_EOS_PATH="/eos/experiment/ALPHAg/midasdata_old/"
#Use tr to remove new line characters... 
MIDAS_FILE_LIST=`ls ${MIDAS_DATA_EOS_PATH}/run0${RUNNO}sub*.mid.lz4  | tr '\n' ' '`
if [ -z ${MIDAS_FILE_LIST} ]; then
   echo "${RED}No matching MIDAS files found for run ${RUNNO}"
   echo `tput sgr0`
   exit
fi


echo "MIDAS files detected: ${MIDAS_FILE_LIST}"
DISK_SPACE=`du -c ${MIDAS_FILE_LIST} | grep total | tail -n1 | awk ' { print $1 }'`
echo "Disk usage of MIDAS data: ${DISK_SPACE}"

echo "
#Run one instance of an analysis program (for ALPHAg)
executable              = ${AGRELEASE}/bin/${EXECUTABLE}
#Pre copying the files will be better
#arguments               = \"-e2000 ~/run0${RUNNO}sub*.mid.zl4 -- --EOS --anasettings ${AGRELEASE}/ana/cern2021_1.json\"
#transfer_input_files    = ${MIDAS_FILE_LIST}
#should_transfer_files = yes

arguments               = \"-D${OUTPUT_PATH} ${MIDAS_FILE_LIST} -- --anasettings ${AGRELEASE}/ana/cern2021_2.json\"

output       = ${LOG_PATH}/${JOBNAME}.out.log
error        = ${LOG_PATH}/${JOBNAME}.err.log
log                     = ${LOG_PATH}/${JOBNAME}.log

batch_runtime = 172800
+JobFlavour = \"nextweek\"
request_cpus   = 1
request_memory = 2048
#request_disk   = ${DISK_SPACE}


getenv = *

queue
" > ${JOBNAME}.sub

echo "Job file created:
"
cat ${JOBNAME}.sub

condor_submit ${JOBNAME}.sub
