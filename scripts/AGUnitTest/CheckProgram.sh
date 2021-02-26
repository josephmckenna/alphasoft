#!/bin/bash
#Set default run number:
RUNNO=45000

echo "Usage:"
echo "./LeakCheckProg.sh -p agana.exe -r 45000 -b NOBUILD -m \"--argumentformodule a --otherarg b\""
echo "-r 12345      Set run number"
echo "-b [string]   Build option, valid strings: FASTBUILD NOBUILD "
echo "-n 6          FASTBUILD Threads"
echo "-t [string]   Test type, valid string LEAK SPEED THREAD"
while getopts p:r:b:l:m:n:t: option
do
case "${option}"
in
p) PROG=${OPTARG};;
r) RUNNO=${OPTARG};;
b) DOBUILD=${OPTARG};;
l) LIMITEVENTS=${OPTARG};;
m) MODULEFLAGS=${OPTARG};;
n) BUILD_THREADS=${OPTARG};;
t) TEST_TYPE=${OPTARG};;
esac
done
if [ -h ${PROG} ]; then
   echo "FATAL: You must give me a program to run"
   exit
fi
if [ -h ${TEST_TYPE} ]; then
   echo "FATAL: You must give a test type"
   exit
fi
if [ ${TEST_TYPE} == "LEAK" ] || [ ${TEST_TYPE} == "SPEED" ] || [ ${TEST_TYPE} == "THREAD" ]; then
   echo "Valid test type"
else
   echo "Invalid test type"
   exit
fi

echo "PROGRAM:      ${PROG}"
echo "RUNNO:        ${RUNNO}"
echo "DOBUILD:      ${DOBUILD}"
echo "LIMITEVENTS:  ${LIMITEVENTS}"
echo "MODULEFLAGS:  ${MODULEFLAGS}"

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

if [ ! -f ${AGMIDASDATA}/run${RUNNO}sub000.mid.lz4 ]; then
  eos cp /eos/experiment/ALPHAg/midasdata_old/run${RUNNO}sub000.mid.lz4 ${AGMIDASDATA}/
else
  echo "run${RUNNO}sub00000.mid.gz found locally"
fi


if [ `echo "$MODULEFLAGS" | wc -c` -gt 3 ]; then
  MODULEFLAGS="-- ${MODULEFLAGS}"
  echo "Module flags: ${MODULEFLAGS}"
fi
BRANCH=`git branch --remote --verbose --no-abbrev --contains | sed -rne 's/^[^\/]*\/([^\ ]+).*$/\1/p' | tail -n 1 |  grep -o "[a-zA-Z0-9]*" | tr -d "\n\r" `

cd ${DIR}
for i in `seq 1 100000`; do
   READYTOGO=1
  for logfile in ${PROG}_${TEST_TYPE}_${i}_${BRANCH}.log \
                  ${PROG}_${TEST_TYPE}_git_diff_${i}_${BRANCH}.log \
                  ${PROG}_${TEST_TYPE}_AnalysisOut_${i}_${BRANCH}.log \
                  ${PROG}_${TEST_TYPE}_MacroOut_${i}_${BRANCH}.log \
                  ${PROG}_${TEST_TYPE}_Build_${i}_${BRANCH}.log; do
      if [ -e ${logfile} ]; then
         ls -lh ${logfile}
         READYTOGO=0
         break
      fi
   done
   if [ ${READYTOGO} -eq 1 ]; then
      VALGRINDTEST="$DIR/${PROG}_${TEST_TYPE}_${i}_${BRANCH}.out"
      LAST_VALGRINDTEST="$DIR/${PROG}_${TEST_TYPE}_${i}_${BRANCH}.out"

      ALPHATEST="$DIR/${PROG}_${TEST_TYPE}_AnalysisOut_${i}_${BRANCH}.log"
      LAST_ALPHATEST="$DIR/${PROG}_${TEST_TYPE}_AnalysisOut_${i}_${BRANCH}.log"

      MACROTEST="$DIR/${PROG}_${TEST_TYPE}_MacroOut_${i}_${BRANCH}.log"
      LAST_MACROTEST="$DIR/${PROG}_${TEST_TYPE}_MacroOut_${i}_${BRANCH}.log"

      GITDIFF="$DIR/${PROG}_${TEST_TYPE}_git_diff_${i}_${BRANCH}.log"
      LAST_GITDIFF="$DIR/${PROG}_${TEST_TYPE}_git_diff_${i}_${BRANCH}.log"

      BUILDLOG="$DIR/${PROG}_${TEST_TYPE}_Build_${i}_${BRANCH}.log"
      LAST_BUILDLOG="$DIR/${PROG}_${TEST_TYPE}_Build_${i}_${BRANCH}.log"
      TESTID=${i}
      break
   fi
