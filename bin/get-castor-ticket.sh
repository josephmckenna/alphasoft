#!/bin/sh

case `hostname` in
alphagdaq*)
    echo "Good, we are on ALPHAgdaq"
    ;;
*)
    echo "This script should be executed on ALPHAgdaq"
    exit 1
    ;;
esac
#kinit -k -t /etc/alphacdr.keytab alphacdr
kinit alphacdr@CERN.CH -k -t /etc/alphacdr.keytab
klist

#end file
