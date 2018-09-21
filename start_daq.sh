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

#end file
