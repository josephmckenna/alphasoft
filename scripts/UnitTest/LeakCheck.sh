#!/bin/bash

RUNNO=$1
DOBUILD=$2
LIMITEVENTS=$3

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




if [ `echo "$RUNNO" | wc -c` -gt 3 ]; then
  echo "Using run $RUNNO"
else
  RUNNO=02364
  echo "Using default run $RUNNO"
  DOBUILD="BUILD"
fi

BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `

cd ${DIR}
for i in `seq 1 100000`; do
  if [ -e LeakTest${i}_${BRANCH}.log ]; then
  DUMMYVAR=1
  else
    if [ -e git_DiffTest${i}_${BRANCH}.log ]; then
    DUMMYVAR=2
    else
      if [ -e AnalysisTest${i}_${BRANCH}.log ]; then
      DUMMYVAR=3
      else
        if [ -e MacroTest${i}_${BRANCH}.log ]; then
          echo -n "."
        else
          LEAKTEST="$DIR/LeakTest${i}_${BRANCH}.log"
          ALPHATEST="$DIR/AnalysisTest${i}_${BRANCH}.log"
          MACROTEST="$DIR/MacroTest${i}_${BRANCH}.log"
          GITDIFF="$DIR/git_DiffTest${i}_${BRANCH}.log"
          BUILDLOG="$DIR/make${i}_${BRANCH}.log"
          TESTID=${i}
          break
        fi
      fi
    fi
  fi
done
if [ "$DOBUILD" != "NOBUILD" ]; then
  echo "Recompiling everything..."
  cd ${AGRELEASE}/ana
  make clean && make &> ${BUILDLOG}
  echo "Recompilation done: chech ${BUILDLOG}"
  WARNING_COUNT=`grep -i warning ${BUILDLOG} | wc -l`
  ERROR_COUNT=`grep -i error ${BUILDLOG} | wc -l`
  echo "Found ${WARNING_COUNT} warning(s) and ${ERROR_COUNT} errors(s) "
fi
if [ `echo "$LIMITEVENTS" | wc -c` -gt 1 ]; then
  export Event_Limit=" -e$LIMITEVENTS "
fi
cd $AGRELEASE
git diff > ${GITDIFF}

echo $LEAKTEST
cd $AGRELEASE/ana
ls -l -h *.exe
echo "Running..."

#Suppress false positives: https://root.cern.ch/how/how-suppress-understood-valgrind-false-positives
valgrind --leak-check=full --error-limit=no --suppressions=${ROOTSYS}/etc/valgrind-root.supp  --log-file="${LEAKTEST}" ./agana.exe ${Event_Limit} run${RUNNO}sub000.mid.lz4 &> ${ALPHATEST}


 
cat ${LEAKTEST} | cut -f2- -d' ' > ${LEAKTEST}.nopid

#cd $AGRELEASE/ana/macros
#echo ".L RunSummary.C
#RunSummary(\"../output$RUNNO.root\")
#.q
#"| root -l -b &> ${MACROTEST}

cat ${LEAKTEST}.nopid | tail -n 16

if [ $TESTID -gt 1 ]; then
   BEFORE=`expr ${TESTID} - 1`
   echo diff "$DIR/AnalysisTest${i}_${BRANCH}.log" "$DIR/AnalysisTest${BEFORE}_${BRANCH}.log"
   diff "$DIR/LeakTest${i}_${BRANCH}.log.nopid" "$DIR/LeakTest${BEFORE}_${BRANCH}.log.nopid" > $AGRELEASE/scripts/UnitTest/LeakDiff.log
   diff "$DIR/AnalysisTest${i}_${BRANCH}.log" "$DIR/AnalysisTest${BEFORE}_${BRANCH}.log" > $AGRELEASE/scripts/UnitTest/AnalysisDiff.log
   diff "$DIR/MacroTest${i}_${BRANCH}.log" "$DIR/MacroTest${BEFORE}_${BRANCH}.log" > $AGRELEASE/scripts/UnitTest/MacroDiff.log
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
          
#cd $RELEASE
