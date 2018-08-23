#!/bin/bash

if [ `echo "$AGRELEASE" | wc -c` -gt 3 ]; then
  echo "AGRELEASE set ok..."
else
  echo "AGRELEASE envvar not set... exiting"
  exit
fi



cd ${AGRELEASE}/ana
./agana.exe -h
./agana.exe fakefile -- --help
echo "Add more here"


