#!/bin/bash
set -e

RUNNO=${1}
if [ `echo "${RUNNO}" | wc -c` -gt 3 ]; then
  echo "Running for RUNNO=${RUNNO}"
else
  RUNNO=02364
  echo "Using default RUNNO of ${RUNNO}"
fi

if [ `echo "${AGRELEASE}" | wc -c` -gt 3 ]; then
  echo "AGRELEASE set ok: $AGRELEASE"
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi

. ${AGRELEASE}/variables

GITHASH=`git rev-parse --short HEAD`
#Fails when detached:
#BRANCH=`git branch | grep \* | cut -c 3-`
BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `


cd $AGRELEASE/scripts/UnitTest/
./SpeedTest.sh ${RUNNO} NOBUILD 30 --time



callgrind_annotate SpeedTest*.log &> annotatedSpeed.txt
head -50 annotatedSpeed.txt &> elogMessage.log
if [[ $(hostname -s) = *runner* ]]; then

   mkdir -p ~/${GITHASH}/SpeedTest/

   cp -v $( ls -tr | tail -n 4 ) ~/${GITHASH}/SpeedTest
   cd ~/${GITHASH}/SpeedTest
   if [ `echo "${ELOG_NO}" | wc -c` -gt 3 ]; then
      echo "No elog number set (ELOG_NO)... not posting anything"
      exit
   fi

   echo "Gitlab runner identified! Making an elog post"

   #Prepare files for elog command
   HOSTNAME=`hostname`
   for file in `ls ~/${GITHASH}/SpeedTest`; do
     FILES="$FILES -f ~/gitCheckerReports/${GITHASH}/SpeedTest/${file}"
   done
   echo "Files to attach: ${FILES}"
   scp -r ~/${GITHASH}/SpeedTest alpha@alphadaq:~/gitCheckerReports/${GITHASH}/

   echo "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH}) - SpeedTest\"  -r $ELOG_NO -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/SpeedTest/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v "
   ssh -X alpha@alphadaq "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH}) - SpeedTest\"  -r $ELOG_NO -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/SpeedTest/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v " &> elog_posting.log
   cat elog_posting.log



fi
