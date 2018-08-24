#!/bin/bash
set -e


if [ `echo "${AGRELEASE}" | wc -c` -gt 3 ]; then
  echo "AGRELEASE set ok: $AGRELEASE"
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi

mkdir test-results

cd $AGRELEASE/ana
export EOS_MGM_URL=root://eospublic.cern.ch

eos cp /eos/experiment/alpha/run02364sub000.mid.lz4 .


#Calling -h returns with a non-zero exit code
#./agana.exe -h
#Calling with a fake input file and --help finishes with a exit code 0 (not fail)
./agana.exe run02364sub000.mid.lz4 -- --usetimerange 0 2 &> $AGRELEASE/test-results
tail -n 50 $AGRELEASE/test-results
#./agana.exe fakefile -- --help
echo "Add more here"


