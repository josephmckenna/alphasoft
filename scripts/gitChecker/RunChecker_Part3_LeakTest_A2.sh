#!/bin/bash
set -e

RUNNO=${1}
if [ `echo "${RUNNO}" | wc -c` -gt 3 ]; then
  echo "Running for RUNNO=${RUNNO}"
else
  RUNNO=39993
  echo "Using default RUNNO of ${RUNNO}"
fi

if [ `echo "${AGRELEASE}" | wc -c` -gt 3 ]; then
  echo "AGRELEASE set ok: $AGRELEASE"
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi

. ${AGRELEASE}/variables


cd $AGRELEASE/alpha2
export EOS_MGM_URL=root://eospublic.cern.ch

if [ ! -f ${AGRELEASE}/run${RUNNO}sub00000.mid.gz  ]; then
  eos cp /eos/experiment/alpha/midasdata/run${RUNNO}sub00000.mid.gz ${AGRELEASE}/
else
  echo "run${RUNNO}sub00000.mid.gz found locally"
fi

GITHASH=`git rev-parse --short HEAD`
#Fails when detached:
#BRANCH=`git branch | grep \* | cut -c 3-`
BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `

mkdir -p ${AGRELEASE}/${GITHASH}/A2LeakTest/

#VALGRIND might be out of memory when running alphaStrips?
cd $AGRELEASE/scripts/A2UnitTest
./CheckProgram.sh -p alphaStrips.exe -r ${RUNNO} -b NOBUILD -t LEAK
#Copy alphaStrips result
cp -v $( ls -tr | tail -n 4 ) ${AGRELEASE}/${GITHASH}/A2LeakTest

#Force alphaStrips to run again, since the above is crashing
#cd $AGRELEASE/bin
#./alphaStrips.exe run${RUNNO}sub00000.mid.gz &> ${AGRELEASE}/${GITHASH}/A2LeakTest/S${RUNNO}.log

#Now test alphaAnalysis
cd $AGRELEASE/scripts/A2UnitTest
./CheckProgram.sh -p alphaAnalysis.exe -r ${RUNNO} -b NOBUILD -t LEAK
cp -v $( ls -tr | tail -n 4 ) ${AGRELEASE}/${GITHASH}/A2LeakTest

if [[ $(hostname -s) = *runner* ]]; then

   if [ ${ELOG_NO} -gt 15000 ]; then
      echo "Elog number: ${ELOG_NO} seems ok"
   else
      echo "No elog number set or invalid (ELOG_NO)... not posting anything"
      exit
   fi

   cd ${AGRELEASE}/${GITHASH}/A2LeakTest
   ls
   if [ `ls *.nopid | wc -l` -gt 0 ]; then
      tail -n 18 *.nopid &> annotatedLeaks.txt
      head -50 annotatedLeaks.txt &> elogMessage.txt
   fi

   echo "Gitlab runner identified! Making an elog post"

   #Prepare files for elog command
   HOSTNAME=`hostname`
   for file in `ls ${AGRELEASE}/${GITHASH}/A2LeakTest`; do
     if [ "${file}" == "elogMessage.txt" ]; then
       continue
     fi
     FILES="$FILES -f ~/gitCheckerReports/${GITHASH}/A2LeakTest/${file}"
   done
   echo "Files to attach: ${FILES}"
   scp -r ${AGRELEASE}/${GITHASH}/A2LeakTest alpha@alphadaq:~/gitCheckerReports/${GITHASH}/

   echo "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH}) - A2LeakTest\"  -r $ELOG_NO -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/A2LeakTest/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v "
   ssh -X alpha@alphadaq "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH}) - A2LeakTest\"  -r $ELOG_NO -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/A2LeakTest/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v " 
   # &> elog_posting.log
   #cat elog_posting.log



fi
