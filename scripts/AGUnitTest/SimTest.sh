




SEED=$1
DOBUILD=$2



SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

if [ `echo "$AGRELEASE" | wc -c` -gt 3 ]; then
  echo "AGANA set ok..."
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi




if [ `echo "$SEED" | wc -c` -gt 3 ]; then
  echo "Using run $SEED"
else
  SEED=555666
  echo "Using default seed $SEED"
  DOBUILD="BUILD"
fi


BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `

cd ${DIR}
for i in `seq 1 100000`; do
   READYTOGO=1
  for logfile in SimTest${i}_${BRANCH}.log \
                  SimTest_git_diff_${i}_${BRANCH}.log \
                  SimTest_AnalysisOut_${i}_${BRANCH}.log \
                  SimTest_MacroOut_${i}_${BRANCH}.log \
                  SimTest_Build_${i}_${BRANCH}.log; do
      if [ -e ${logfile} ]; then
         ls -lh ${logfile}
         READYTOGO=0
         break
      fi
   done
   if [ ${READYTOGO} -eq 1 ]; then
      SIMTEST="$DIR/SimTest${i}_${BRANCH}.log"
      ANA_TEST="$DIR/SimTest_AnalysisOut_${i}_${BRANCH}.log "
      GITDIFF="$DIR/SimTest_git_diff_${i}_${BRANCH}.log"
      BUILDLOG="$DIR/SimTest_Build_${i}_${BRANCH}.log"
      TESTID=${i}
      break
   fi
done
if [ "$DOBUILD" != "NOBUILD" ]; then
  echo "Recompiling everything..."
  cd ${AGRELEASE}
  make clean
  cd ${AGRELEASE}/simulation
  rm -rf CMakeFiles CMakeCache.txt
  cmake3 geant4/  &> ${BUILDLOG}
  if [ "$DOBUILD" == "FASTBUILD" ]; then
    make -j &>> ${BUILDLOG}
    cd ${AGRELEASE}
    make -j &>> ${BUILDLOG}
  else
    make &>> ${BUILDLOG}
    cd ${AGRELEASE}
    make &>> ${BUILDLOG}
  fi
  echo "Recompilation done: chech ${BUILDLOG}"
  WARNING_COUNT=`grep -i warning ${BUILDLOG} | wc -l`
  ERROR_COUNT=`grep -i error ${BUILDLOG} | wc -l`
  echo "Found ${WARNING_COUNT} warning(s) and ${ERROR_COUNT} errors(s) "
fi

cd $AGRELEASE
git diff > ${GITDIFF}


cd ${AGRELEASE}/simulation
export MCDATA=${AGRELEASE}/simulation
./AGTPC runHeedInterface.mac --GarSeed ${SEED} &>${SIMTEST}
#cp $AGRELEASE/simlogs/simulation_${GITHASH}.log ~/${GITHASH}/

cd ../reco
ROOT_FILE=`ls ../simulation/*.root`
echo "root file: ${ROOT_FILE}"
./g4ana.exe --rootfile ${ROOT_FILE} &> ${ANA_TEST}
#cp $AGRELEASE/simlogs/analysis_${GITHASH}.log ~/${GITHASH}/
end=`date +%s`


if [ $TESTID -gt 1 ]; then
   BEFORE=`expr ${TESTID} - 1`
   echo diff -u "$DIR/SimTest_AnalysisOut_${BEFORE}_${BRANCH}.log" "$DIR/SimTest_AnalysisOut_${i}_${BRANCH}.log"
   diff -u "$DIR/SimTest${BEFORE}_${BRANCH}.log" "$DIR/SimTest${i}_${BRANCH}.log" > $AGRELEASE/scripts/UnitTest/LeakDiff.log
   diff -u "$DIR/SimTest_AnalysisOut_${BEFORE}_${BRANCH}.log" "$DIR/SimTest_AnalysisOut_${i}_${BRANCH}.log" > $AGRELEASE/scripts/UnitTest/AnalysisDiff.log
else
   echo "No previous log to diff" > $AGRELEASE/scripts/UnitTest/LeakDiff.log
   echo "No previous log to diff" > $AGRELEASE/scripts/UnitTest/AnalysisDiff.log
   echo "No previous log to diff" > $AGRELEASE/scripts/UnitTest/MacroDiff.log
fi
echo "done..."
echo "check:
  ${LEAKTEST}
  ${ALPHATEST}
  ${MACROTEST}
          "
