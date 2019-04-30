#!/bin/bash

if [ ${AGRELEASE} != "" ]; then
  cd ${AGRELEASE}
  if [ -d rootana ]; then
    mv -f rootana rootana_main
  fi
git clone https://jtkm@bitbucket.org/jtkm/multithread-manalyser.git rootana
#make clean
#make

else
echo "AGRELEASE not set"
fi
