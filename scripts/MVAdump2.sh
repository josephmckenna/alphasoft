#!/bin/bash


# You should probably hand craft a run list...
# but for now this tight range of runs includes both cosmic and mixing... 
FIRST_RUN=55100
LAST_RUN=55110

for i in `seq ${FIRST_RUN} ${LAST_RUN}`; do
   echo "Processing run ${i}"
   alphaStrips.exe /eos/experiment/alpha/midasdata/run${i}sub000* --mt &> S${i}.log
   alphaAnalysis.exe /eos/experiment/alpha/midasdata/run${i}sub000* --mt &> R${i}.log
done


rm Mixing.list

#
echo ".L alpha2/macros/DumpMixingData.C
for (int i = ${FIRST_RUN}; i < ${LAST_RUN}; i++) { DumpMixingData(i); }
.q
" | root -l -b

for i in `cat Mixing.list | awk -F: '{print $1}' | sort | uniq`; do
   echo "Dumping mixing data for run ${i}"
   a2dumper.exe /eos/experiment/alpha/midasdata/run${i}sub000* -- --eventlist Mixing.list --datalabel Mixing &> D${i}.Mixing.log
done

rm Cosmic.list

echo " .L alpha2/macros/DumpCosmicData.C
 for (int i = ${FIRST_RUN}; i < ${LAST_RUN}; i++) { DumpCosmicData(i); }
 .q
"  | root -l -b 

for i in `cat Cosmic.list | awk -F: '{print $1}' | sort | uniq`; do
   echo "Dumping cosmic data for run ${i}"
   a2dumper.exe /eos/experiment/alpha/midasdata/run${i}sub000* -- --eventlist Cosmic.list --datalabel Cosmic  &> D${i}.Cosmic.log
done

hadd merged_data.root dumperoutput55*

# Data collection complete... this output is basically compatible with
# the alphaClassification program from the old repo. It needs to 
# be migrated
