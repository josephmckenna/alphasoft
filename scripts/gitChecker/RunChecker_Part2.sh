#!/bin/bash
set -e

RUNNO=${1}
if [ `echo "${RUNNO}" | wc -c` -gt 3 ]; then
  echo "Running for RUNNO=${RUNNO}"
else
  #RUNNO=02364
  RUNNO=03213
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

rm -vf $AGRELEASE/ana/LookUp*.dat
cd $AGRELEASE/scripts/UnitTest/
./SpeedTest.sh ${RUNNO} NOBUILD 100 --time



if [[ $(hostname -s) = *runner* ]]; then

   if [ ${ELOG_NO} -gt 15000 ]; then
      echo "Elog number: ${ELOG_NO} seems ok"
   else
      echo "No elog number set or invalid (ELOG_NO)... not posting anything"
      exit
   fi

   mkdir -p ${AGRELEASE}/${GITHASH}/SpeedTest/

   cp -v $( ls -tr | tail -n 3 ) ${AGRELEASE}/${GITHASH}/SpeedTest
   cd ${AGRELEASE}/${GITHASH}/SpeedTest

   callgrind_annotate SpeedTest*.out &> annotatedSpeed.txt
   head -50 annotatedSpeed.txt &> elogMessage.txt
   cp SpeedTest*.out  ${AGRELEASE}/callgrind.log
   gzip SpeedTest*.out

   echo "Gitlab runner identified! Making an elog post"

   #Prepare files for elog command
   HOSTNAME=`hostname`
   for file in `ls ${AGRELEASE}/${GITHASH}/SpeedTest`; do
     FILES="$FILES -f ~/gitCheckerReports/${GITHASH}/SpeedTest/${file}"
   done
   echo "Files to attach: ${FILES}"
   scp -r ${AGRELEASE}/${GITHASH}/SpeedTest alpha@alphadaq:~/gitCheckerReports/${GITHASH}/

   echo "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH}) - SpeedTest\"  -r $ELOG_NO -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/SpeedTest/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v "
   ssh -X alpha@alphadaq "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH}) - SpeedTest\"  -r $ELOG_NO -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/SpeedTest/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v " 
   # &> elog_posting.log
   #cat elog_posting.log



fi
