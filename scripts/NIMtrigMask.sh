#!/bin/bash

#echo "obase=16; $((0x2 << (2*0)))" | bc
link=$1
if [[ "${link}" == "" ]]; then
    echo "provide link index [0..16]"
    exit 123
else
    echo -n "NIM trig mask 0x"
    echo "obase=16; $((0x2 << (2*$link)))" | bc
fi
