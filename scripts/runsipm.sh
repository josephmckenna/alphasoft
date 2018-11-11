#!/bin/bash -x

#Global vars:
MIDAS_PATH="/alpha/agdaq/data"

RUNNO=$1
if [ ${RUNNO} -lt 10000 ]; then
      RUNNO_FILE="0${RUNNO}"
      echo "${RUNNO_FILE} reformatted"
else
    RUNNO_FILE="${RUNNO}"
fi

echo "agana starting..."
{ time ./agana.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub*.mid.lz4 -- --nounpack ; } &> R${RUNNO}_nounpack.log
echo "agana finished..."
tail -70 R${RUNNO}_nounpack.log

echo "void tempmacroR${RUNNO}() {" > tempmacroR${RUNNO}.C
echo "PrintSequenceQOD(${RUNNO});" >> tempmacroR${RUNNO}.C
echo "}" >> tempmacroR${RUNNO}.C
root -l -q -b tempmacroR${RUNNO}.C > Seq${RUNNO}.log

xmessage "Run ${RUNNO} is ready" -center -timeout 2

cat Seq${RUNNO}.log

echo "Enter Start Time:"
read start_dump

echo "Enter Stop Time:"
read stop_dump

echo "void tempmacroR${RUNNO}() {" > tempmacroR${RUNNO}.C
#echo "SetBinNumber(1000);" >> tempmacroR${RUNNO}.C
#echo "Plot_Chrono(${RUNNO},\"SiPM_A_AND_D\",${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C
#echo "Plot_Chrono(${RUNNO},\"SiPM_C_AND_F\",${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C
#echo "Plot_Chrono(${RUNNO},\"SiPM_B\",${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C
#echo "Plot_Chrono(${RUNNO},\"SiPM_E\",${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C
#echo "Plot_Chrono(${RUNNO},\"SiPM_A\",${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C
#echo "Plot_Chrono(${RUNNO},\"SiPM_F\",${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C
#echo "Plot_Chrono(${RUNNO},\"SiPM_C\",${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C

#echo "Plot_Chrono(${RUNNO},\"SiPM_A_AND_D\",\"Hold Dump\");" >> tempmacroR${RUNNO}.C
#echo "Plot_Chrono(${RUNNO},\"SiPM_C_AND_F\",\"Hold Dump\");" >> tempmacroR${RUNNO}.C
#echo "Plot_Chrono(${RUNNO},\"SiPM_B\",\"Hold Dump\");" >> tempmacroR${RUNNO}.C
#echo "Plot_Chrono(${RUNNO},\"SiPM_E\",\"Hold Dump\");" >> tempmacroR${RUNNO}.C
#echo "Plot_Chrono(${RUNNO},\"SiPM_C\",\"Hold Dump\");" >> tempmacroR${RUNNO}.C
#echo "Plot_Chrono(${RUNNO},\"SiPM_F\",\"Hold Dump\");" >> tempmacroR${RUNNO}.C

echo "PlotScintillators(${RUNNO},${start_dump},${stop_dump});" >> tempmacroR${RUNNO}.C

echo "TSeqCollection* cc = gROOT->GetListOfCanvases();" >> tempmacroR${RUNNO}.C
echo "TString fold = MakeAutoPlotsFolder(\"\");" >> tempmacroR${RUNNO}.C
echo "for( int ic =0 ; ic <cc->GetEntries(); ++ic ) {" >> tempmacroR${RUNNO}.C
echo "TString sname = fold; sname+=\"_R${RUNNO}\"; sname+=((TCanvas*) cc->At(ic))->GetName();" >> tempmacroR${RUNNO}.C
echo "sname += \".pdf\";" >> tempmacroR${RUNNO}.C
echo "((TCanvas*) cc->At(ic))->SaveAs(sname.Data());" >> tempmacroR${RUNNO}.C
echo "}" >> tempmacroR${RUNNO}.C
echo "}" >> tempmacroR${RUNNO}.C

root -l tempmacroR${RUNNO}.C
