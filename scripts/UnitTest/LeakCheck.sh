#!/bin/bash

RUNNO=$1
DOBUILD=$2
LIMITEVENTS=$3
MODULEFLAGS=$4

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

if [ `echo "$MODULEFLAGS" | wc -c` -gt 3 ]; then
  MODULEFLAGS="-- ${MODULEFLAGS}"
  echo "Module flags: ${MODULEFLAGS}"
fi
BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `

cd ${DIR}
for i in `seq 1 100000`; do
   READYTOGO=1
  for logfile in LeakTest${i}_${BRANCH}.log \
                  LeakTest_git_diff_${i}_${BRANCH}.log \
                  LeakTest_AnalysisOut_${i}_${BRANCH}.log \
                  LeakTest_MacroOut_${i}_${BRANCH}.log \
                  LeakTest_Build_${i}_${BRANCH}.log; do
      if [ -e ${logfile} ]; then
         ls -lh ${logfile}
         READYTOGO=0
         break
      fi
   done
   if [ ${READYTOGO} -eq 1 ]; then
      LEAKTEST="$DIR/LeakTest${i}_${BRANCH}.log"
      ALPHATEST="$DIR/LeakTest_AnalysisOut_${i}_${BRANCH}.log"
      MACROTEST="$DIR/LeakTest_MacroOut_${i}_${BRANCH}.log"
      GITDIFF="$DIR/LeakTest_git_diff_${i}_${BRANCH}.log"
      BUILDLOG="$DIR/LeakTest_Build_${i}_${BRANCH}.log"
      TESTID=${i}
      break
   fi
done
if [ "$DOBUILD" != "NOBUILD" ]; then
  echo "Recompiling everything..."
  cd ${AGRELEASE}
  if [ "$DOBUILD" == "FASTBUILD" ]; then
    make clean && make -j &> ${BUILDLOG}
  else
    make clean && make &> ${BUILDLOG}
  fi
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
cd $AGRELEASE
ls -l -h *.exe
echo "Running..."
if [ -f ${ROOTSYS}/etc/valgrind-root.supp ]; then
SUPP="--suppressions=${ROOTSYS}/etc/valgrind-root.supp"
fi
set -x
#Suppress false positives: https://root.cern.ch/how/how-suppress-understood-valgrind-false-positives
valgrind --leak-check=full --error-limit=no ${SUPP} --log-file="${LEAKTEST}" ./agana.exe ${Event_Limit} -Orun${RUNNO}sub000leaktest.root ${AGMIDASDATA}/run${RUNNO}sub000.mid.lz4 ${MODULESFLAGS} &> ${ALPHATEST}
set +x

 
cat ${LEAKTEST} | cut -f2- -d' ' > ${LEAKTEST}.nopid

#echo ".L macros/ReadEventTree.C 
#ReadEventTree()
#.q
#" | root -l -b *${RUNNO}*.root &> ${MACROTEST}

root -q -b run${RUNNO}sub000leaktest.root ana/macros/ReadEventTree.C

cat ${LEAKTEST}.nopid | tail -n 16

if [ $TESTID -gt 1 ]; then
   BEFORE=`expr ${TESTID} - 1`
   echo diff -u "$DIR/LeakTest_AnalysisOut_${BEFORE}_${BRANCH}.log" "$DIR/LeakTest_AnalysisOut_${i}_${BRANCH}.log"
   diff -u "$DIR/LeakTest${BEFORE}_${BRANCH}.log.nopid" "$DIR/LeakTest${i}_${BRANCH}.log.nopid" > $AGRELEASE/scripts/UnitTest/LeakDiff.log
   diff -u "$DIR/LeakTest_AnalysisOut_${BEFORE}_${BRANCH}.log" "$DIR/LeakTest_AnalysisOut_${i}_${BRANCH}.log" > $AGRELEASE/scripts/UnitTest/AnalysisDiff.log
   diff -u "$DIR/LeakTest_MacroOut_${BEFORE}_${BRANCH}.log" "$DIR/LeakTest_MacroOut_${i}_${BRANCH}.log" > $AGRELEASE/scripts/UnitTest/MacroDiff.log
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
