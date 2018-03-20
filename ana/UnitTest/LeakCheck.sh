#!/bin/bash

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
          LEAKTEST="${AGANA}/UnitTest/LeakTest${i}.log"
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
echo "AGANA:"
git diff > ${GITDIFF}
echo "aged:"
cd ../aged
git diff  >>${GITDIFF}
echo "#Analyis:" >> ${GITDIFF}
cd ../../analysis/
svn diff  >> ${GITDIFF}
cd $AGANA
echo "Running..."

#Suppress false positives: https://root.cern.ch/how/how-suppress-understood-valgrind-false-positives
valgrind --leak-check=full --error-limit=no --suppressions=${ROOTSYS}/etc/valgrind-root.supp  --log-file="${LEAKTEST}" ./agana.exe $MIDASDATA/run${RUNNO}sub000.mid.lz4 &> ${ALPHATEST}


 
cat ${LEAKTEST} | cut -f2- -d' ' > ${LEAKTEST}.nopid

cd $AGANA/macros
echo ".L RunSummary.C
RunSummary(\"../output$RUNNO.root\")
.q
"| root -l &> ${MACROTEST}

cat ${LEAKTEST}.nopid | tail -n 16

echo "done..."
echo "check:
  ${LEAKTEST}
  ${ALPHATEST}
  ${MACROTEST}
          "
#cd $RELEASE

