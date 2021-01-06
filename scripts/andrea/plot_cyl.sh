#!/bin/bash

#run_list=(903963 903965 903967 903990 903991 904014)
run_list=(903963 903965 903967 903990 903991)
comments=("T09up" "T13up" "T14up w/Supp" "T10up w/Supp" "T10up w/Supp" "all PWBs")
cmd="hadd -ff $DATADIR/agmini/cosmics903916.root"

for idx in ${!run_list[*]}; do
    run=${run_list[$idx]}
    printf "Plotting Run   %s\t%s\n" ${run} "${comments[$idx]}"
    #time $AGRELEASE/build/reco/MainEventTree.exe -f $DATADIR/agmini/cosmics${run}.root -p 1 -s 0
    cmd="${cmd} $DATADIR/agmini/cosmics${run}.root"
done
echo $cmd

time $AGRELEASE/build/reco/MainEventTree.exe -f $DATADIR/agmini/cosmics904014.root -p 1 &
time $AGRELEASE/build/reco/MainEventTree.exe -f $DATADIR/agmini/cosmics903916.root -p 1 &
