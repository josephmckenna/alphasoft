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


start=`date +%s`

mkdir -p $AGRELEASE/testlogs

cd $AGRELEASE/ana

export EOS_MGM_URL=root://eospublic.cern.ch

if [ ! -f run${RUNNO}sub000.mid.lz4  ]; then
  eos cp /eos/experiment/alpha/run${RUNNO}sub000.mid.lz4 .
else
  echo "run${RUNNO}sub000.mid.lz4 found locally"
fi

#Calling -h returns with a non-zero exit code
#./agana.exe -h
#Calling with a fake input file and --help finishes with a exit code 0 (not fail)

GITHASH=`git rev-parse --short HEAD`
#Fails when detached:
#BRANCH=`git branch | grep \* | cut -c 3-`
BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" `

mkdir -p $AGRELEASE/testlogs
start_ana=`date +%s`
./agana.exe run${RUNNO}sub000.mid.lz4 -- --usetimerange 0. 1.0 &> $AGRELEASE/testlogs/agana_run_${RUNNO}_${GITHASH}.log
#./agana.exe run02364sub000.mid.lz4 -- ---useeventrange  0 2 | tee test-results/agana_run_${RUNNO}.log

end_ana=`date +%s`
tail -n 50 $AGRELEASE/testlogs/agana_run_${RUNNO}_${GITHASH}.log

echo ".L macros/ReadEventTree.C 
ReadEventTree()
.q
" | root -l -b *${RUNNO}*.root

echo "Leak test:"
cd $AGRELEASE/scripts/UnitTest/
./LeakCheck.sh ${RUNNO} NOBUILD 20
echo "Moving these files:"
ls -tr | tail -n 4
cp -v $( ls -tr | tail -n 4 ) $AGRELEASE/testlogs/
echo "Test logs:"
ls  $AGRELEASE/testlogs/

#Move git logs to alphadaq

mkdir -p ~/${GITHASH}
cp $AGRELEASE/ana/BuildLog.txt ~/${GITHASH}/
if [ -f $AGRELEASE/ana/LastBuildLog.txt ]; then
   diff $AGRELEASE/ana/LastBuildLog.txt $AGRELEASE/ana/BuildLog.txt > ~/${GITHASH}/BuildDiff.log
fi
cp $AGRELEASE/testlogs/agana_run_${RUNNO}_${GITHASH}.log ~/${GITHASH}/
cp -v $( ls -tr | tail -n 4 ) ~/${GITHASH}/
cp LeakDiff.log AnalysisDiff.log  MacroDiff.log  ~/${GITHASH}/
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
   echo "RunChecker time: ${runtime}s (agana time: ${aganatime}s) " >  ~/${GITHASH}/elogMessage.txt
   git log -n 1  | tr -d '"' | tr -d "'" | tr -d '`'>> ~/${GITHASH}/elogMessage.txt
   ERRORS=`grep -i Error $AGRELEASE/ana/BuildLog.txt | wc -l`
   WARNINGS=`grep -i Warning $AGRELEASE/ana/BuildLog.txt | wc -l`
   echo "${ERRORS} Error and ${WARNINGS} Warnings during build..." >> ~/${GITHASH}/elogMessage.txt
   echo "Analysis Diff:" >> ~/${GITHASH}/elogMessage.txt
   cat ~/${GITHASH}/AnalysisDiff.log >> ~/${GITHASH}/elogMessage.txt
   tail ~/${GITHASH}/LeakTest*.log.nopid -n 17 >> ~/${GITHASH}/elogMessage.txt 
   #Limit the size of the elogMessage
   if [ `cat ~/${GITHASH}/elogMessage.txt | wc -l` -gt 400 ]; then
      mv ~/${GITHASH}/elogMessage.txt ~/${GITHASH}/elogMessage_full.txt
      head -n 350 ~/${GITHASH}/elogMessage_full.txt > ~/${GITHASH}/elogMessage.txt
      echo "Message too long... cutting off at 350 lines..." >> ~/${GITHASH}/elogMessage.txt
   fi

   #Move files to alphadaq (so that they can be added to elog post)
   scp -r ~/${GITHASH} alpha@alphadaq:~/gitCheckerReports/
   echo "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH})\" -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v "
   ssh -X alpha@alphadaq "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH})\" -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v "
fi
#./agana.exe fakefile -- --help
#echo "Add more here"
