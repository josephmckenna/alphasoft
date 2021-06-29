#!/bin/bash


echo -e "All plots.\n" > msg.txt

MSG=$(readlink -f msg.txt)
ATT=""
N=0
for F in `ls /home/acapra/agsoft/AutoPlots/R904685/*.pdf`; do
    ATT=${ATT}"-f $F "
    N=$((N+1))
done
if [[ $N -eq 0 ]]; then
    exit 1
else
    echo "${ATT}"
fi
ssh daq.triumf.ca /home/alphag/packages/elog/elog -h localhost -p 9081 -l alphag -v -a Author="Andrea" -a Type="Analysis" -a Category="TPC" -a Subject="Cosmic\ run\ 905685\ analysis" -r 3662 -q -u acapra Hbar_2015 -m $MSG "${ATT}"
rm msg.txt
