#!/bin/bash

if [ -d rootana ] || [ `ls -l rootana | wc -l` -gt 1 ]; then
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

  #Check root is sourced... 
  ROOT_INC_DIR=`root-config --incdir`
  if [ `echo ${ROOT_INC_DIR} | wc -c` -lt 10 ]; then
    echo "root misconfigured... source thisroot.sh somewhere!"
    return
  fi

  #Check rootana is build with current version of root
  ROOT_MATCHES_ROOTANA=`grep "${ROOT_INC_DIR}" include/*.txt`
  if [ `echo ${ROOT_MATCHES_ROOTANA} | wc -c` -lt 10 ]; then
    echo "rootana doesn't match root version... rebuilding..."
    make clean
    make
  fi

  #Catch incomplete builds and rebuild
  if [ `cat include/*.txt | wc -c` -lt 20 ]; then
    echo "rootana seems badly build:"
    cat include/*.txt
    echo "rebuilding..."
    make clean
    make
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
