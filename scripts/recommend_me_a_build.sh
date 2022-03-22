#!/bin/bash

# Usage:
# recommend_me_a_build.sh jagana.exe run06070sub000.mid.lz4


# It may take ~2 hours+ for this to finish running...
#                           ...
#      ...perhaps someone can program a minimising search rather than
# brute force... Id like to see it in  bash but maybe python is better

BIN=${1}
RUNS="${@:2}"

# Theoretical top speed:
/usr/bin/time -al lz4cat ${RUNS} > /dev/null 2> $AGRELEASE/scripts/MT_MODE_TEST_MAX_SPEED.log

#grep set_property ana/CMakeLists.txt
#set_property(CACHE N_PAD_DECONV_THREADS PROPERTY STRINGS "1" "2" "3" "4" "5" "8")
#set_property(CACHE N_KDTREE_MATCH_THREADS PROPERTY STRINGS "1" "2")
#set_property(CACHE N_TRACK_FIT_THREADS PROPERTY STRINGS "1" "2" "3" "4" "8")
#set_property(CACHE N_VERTEX_FIT_THREADS PROPERTY STRINGS "1" "2" "3")

N_PAD_DECONV_THREADS N_KDTREE_MATCH_THREADS N_TRACK_FIT_THREADS N_VERTEX_FIT_THREADS

for N_PAD_DECONV_THREADS in 1 2 3 4 5 8; do
    for N_KDTREE_MATCH_THREADS in 1 2; do
        for N_TRACK_FIT_THREADS in 1 2 3 4 8; do
            for N_VERTEX_FIT_THREADS in 1 2 3; do
                cd build
                echo "cmake ../ -DN_PAD_DECONV_THREADS=${N_PAD_DECONV_THREADS} -DN_KDTREE_MATCH_THREADS=${N_KDTREE_MATCH_THREADS} -DN_TRACK_FIT_THREADS=${N_TRACK_FIT_THREADS} -DN_VERTEX_FIT_THREADS=${N_VERTEX_FIT_THREADS}"
                cmake ../ -DN_PAD_DECONV_THREADS=${N_PAD_DECONV_THREADS} -DN_KDTREE_MATCH_THREADS=${N_KDTREE_MATCH_THREADS} -DN_TRACK_FIT_THREADS=${N_TRACK_FIT_THREADS} -DN_VERTEX_FIT_THREADS=${N_VERTEX_FIT_THREADS}
                make install
                OUTPUT=${N_PAD_DECONV_THREADS}_${N_KDTREE_MATCH_THREADS}_${N_TRACK_FIT_THREADS}_${N_VERTEX_FIT_THREADS}
                cd $AGRELEASE
                /usr/bin/time -al ${BIN} ${RUNS} --mt -- --anasettings ana/cern2021_2.json > $AGRELEASE/scripts/${BIN}_${OUTPUT}.log 2> $AGRELEASE/scripts/MT_MODE_TEST_${OUTPUT}.log
            done
        done
    done
done

echo "All results:"
grep real $AGRELEASE/scripts/MT_MODE_TEST_*.log 


echo "Best setting:"
grep `grep real $AGRELEASE/scripts/MT_MODE_TEST_*.log | awk ' { print $2 } ' | sort | head -n1` $AGRELEASE/scripts/MT_MODE_TEST_*.log