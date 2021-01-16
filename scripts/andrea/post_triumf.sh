#!/bin/bash


echo -e "This is pulser run 904559, with ~11k events.\nThe sleeve is off, the HV is on.\nI no longer see the noise previously detected." > msg.txt

MSG=$(readlink -f msg.txt)
ATT=""
N=0
for F in `ls /home/acapra/agsoft_percy/*.pdf`; do
    ATT=${ATT}"-f $F "
    N=$((N+1))
done
if [[ $N -eq 0 ]]; then
    exit 1
else
    echo "${ATT}"
fi
ssh daq.triumf.ca /home/alphag/packages/elog/elog -h localhost -p 9081 -l alphag -v -a Author="Andrea" a Type="Analysis" -a Category="TPC" -a Subject="AW\ noise\ investigation" -r 3573 -q -u acapra Hbar_2015 -m $MSG "${ATT}"
rm msg.txt
