#!/bin/bash
set -e


if [ `echo "$AGRELEASE" | wc -c` -gt 3 ]; then
  echo "AGRELEASE set ok..."
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi



cd ${AGRELEASE}/ana
#Calling -h returns with a non-zero exit code
#./agana.exe -h
#Calling with a fake input file and --help finishes with a exit code 0 (not fail)
./agana.exe fakefile -- --help
echo "Add more here"


