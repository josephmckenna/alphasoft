#!/bin/sh

case `hostname` in
alphagdaq*)
    echo "Good, we are on alphagdaq!"
    ;;
*)
    echo "The kill_daq script should be executed on alphagdaq"
    exit 1
    ;;
esac


odbedit -c "sh all"
sleep 1

#end file
