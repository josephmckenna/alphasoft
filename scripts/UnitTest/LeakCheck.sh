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



cd ${DIR}
for i in `seq 1 100000`; do
  if [ -e LeakTest${i}.log ]; then
  DUMMYVAR=1
  else
    if [ -e git_DiffTest${i}.log ]; then
    DUMMYVAR=2
    else
      if [ -e AnalysisTest${i}.log ]; then
      DUMMYVAR=3
      else
        if [ -e MacroTest${i}.log ]; then
          echo -n "."
        else
          LEAKTEST="$DIR/LeakTest${i}.log"
          ALPHATEST="$DIR/AnalysisTest${i}.log"
          MACROTEST="$DIR/MacroTest${i}.log"
          GITDIFF="$DIR/git_DiffTest${i}.log"
          BUILDLOG="$DIR/make${i}.log"
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
   echo diff "$DIR/AnalysisTest${i}.log" "$DIR/AnalysisTest${BEFORE}.log"
   diff "$DIR/LeakTest${i}.log.nopid" "$DIR/LeakTest${BEFORE}.log.nopid" > $AGRELEASE/scripts/UnitTest/LeakDiff.log
   diff "$DIR/AnalysisTest${i}.log" "$DIR/AnalysisTest${BEFORE}.log" > $AGRELEASE/scripts/UnitTest/AnalysisDiff.log
   diff "$DIR/MacroTest${i}.log" "$DIR/MacroTest${BEFORE}.log" > $AGRELEASE/scripts/UnitTest/MacroDiff.log
fi
echo "done..."
echo "check:
  ${LEAKTEST}
  ${ALPHATEST}
  ${MACROTEST}
          "
          
#cd $RELEASE