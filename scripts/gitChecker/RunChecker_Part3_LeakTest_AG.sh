#!/bin/bash
set -e

RUNNO=${1}
if [ `echo "${RUNNO}" | wc -c` -gt 3 ]; then
  echo "Running for RUNNO=${RUNNO}"
else
  #RUNNO=02364
  #RUNNO=03213
  RUNNO=03586
  echo "Using default RUNNO of ${RUNNO}"
fi

if [ `echo "${AGRELEASE}" | wc -c` -gt 3 ]; then
  echo "AGRELEASE set ok: $AGRELEASE"
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi

. ${AGRELEASE}/variables

cd $AGRELEASE
export EOS_MGM_URL=root://eospublic.cern.ch

GITHASH=`git rev-parse --short HEAD`
#Fails when detached:
#BRANCH=`git branch | grep \* | cut -c 3-`
BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `

mkdir -p ${AGRELEASE}/${GITHASH}/AGLeakTest/

rm -vf $AGRELEASE/LookUp*.dat
cd $AGRELEASE/scripts/AGUnitTest/
./CheckProgram.sh -p agana.exe -r ${RUNNO} -b NOBUILD -t LEAK -l 1500
cp -v $( ls -tr  *LEAK*.log *LEAK*.nopid | tail -n 4  ) ${AGRELEASE}/${GITHASH}/AGLeakTest

if [[ $(hostname -s) = *runner* ]]; then

   if [ ${ELOG_NO} -gt 15000 ]; then
      echo "Elog number: ${ELOG_NO} seems ok"
   else
      echo "No elog number set or invalid (ELOG_NO)... not posting anything"
      exit
   fi

   cd ${AGRELEASE}/${GITHASH}/AGLeakTest
   ls
   if [ `ls *.nopid | wc -l` -gt 0 ]; then
      tail -n 18 *.nopid &> annotatedLeaks.txt
      head -50 annotatedLeaks.txt &> elogMessage.txt
   fi

   echo "Gitlab runner identified! Making an elog post"

   #Prepare files for elog command
   HOSTNAME=`hostname`
   for file in `ls ${AGRELEASE}/${GITHASH}/AGLeakTest`; do
     if [ "${file}" == "elogMessage.txt" ]; then
       continue
     fi
     FILES="$FILES -f ~/gitCheckerReports/${GITHASH}/AGLeakTest/${file}"
   done
   echo "Files to attach: ${FILES}"
   scp -r ${AGRELEASE}/${GITHASH}/AGLeakTest alpha@alphadaq:~/gitCheckerReports/${GITHASH}/

   echo "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH}) - AGLeakTest\"  -r $ELOG_NO -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/AGLeakTest/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v "
   ssh -X alpha@alphadaq "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH}) - AGLeakTest\"  -r $ELOG_NO -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/AGLeakTest/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v " 
   # &> elog_posting.log
   #cat elog_posting.log



fi
