#!/bin/bash

if [ ! -d "$AGRELEASE/RunLogs" ]; then
    mkdir -p $AGRELEASE/RunLogs
fi



#g4ana.exe -f $MCDATA/outAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30_cosmics.root -a $AGRELEASE/ana/oldsim.hjson &> $AGRELEASE/RunLogs/g4ana_agg4_cosmics_B1T.log&

#g4ana.exe -f $MCDATA/outAgTPC_det_AWtime16ns_PADtime16ns_B0.00T_Q30_cosmics.root -a $AGRELEASE/ana/oldsim.hjson --Bfield 0 &> $AGRELEASE/RunLogs/g4ana_agg4_cosmics_B0T.log&

FNAME=""

RUNNO=1
for SEED in 10081985 18061985 3092016 26092019 28091956 18031956 20051985 23061990 17122013 30112015; do

#    g4ana.exe -f $MCDATA/outAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30_cosmrun${RUNNO}_seed${SEED}.root -a $AGRELEASE/ana/oldsim.hjson &> $AGRELEASE/RunLogs/g4ana_agg4_cosmics${RUNNO}_B1T.log&

    ls -lh $MCDATA/outAgTPC_det_AWtime16ns_PADtime16ns_B0.00T_Q30_cosmrun${RUNNO}_seed${SEED}.root
    NN=$((RUNNO+10))
    ls -lh $MCDATA/outAgTPC_det_AWtime16ns_PADtime16ns_B0.00T_Q30_cosmrun${NN}_seed${SEED}.root

    g4ana.exe -f $MCDATA/outAgTPC_det_AWtime16ns_PADtime16ns_B0.00T_Q30_cosmrun${RUNNO}_seed${SEED}.root -a $AGRELEASE/ana/oldsim.hjson &> $AGRELEASE/RunLogs/g4ana_agg4_cosmics${RUNNO}_B0T.log&

    g4ana.exe -f $MCDATA/outAgTPC_det_AWtime16ns_PADtime16ns_B0.00T_Q30_cosmrun${NN}_seed${SEED}.root -a $AGRELEASE/ana/oldsim.hjson &> $AGRELEASE/RunLogs/g4ana_agg4_cosmics${NN}_B0T.log&

    FNAME="anaoutAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30_cosmrun${RUNNO}_seed${SEED}.root "${FNAME}

    RUNNO=$((RUNNO+1))
done

#echo $FNAME
echo "hadd -ff anaoutAgTPC_det_AWtime16ns_PADtime16ns_B0.00T_Q30_cosmmultirun.root $FNAME"
