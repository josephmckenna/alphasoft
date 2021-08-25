#!/bin/bash

#
#baranalysis.exe p &
#baranalysis.exe c &
#
#baranalysis.exe c 0.1 > /dev/null &
#baranalysis.exe c 0.2 > /dev/null &
#baranalysis.exe c 0.3 > /dev/null &
#baranalysis.exe c 0.4 > /dev/null &
#baranalysis.exe c 0.5 > /dev/null &
#baranalysis.exe c 0.6 > /dev/null &
#baranalysis.exe c 0.7 > /dev/null &
#baranalysis.exe c 0.8 > /dev/null &
#
#baranalysis.exe p 0.1 > /dev/null &
#baranalysis.exe p 0.2 > /dev/null &
#baranalysis.exe p 0.3 > /dev/null &
#baranalysis.exe p 0.4 > /dev/null &
#baranalysis.exe p 0.5 > /dev/null &
#baranalysis.exe p 0.6 > /dev/null &
#baranalysis.exe p 0.7 > /dev/null &
#baranalysis.exe p 0.8 > /dev/null &
#


## time resolution loop
#for TS in $(seq 0 100 800); do
#    baranalysis.exe c $TS > /dev/null &
#    baranalysis.exe p $TS > /dev/null &
#done
##

## time resolution loop
#for TS in $(seq 0 100 800); do
## energy cut loop
#    for ECUT in $(seq 100 200 1000); do
#	baranalysis.exe c $TS $ECUT > /dev/null &
#	baranalysis.exe p $TS $ECUT > /dev/null &
#    done
#done
##

## time resolution loop
#for TS in $(seq 100 100 600); do
## energy cut loop
#    for ECUT in $(seq 300 300 1000); do
##	for ACUT in $(seq 5.625 5.625 45.0); do
#	for ACUT in $(seq 0 22.5 45.0); do
#	    echo "Time Resolution: " $TS " ps   Energy cut: " $ECUT " keV   Angle Cut: " $ACUT " deg"
#	    baranalysis.exe c $TS $ECUT $ACUT > /dev/null
#	    baranalysis.exe p $TS $ECUT $ACUT > /dev/null 
#	done
#    done
#done
#

## time resolution loop
#for TS in 0 200 300 400 600 800; do
#    echo "Time Resolution: " $TS " ps   Energy cut: " $ECUT " keV   Angle Cut: " $ACUT " deg"
#    baranalysis.exe c $TS 300 5.625 &> logs/cosmanalysis_${TS}ps_300keV_1bar.log &
#    baranalysis.exe p $TS 300 5.625 &> logs/pbaranalysis_${TS}ps_300keV_1bar.log &
#    gedit logs/cosmanalysis_${TS}ps_300keV_1bar.log logs/pbaranalysis_${TS}ps_300keV_1bar.log &
#done
##


# energy cut loop
#for ECUT in $(seq 100 200 1000); do
#    echo $ECUT
#    root -l -q -b baranalysis.C\(${ECUT}\) > logs/baranalysis_${ECUT}keV.log &
#    gedit logs/baranalysis_${ECUT}keV.log &
#done

for ECUT in $(seq 300 300 1000); do
#    for ACUT in $(seq 5.625 5.625 45); do
    for ACUT in $(seq 0 22.5 45.0); do
	echo "Energy cut: " $ECUT " keV   Angle Cut: " $ACUT " deg"
	root -l -q -b baranalysis.C\(${ACUT}\,${ECUT}\) > logs/baranalysis_${ACUT}deg_${ECUT}keV.log &
	    gedit logs/baranalysis_${ACUT}deg_${ECUT}keV.log &
	    #	root -l -q -b baranalysis.C\(${ACUT}\,${ECUT}\)
    done
done
