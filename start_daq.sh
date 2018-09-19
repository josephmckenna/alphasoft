#!/bin/sh

cd $HOME/online

case `hostname` in
alphagdaq*)
    echo "Good, we are on alphagdaq!"
    ;;
*)
    echo "The start_daq script should be executed on alphagdaq"
    exit 1
    ;;
esac

odbedit -c clean
#$HOME/packages/ser2net/ser2net -c $HOME/online/ser2net.conf -p 3000
sleep 1
mhttpd  -D
sleep 1
mserver -D
sleep 1
mlogger -D
sleep 1
/opt/nut/sbin/upsdrvctl start
sleep 1
/opt/nut/sbin/upsd
sleep 1
/opt/nut/bin/upsc ups
sleep 1

echo "start UPS monitor"
/home/agdaq/packages/frontends/fenutups/fenutups.exe -i 1 -D


echo "start HV controller"
/home/agdaq/online/src/fecaenr14xxet.exe hvps01 -D

echo "start GHS controller"
/home/agdaq/online/src/fegastelnet.exe -D

echo "start LV controller"
/home/agdaq/online/src/fewienerlvps.exe lvps01 -D

echo "start VME crate monitor"
/home/agdaq/online/src/fewienerlvps.exe vmeps01 -D

echo "start fectrl"
/home/agdaq/online/src/fectrl.exe -D

echo "start H2O cooling monitor"
/home/agdaq/online/src/femoxa.exe -D

echo "start UDP frontend"
/home/agdaq/online/src/fexudp.exe -D

echo "start Event Builder"
/home/agdaq/online/src/feevb.exe -D

#end file