done
if [ "$DOBUILD" != "NOBUILD" ]; then
  echo "Recompiling everything..."
  cd ${AGRELEASE}
  if [ "$DOBUILD" == "FASTBUILD" ]; then
    rm -rf bin build
    mkdir -p build  &> ${BUILDLOG}
    cd build  &> ${BUILDLOG}
    cmake ../ -DBUILD_AG=ON  &> ${BUILDLOG}
    make -j${BUILD_THREADS}  &>> ${BUILDLOG}
    make install  &>> ${BUILDLOG}
  else
    rm -rf bin build
    mkdir -p build  &> ${BUILDLOG}
    cd build  &> ${BUILDLOG}
    cmake ../ -DBUILD_AG=ON  &> ${BUILDLOG}
    make  &>> ${BUILDLOG}
    make install  &>> ${BUILDLOG}
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

echo $VALGRINDTEST
ls -l -h $AGRELEASE/bin/*.exe
echo "Running..."
if [ -f ${ROOTSYS}/etc/valgrind-root.supp ]; then
SUPP="--suppressions=${ROOTSYS}/etc/valgrind-root.supp"
fi


if [ ${TEST_TYPE} == "LEAK" ]; then
   #Suppress false positives: https://root.cern.ch/how/how-suppress-understood-valgrind-false-positives
   set -x
   valgrind --leak-check=full --error-limit=no ${SUPP} --log-file="${VALGRINDTEST}" ${PROG} ${Event_Limit} ${AGRELEASE}/run${RUNNO}sub000.mid.lz4 ${MODULESFLAGS} &> ${ALPHATEST}
elif [ ${TEST_TYPE} == "SPEED" ]; then
   #Suppress false positives: https://root.cern.ch/how/how-suppress-understood-valgrind-false-positives
   set -x
   valgrind --tool=callgrind --callgrind-out-file="${VALGRINDTEST}" ${PROG} ${Event_Limit} ${AGRELEASE}/run${RUNNO}sub000.mid.lz4 &> ${ALPHATEST}
elif [ ${TEST_TYPE} == "THREAD" ]; then
   #Suppress false positives: https://root.cern.ch/how/how-suppress-understood-valgrind-false-positives
   set -x
   valgrind -v --tool=helgrind --error-limit=no  --log-file="${VALGRINDTEST}" ${PROG} --mt ${Event_Limit} ${AGRELEASE}/run${RUNNO}sub000.mid.lz4 ${MODULESFLAGS} &> ${ALPHATEST}
else
   echo "FATAL Test type not understood"
fi


cd $AGRELEASE
set +x

 
cat ${VALGRINDTEST} | cut -f2- -d' ' > ${VALGRINDTEST}.nopid

#echo ".L macros/ReadEventTree.C 
#ReadEventTree()
#.q
#" | root -l -b *${RUNNO}*.root &> ${MACROTEST}

cat ${VALGRINDTEST}.nopid | tail -n 16

if [ -f ${VALGRINDTEST} ] && [ -f ${LAST_VALGRINDTEST} ]; then
   diff -u ${VALGRINDTEST} ${LAST_VALGRINDTEST} > $AGRELEASE/scripts/AGUnitTest/LeakDiff.log
else
   echo "No previous log to diff" > $AGRELEASE/scripts/AGUnitTest/LeakDiff.log
fi

if [ -f ${ALPHATEST} ] && [ -f ${LAST_ALPHATEST} ]; then
   diff -u ${ALPHATEST} ${LAST_ALPHATEST} > $AGRELEASE/scripts/AGUnitTest/AnalysisDiff.log
else
   echo "No previous log to diff" > $AGRELEASE/scripts/AGUnitTest/AnalysisDiff.log
fi

#   diff -u "$DIR/${PROG}_${TEST_TYPE}_MacroOut_${BEFORE}_${BRANCH}.log" "$DIR/${PROG}_${TEST_TYPE}_MacroOut_${i}_${BRANCH}.log" > $AGRELEASE/scripts/AGUnitTest/MacroDiff.log

echo "done..."
echo "check:
  ${VALGRINDTEST}
  ${ALPHATEST}
  ${MACROTEST}
          "
          
#cd $RELEASE
