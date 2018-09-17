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
sleep 2
odbedit -c "sh all"
sleep 2

echo "the following list should be empty [except for odbedit]"
odbedit -c scl
#end file
