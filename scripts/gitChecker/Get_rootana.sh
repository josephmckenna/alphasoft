#!/bin/bash

if [ -d rootana ] && [ `ls -l rootana | wc -l` -gt 1 ]; then
  echo "rootana fould"
  cd rootana
  UPSTREAM=${1:-'@{u}'}
  LOCAL=$(git rev-parse @{0})
  REMOTE=$(git rev-parse "$UPSTREAM")
  BASE=$(git merge-base @{0} "$UPSTREAM")

  echo "Checking rootana status"
  if [ $LOCAL = $REMOTE ]; then
    echo "Up-to-date"
  elif [ $LOCAL = $BASE ]; then
    echo "Need to pull"
    git pull
    make clean && make
  elif [ $REMOTE = $BASE ]; then
    echo "Need to push"
  else
    echo "Diverged"
  fi
  export ROOTANASYS=`pwd`
  cd -
else
  git clone https://jtkm@bitbucket.org/tmidas/rootana.git
  cd rootana
  make
  export ROOTANASYS=`pwd`
  cd -
fi
