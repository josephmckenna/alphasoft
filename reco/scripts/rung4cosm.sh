#!/bin/bash

if [ ! -d "$AGRELEASE/RunLogs" ]; then
    mkdir -p $AGRELEASE/RunLogs
fi



g4ana.exe -f $MCDATA/outAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30_cosmics.root -a $AGRELEASE/ana/oldsim.hjson &> $AGRELEASE/RunLogs/g4ana_agg4_cosmics_B1T.log&

g4ana.exe -f $MCDATA/outAgTPC_det_AWtime16ns_PADtime16ns_B0.00T_Q30_cosmics.root -a $AGRELEASE/ana/oldsim.hjson --Bfield 0 &> $AGRELEASE/RunLogs/g4ana_agg4_cosmics_B0T.log&
