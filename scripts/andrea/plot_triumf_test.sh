#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi


#FILE_LIST=($(ls -rt $DATADIR/test/*.root))
#FILE_LIST=("$DATADIR/agmini/cosmics904690.root" "$DATADIR/agmini/cosmics904648sub000.root")

FILE_LIST=("/daq/alpha_data0/acapra/alphag/test/cosmics904472.root" "/daq/alpha_data0/acapra/alphag/test/cosmics904554.root" "/daq/alpha_data0/acapra/alphag/test/cosmics904555.root" "/daq/alpha_data0/acapra/alphag/test/cosmics904648.root" "/daq/alpha_data0/acapra/alphag/test/cosmics904474.root" "/daq/alpha_data0/acapra/alphag/test/cosmics904685.root" "/daq/alpha_data0/acapra/alphag/test/cosmics904501.root" "/daq/alpha_data0/acapra/alphag/test/cosmics904547.root" "/daq/alpha_data0/acapra/alphag/test/cosmics904503.root")


echo "list of file:" ${FILE_LIST[*]}
for RF in ${FILE_LIST[*]}; do
    ls -lh $RF
    runno=$(basename $RF | grep -o '[0-9]\{4,\}')
    echo "Run Number:" ${runno}
    set -x
    MainEventTree.exe -f $RF -p 0 -s 1 &> RunLogs/Reco${runno}.log&
    #MainEventTree.exe -f $RF -p 1 -s 0
    #MainEventTree.exe -f $RF -p 1 -s 1
    set +x
done

