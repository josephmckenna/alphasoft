#!/bin/bash
set -e
. ${AGRELEASE}/variables

GITHASH=`git rev-parse --short HEAD`
BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `

mkdir -p $AGRELEASE/simlogs

start=`date +%s`

cd ${AGRELEASE}/simulation
rm *.root
./AGTPC runHeedInterface.mac &>$AGRELEASE/simlogs/simulation_${GITHASH}.log
#cp $AGRELEASE/simlogs/simulation_${GITHASH}.log ~/${GITHASH}/

cd ../reco
ROOT_FILE=`ls ../simulation/*.root`
echo "root file: ${ROOT_FILE}"
./g4ana.exe --rootfile ${ROOT_FILE} &>$AGRELEASE/simlogs/analysis_of_sim_${GITHASH}.log
#cp $AGRELEASE/simlogs/analysis_${GITHASH}.log ~/${GITHASH}/
end=`date +%s`

   #Elog message:
   runtime=$((end-start))
   aganatime=$((end_ana-start_ana))
   mtaganatime=$((mtend_ana-mtstart_ana))
   echo "Simulation time: ${runtime}s (agana time st/mt: ${aganatime}/${mtaganatime}s) " >  $AGRELEASE/simlogs/elogMessage.txt
   echo "Making diff of analysis..."
   #Catch exit state (1 if there is a differnce) with ||
   echo "Diff of analysis:
   
   " >>$AGRELEASE/simlogs/elogMessage.txt
   diff -u `ls -tr $AGRELEASE/simlogs/analysis_of_sim_* | tail -n 2 ` > $AGRELEASE/simlogs/AnalysisDiff.log || :
   if [ -f $AGRELEASE/simlogsAnalysisDiff.log ]; then
       cat $AGRELEASE/simlogs/AnalysisDiff.log
       cat $AGRELEASE/simlogs/AnalysisDiff.log >> $AGRELEASE/simlogs/elogMessage.txt
   fi

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
   FILES="$FILES -f ~/gitCheckerReports/${GITHASH}/simlogs/simulation_${GITHASH}.log"
   FILES="$FILES -f ~/gitCheckerReports/${GITHASH}/simlogs/analysis_of_sim_${GITHASH}.log"
   #for file in `ls ${AGRELEASE}/${GITHASH}/SpeedTest`; do
   #  FILES="$FILES -f ~/gitCheckerReports/${GITHASH}/SimTest/${file}"
   #done
   echo "Files to attach: ${FILES}"
   scp -r $AGRELEASE/simlogs alpha@alphadaq:~/gitCheckerReports/${GITHASH}/

   echo "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH}) - AGSimulation\"  -r $ELOG_NO -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/simlogs/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v "
   ssh -X alpha@alphadaq "~/packages/elog/elog -h localhost -a Author=${HOSTNAME} -a Run=\"${RUNNO}\" -a Subject=\"git-checker: $GITHASH (${BRANCH}) - AGSimulation\"  -r $ELOG_NO -a Tags=\"gitcheck\" -m ~/gitCheckerReports/${GITHASH}/simlogs/elogMessage.txt ${FILES}  -p 8080 -l AutoAnalysis -v " 
   # &> elog_posting.log
   #cat elog_posting.log



fi


