#!/bin/bash
echo "Warning: I take a long time... like 10x normal speed..."

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
  if [ -e SpeedTest${i}_${BRANCH}.log ]; then
  DUMMYVAR=1
  else
    if [ -e SpeedTest_git_diff_${i}_${BRANCH}.log ]; then
    DUMMYVAR=2
    else
      if [ -e SpeedTest_AnalysisOut_${i}_${BRANCH}.log ]; then
      DUMMYVAR=3
      else
        if [ -e SpeedTest_MacroOut_${i}_${BRANCH}.log ]; then
          echo -n "."
        else
          SPEEDTEST="$DIR/SpeedTest${i}_${BRANCH}.out"
          ALPHATEST="$DIR/SpeedTest_AnalysisOut_${i}_${BRANCH}.log"
          MACROTEST="$DIR/SpeedTest_MacroOut_${i}_${BRANCH}.log"
          GITDIFF="$DIR/SpeedTest_git_diff_${i}_${BRANCH}.log"
          BUILDLOG="$DIR/SpeedTest_Build_${i}_${BRANCH}.log"
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

echo $SPEEDTEST
cd $AGRELEASE/ana
echo "Running..."

echo "Running ..."

#Suppress false positives: https://root.cern.ch/how/how-suppress-understood-valgrind-false-positives
valgrind --tool=callgrind --callgrind-out-file="${SPEEDTEST}" ./agana.exe ${Event_Limit} run${RUNNO}sub000.mid.lz4 &> ${ALPHATEST}
 

echo "done..."
echo "check:
  kcachegrind ${SPEEDTEST}
  ${ALPHATEST}
  ${MACROTEST}
          "
#cd $RELEASE
