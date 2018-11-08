#!/bin/bash -x

#Global vars:
MIDAS_PATH="/alpha/agdaq/data"

. agconfig.sh

RUNNO=$1
if [ ${RUNNO} -lt 10000 ]; then
      RUNNO_FILE="0${RUNNO}"
      echo "${RUNNO_FILE} reformatted"
else
    RUNNO_FILE="${RUNNO}"
fi

echo "agana starting..."
{ time ./agana.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub*.mid.lz4 -- --recoff ; } &> R${RUNNO}_recoff.log
echo "agana finished..."
tail -70 R${RUNNO}_recoff.log

#cd $AGRELEASE/ana
echo "void tempmacroR${RUNNO}() {" > tempmacroR${RUNNO}.C
echo "PrintSequenceQOD(${RUNNO});" >> tempmacroR${RUNNO}.C
echo "}" >> tempmacroR${RUNNO}.C
root -l -q -b tempmacroR${RUNNO}.C > Seq${RUNNO}.log

cat Seq${RUNNO}.log

echo "Enter Start Time:"
read start_dump

echo "Enter Stop Time:"
read stop_dump

echo "void tempmacroR${RUNNO}() {" > tempmacroR${RUNNO}.C
echo "cout<<\"TSTART \"<<GetTrigTimeBefore(${RUNNO},${start_dump})<<endl;" >> tempmacroR${RUNNO}.C
echo "}" >> tempmacroR${RUNNO}.C
start_time=$(root -l -q -b tempmacroR${RUNNO}.C | grep "TSTART " | awk '{print $2}')

echo "void tempmacroR${RUNNO}() {" > tempmacroR${RUNNO}.C
echo "cout<<\"TSTOP \"<<GetTrigTimeAfter(${RUNNO},${stop_dump})<<endl;" >> tempmacroR${RUNNO}.C
echo "}" >> tempmacroR${RUNNO}.C
stop_time=$(root -l -q -b tempmacroR${RUNNO}.C | grep "TSTOP " | awk '{print $2}')

mv output${RUNNO_FILE}.root output${RUNNO_FILE}_noreco.root
echo "agana starting..."
echo "./agana.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub*.mid.lz4 -- --usetimerange $start_time $stop_time"
#cd $AGRELEASE
{ time ./agana.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub*.mid.lz4 -- --usetimerange $start_time $stop_time ; } &> R${RUNNO}_timerange${start_time}-${stop_time}.log
echo "agana finished..."
tail -70 R${RUNNO}_timerange${start_time}-${stop_time}.log

#cd $AGRELEASE/ana
echo "void tempmacroR${RUNNO}() {" > tempmacroR${RUNNO}.C
echo "Plot_TPC(${RUNNO},${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C
echo "Plot_Chrono(${RUNNO},\"SiPM_A_AND_D\",${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C
echo "Plot_Chrono(${RUNNO},\"SiPM_C_AND_F\",${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C
echo "Plot_Chrono(${RUNNO},\"SiPM_B\",${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C
echo "Plot_Chrono(${RUNNO},\"SiPM_E\",${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C
echo "TSeqCollection* cc = gROOT->GetListOfCanvases();" >> tempmacroR${RUNNO}.C
echo "TString fold = MakeAutoPlotsFolder(\"\");" >> tempmacroR${RUNNO}.C
echo "for( int ic =0 ; ic <cc->GetEntries(); ++ic ) {" >> tempmacroR${RUNNO}.C
echo "TString sname = fold; sname+=\"_R${RUNNO}\"; sname+=((TCanvas*) cc->At(ic))->GetName();" >> tempmacroR${RUNNO}.C
echo "sname += \".pdf\";" >> tempmacroR${RUNNO}.C
echo "((TCanvas*) cc->At(ic))->SaveAs(sname.Data());" >> tempmacroR${RUNNO}.C
echo "}" >> tempmacroR${RUNNO}.C
echo "}" >> tempmacroR${RUNNO}.C

root -l tempmacroR${RUNNO}.C

rm -f tempmacroR${RUNNO}.C

#mv output${RUNNO_FILE}.root "output${RUNNO_FILE}_timerange${start_time}-${stop_time}.root"
