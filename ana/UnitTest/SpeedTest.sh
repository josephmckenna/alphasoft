#!/bin/bash
RUNNO=$1
echo "Warning: I take a long time... like 10x normal speed..."

RUNNO=$1

if [ `echo "$AGANA" | wc -c` -gt 3 ]; then
  echo "AGANA set ok..."
else
  echo "AGANA envvar not set"
fi


if [ `echo "$RUNNO" | wc -c` -gt 3 ]; then
  echo "Using run $RUNNO"
else
  RUNNO=01259
  echo "Using default run $RUNNO"
fi

for i in `seq 1 1000`; do
  if [ -e LeakTest${i}.log ]; then
  EATCAKE=1
  else
    if [ -e StripTest${i}.log ]; then
    EATCAKE=2
    else
      if [ -e AnalysisTest${i}.log ]; then
      EATCAKE=3
      else
        if [ -e MacroTest${i}.log ]; then
          echo -n "."
        else
          SPEEDTEST="${AGANA}/UnitTest/SpeedTest${i}.out"
          ALPHATEST="${AGANA}/UnitTest/AnalysisTest${i}.log"
          MACROTEST="${AGANA}/UnitTest/MacroTest${i}.log"
          GITDIFF="${AGANA}/UnitTest/git_DiffTest${i}.log"
          TESTID=${i}
          break
        fi
      fi
    fi
  fi
done


cd $AGANA
git diff > ${GITDIFF}


echo "Running ..."

#Suppress false positives: https://root.cern.ch/how/how-suppress-understood-valgrind-false-positives
valgrind --tool=callgrind --callgrind-out-file="${SPEEDTEST}" ./agana.exe $MIDASDATA/run${RUNNO}sub000.mid.lz4 &> ${ALPHATEST}
 

echo "done..."
echo "check:
  kcachegrind ${SPEEDTEST}
  ${ALPHATEST}
  ${MACROTEST}
          "
#cd $RELEASE
