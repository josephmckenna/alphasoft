#!/bin/bash

if [ ! -d "RunLogs" ]; then
    mkdir -p RunLogs
fi


#FILE_LIST=($(ls -rt $DATADIR/test/*.root))
#FILE_LIST=("$DATADIR/test/cosmics904502.root" "$DATADIR/test/cosmics904512.root")
#FILE_LIST=("$DATADIR/test/cosmics904565.root" "$DATADIR/test/cosmics904554.root" "$DATADIR/test/cosmics904562.root" "$DATADIR/test/cosmics904555.root" "$DATADIR/test/cosmics904547.root" "$DATADIR/test/cosmics904549.root")


echo "list of file:" ${FILE_LIST[*]}
for RF in ${FILE_LIST[*]}; do
    ls -lh $RF
    runno=$(basename $RF | grep -o '[0-9]\{4,\}')
    echo "Run Number:" ${runno}
    set -x
    MainEventTree.exe -f $RF -p 0 -s 1 &> RunLogs/Reco${runno}.log&
    #MainEventTree.exe -f $RF -p 1 -s 0
    set +x
done
