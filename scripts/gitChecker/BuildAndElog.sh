#!/bin/bash
set -e

if [ `echo "${AGRELEASE}" | wc -c` -gt 3 ]; then
  echo "AGRELEASE set ok: $AGRELEASE"
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi


start=`date +%s`

GITHASH=`git rev-parse --short HEAD`
#Fails when detached:
#BRANCH=`git branch | grep \* | cut -c 3-`
BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `

cd ${AGRELEASE}
rm -rf build bin
mkdir build
cd build
cmake3 -DBUILD_AG_SIM=ON -DBUILD_A2_SIM=ON -DBUILD_A2=ON -DBUILD_AG=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../bin ../
cmake3 --build .  2>&1 | tee ../BuildLog.txt
cmake3 --build . --target install  2>&1 | tee -a ../BuildLog.txt


#Move git logs to alphadaq

mkdir -p ~/${GITHASH}
cp -v $AGRELEASE/BuildLog.txt ~/${GITHASH}/
if [ -f $AGRELEASE/LastBuildLog.txt ]; then
   diff -u $AGRELEASE/LastBuildLog.txt $AGRELEASE/BuildLog.txt > ~/${GITHASH}/BuildDiff.log || :
fi
end=`date +%s`

if [[ $(hostname -s) = *runner* ]]; then
   echo "Gitlab runner identified! Making an elog post"

   #Prepare files for elog command
   HOSTNAME=`hostname`
   for file in `ls ~/${GITHASH}/`; do
     FILES="$FILES -f ~/gitCheckerReports/${GITHASH}/${file}"
   done
   echo "Files to attach: ${FILES}"

   #Elog message:
   runtime=$((end-start))
   git log -n 1  | tr -d '"' | tr -d "'" | tr -d '`'>> ~/${GITHASH}/elogMessage.txt
   ERRORS=`grep -i Error $AGRELEASE/BuildLog.txt | wc -l`
   WARNINGS=`grep -i Warning $AGRELEASE/BuildLog.txt | wc -l`
   echo "${ERRORS} Error and ${WARNINGS} Warnings during build..." >> ~/${GITHASH}/elogMessage.txt
   echo ""  >> ~/${GITHASH}/elogMessage.txt
   grep -i 'Warning\|Error' >> ~/${GITHASH}/elogMessage.txt
   #Limit the size of the elogMessage
   if [ `cat ~/${GITHASH}/elogMessage.txt | wc -l` -gt 400 ]; then
      mv ~/${GITHASH}/elogMessage.txt ~/${GITHASH}/elogMessage_full.txt
      head -n 350 ~/${GITHASH}/elogMessage_full.txt > ~/${GITHASH}/elogMessage.txt
      echo "Message too long... cutting off at 350 lines..." >> ~/${GITHASH}/elogMessage.txt
   fi

   #Move files to alphadaq (so that they can be added to elog post)
   scp -r ~/${GITHASH} alpha@alphadaq:~/gitCheckerReports/
   echo "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH})\" -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v "
   ssh -X alpha@alphadaq "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH})\" -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v " &> elog_posting.log
   echo "Tail of elog:"
   tail -n 100 elog_posting.log
   ELOG_NO=`cat elog_posting.log  | grep ID= | tr 'Message successfully transmitted, ID=' "\n"| grep [0-9] | tail -n 1`
   echo "export ELOG_NO=$ELOG_NO" > ${AGRELEASE}/variables
fi
