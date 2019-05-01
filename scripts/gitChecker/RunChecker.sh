#!/bin/bash
set -e

RUNNO=${1}
if [ `echo "${RUNNO}" | wc -c` -gt 3 ]; then
  echo "Running for RUNNO=${RUNNO}"
else
  #RUNNO=02364
  #RUNNO=03213
  RUNNO=03586 #Has magnetic field, Has Bars
  echo "Using default RUNNO of ${RUNNO}"
fi

if [ `echo "${AGRELEASE}" | wc -c` -gt 3 ]; then
  echo "AGRELEASE set ok: $AGRELEASE"
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi


start=`date +%s`

mkdir -p $AGRELEASE/testlogs

cd $AGRELEASE

export EOS_MGM_URL=root://eospublic.cern.ch

if [ ! -f run${RUNNO}sub000.mid.lz4  ]; then
  eos cp /eos/experiment/ALPHAg/midasdata_old/run${RUNNO}sub000.mid.lz4 .
else
  echo "run${RUNNO}sub000.mid.lz4 found locally"
fi

#Calling -h returns with a non-zero exit code
#./agana.exe -h
#Calling with a fake input file and --help finishes with a exit code 0 (not fail)

GITHASH=`git rev-parse --short HEAD`
#Fails when detached:
#BRANCH=`git branch | grep \* | cut -c 3-`
BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `

mkdir -p $AGRELEASE/testlogs
start_ana=`date +%s`
rm -vf $AGRELEASE/LookUp*.dat
echo "Running: ./agana.exe run${RUNNO}sub000.mid.lz4 -- --usetimerange 0. 15.0 --time"
./agana.exe run${RUNNO}sub000.mid.lz4 -- --usetimerange 0. 15.0 --time &> $AGRELEASE/testlogs/agana_run_${RUNNO}_${GITHASH}.log

if [ ! -f run02364sub000.mid.lz4  ]; then
  eos cp /eos/experiment/ALPHAg/midasdata_old/run02364sub000.mid.lz4 .
else
  echo "run02364sub000.mid.lz4 found locally"
fi
echo "Running: ./agana.exe run02364sub000.mid.lz4 -- --usetimerange 0. 5.0 --time"
./agana.exe run02364sub000.mid.lz4 -- --usetimerange 0. 5.0 --time &> $AGRELEASE/testlogs/agana_run_02364_${GITHASH}.log
if [ `ls $AGRELEASE/testlogs/agana_run_02364_* | wc -l` -gt 1 ]; then
   echo "Making diff of analysis..."
   #Catch exit state (1 if there is a differnce) with ||
   diff `ls -tr $AGRELEASE/testlogs/agana_run_02364_* | tail -n 2 ` > $AGRELEASE/testlogs/AnalysisDiff.log || 
   if [ -f $AGRELEASE/testlogs/AnalysisDiff.log ]; then
       cat $AGRELEASE/testlogs/AnalysisDiff.log
   fi
fi
end_ana=`date +%s`
mtstart_ana=`date +%s`
./agana.exe -mt run${RUNNO}sub000.mid.lz4 -- --usetimerange 0. 15.0 --time &> $AGRELEASE/testlogs/mt_agana_run_${RUNNO}_${GITHASH}.log
mtend_ana=`date +%s`

tail -n 50 $AGRELEASE/testlogs/agana_run_${RUNNO}_${GITHASH}.log
echo ".L macros/ReadEventTree.C 
ReadEventTree()
.q
" | root -l -b *${RUNNO}*.root &> $AGRELEASE/testlogs/ReadEventTree_${RUNNO}_${GITHASH}.log

#Move git logs to alphadaq

mkdir -p ~/${GITHASH}
cp -v $AGRELEASE/BuildLog.txt ~/${GITHASH}/
if [ -f $AGRELEASE/LastBuildLog.txt ]; then
   diff -u $AGRELEASE/LastBuildLog.txt $AGRELEASE/BuildLog.txt > ~/${GITHASH}/BuildDiff.log
fi
cp -v $AGRELEASE/testlogs/agana_run_${RUNNO}_${GITHASH}.log ~/${GITHASH}/
cp -v $AGRELEASE/testlogs/mt_agana_run_${RUNNO}_${GITHASH}.log ~/${GITHASH}/
cp -v $AGRELEASE/testlogs/agana_run_02364_${GITHASH}.log ~/${GITHASH}/

if [ -f $AGRELEASE/testlogs/AnalysisDiff.log ]; then
  cp -v $AGRELEASE/testlogs/AnalysisDiff.log ~/${GITHASH}/
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
   aganatime=$((end_ana-start_ana))
   mtaganatime=$((mtend_ana-mtstart_ana))
   echo "RunChecker time: ${runtime}s (agana time st/mt: ${aganatime}/${mtaganatime}s) " >  ~/${GITHASH}/elogMessage.txt
   git log -n 1  | tr -d '"' | tr -d "'" | tr -d '`'>> ~/${GITHASH}/elogMessage.txt
   ERRORS=`grep -i Error $AGRELEASE/BuildLog.txt | wc -l`
   WARNINGS=`grep -i Warning $AGRELEASE/BuildLog.txt | wc -l`
   echo "${ERRORS} Error and ${WARNINGS} Warnings during build..." >> ~/${GITHASH}/elogMessage.txt
   echo "Analysis Diff:" >> ~/${GITHASH}/elogMessage.txt
   if [ -f  ~/${GITHASH}/AnalysisDiff.log ]; then
   cat ~/${GITHASH}/AnalysisDiff.log >> ~/${GITHASH}/elogMessage.txt
   fi
   echo ""  >> ~/${GITHASH}/elogMessage.txt
   echo "Analysis tail:" >> ~/${GITHASH}/elogMessage.txt
   tail -n 15 $AGRELEASE/testlogs/agana_run_${RUNNO}_${GITHASH}.log >> ~/${GITHASH}/elogMessage.txt
   
   echo ""  >> ~/${GITHASH}/elogMessage.txt
   echo "Single thread/ Multithread tail diff:"  >> ~/${GITHASH}/elogMessage.txt
   diff -u <(tail -n 15 $AGRELEASE/testlogs/agana_run_${RUNNO}_${GITHASH}.log) <(tail -n 15 $AGRELEASE/testlogs/mt_agana_run_${RUNNO}_${GITHASH}.log)  >> ~/${GITHASH}/elogMessage.txt ||
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
#./agana.exe fakefile -- --help
#echo "Add more here"
#Set up variables for next job:

