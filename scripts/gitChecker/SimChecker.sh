#!/bin/bash
set -e
. ${AGRELEASE}/variables

GITHASH=`git rev-parse --short HEAD`
BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `

mkdir -p $AGRELEASE/simlogs

start_ana=`date +%s`

cd ${AGRELEASE}/simulation
rm *.root
./AGTPC runHeedInterface.mac &>$AGRELEASE/simlogs/simulation_${GITHASH}.log
#cp $AGRELEASE/simlogs/simulation_${GITHASH}.log ~/${GITHASH}/

cd ../reco
ROOT_FILE=`ls ../simulation/*.root`
echo "root file: ${ROOT_FILE}"
./g4ana.exe --rootfile ${ROOT_FILE} &>$AGRELEASE/simlogs/analysis_of_sim_${GITHASH}.log
#cp $AGRELEASE/simlogs/analysis_${GITHASH}.log ~/${GITHASH}/


if [[ $(hostname -s) = *runner* ]]; then

   if [ ${ELOG_NO} -gt 15000 ]; then
      echo "Elog number: ${ELOG_NO} seems ok"
   else
      echo "No elog number set or invalid (ELOG_NO)... not posting anything"
      exit
   fi

   echo "Gitlab runner identified! Making an elog post"

   #Prepare files for elog command
   HOSTNAME=`hostname`
   FILES="$FILES -f ~/gitCheckerReports/${GITHASH}/simlogs/imulation_${GITHASH}.log"
   FILES="$FILES -f ~/gitCheckerReports/${GITHASH}/simlogs/analysis_of_sim_${GITHASH}.log"
   #for file in `ls ${AGRELEASE}/${GITHASH}/SpeedTest`; do
   #  FILES="$FILES -f ~/gitCheckerReports/${GITHASH}/SimTest/${file}"
   #done
   echo "Files to attach: ${FILES}"
   scp -r $AGRELEASE/simlogs alpha@alphadaq:~/gitCheckerReports/${GITHASH}/

   echo "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH}) - SpeedTest\"  -r $ELOG_NO -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/SpeedTest/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v "
   ssh -X alpha@alphadaq "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH}) - SpeedTest\"  -r $ELOG_NO -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/SpeedTest/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v " 
   # &> elog_posting.log
   #cat elog_posting.log



fi


