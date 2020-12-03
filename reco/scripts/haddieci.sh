#!/bin/bash

dieci()
{
    RUNNO=$1
    FILELIST=""
    PATHTO="./deconv_1-10subs"
    for SEED in 10081985 18061985 3092016 26092019 28091956 18031956 20051985 23061990 17122013 30112015; do
	FILELIST="${PATHTO}/anaoutAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30_multirun${RUNNO}_seed${SEED}.root ${FILELIST}"
	RUNNO=$((RUNNO+1))    
    done
    ls -lh ${FILELIST}
    hadd -ff ../macros/anaoutAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30_${tag}.root ${FILELIST}
}

tag='Xdeconv'
dieci 1

#tag='XX'
#dieci 11
#
#tag='XXX'
#dieci 21
#
#tag='XL'
#dieci 31
#
#tag='L'
#dieci 41
