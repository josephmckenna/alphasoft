#!/bin/bash

if [ ${AGRELEASE} != "" ]; then
  cd ${AGRELEASE}
  if [ -d rootana ]; then
    rm -rf rootana_main
    mv -f rootana rootana_main
  fi
git clone https://jtkm@bitbucket.org/jtkm/multithread-manalyser.git rootana
#make clean
#make

else
echo "AGRELEASE not set"
fi
