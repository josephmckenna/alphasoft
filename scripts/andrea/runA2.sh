#!/bin/bash

if [ ! -d "${AGRELEASE}/RunLogs" ]; then
    mkdir -p ${AGRELEASE}/RunLogs
fi

A2MIDASDATA="/eos/experiment/alpha/midasdata"
#A2OUTPUT="/afs/cern.ch/user/a/acapra/CERNbox/ALPHA-2_output"
#echo "Saving files to ${A2OUTPUT}"

RUN_LIST=(39993 45000 57181 57195 57208 57350 54845)

for RUN in ${RUN_LIST[*]};do
    printf "   Starting %d\n" $RUN

    #alphaStrips.exe --mt run${RUN}sub00000.mid.gz -- --EOS |& tee ${AGRELEASE}/RunLogs/alphaStripsRun${RUN}.log
    #alphaAnalysis.exe --mt run${RUN}sub00000.mid.gz -- --EOS |& tee ${AGRELEASE}/RunLogs/alphaAnalysisRun${RUN}.log
    
    alphaStrips.exe --mt $A2MIDASDATA/run${RUN}sub00000.mid.gz |& tee ${AGRELEASE}/RunLogs/alphaStripsRun${RUN}.log
    alphaAnalysis.exe --mt $A2MIDASDATA/run${RUN}sub*.mid.gz |& tee ${AGRELEASE}/RunLogs/alphaAnalysisRun${RUN}.log
    
    #gdb -ex=r --args alphaAnalysis.exe --mt run${RUN}sub00000.mid.gz -- --EOS
done
