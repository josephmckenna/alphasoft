#!/bin/bash

if [ -z "$MIDASSYS" ]; then
    echo "MIDAS needs to be installed and available through MIDASSYS"
else
    agana.exe -R8088 -Hlocalhost -- --aged
    #agana.exe -R8088 -Hlocalhost -- --wfexport
fi
