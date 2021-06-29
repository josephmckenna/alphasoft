#!/bin/bash

ROOTFILE="/daq/alpha_data0/acapra/alphag/output5/output03873.root"

if [[ $# -eq 1 ]]; then
    ROOTFILE=$1
fi

echo "Reconstructing" ${ROOTFILE}

valgrind --leak-check=full --error-limit=no --suppressions=${ROOTSYS}/etc/valgrind-root.supp  --log-file="leak_check_agreco.log" ./agreco.exe -f ${ROOTFILE} --anasettings $AGRELEASE/ana/cosm.json -e 1000 | tee valagreco.log
