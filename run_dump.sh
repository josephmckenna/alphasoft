#!/bin/bash -x
. agconfig.sh

#Global vars:
MIDAS_PATH="midasdata"

RUNNO=$1
if [ ${#RUNNO} -lt 3 ]; then
   echo "No run number given!"
   exit
fi
if [ ${RUNNO} -lt 10000 ]; then
      RUNNO_FILE="0${RUNNO}"
      echo "${RUNNO_FILE} reformatted"
else
    RUNNO_FILE="${RUNNO}"
fi

echo "This analysis will run reconstruction for a single dump... please give me a dump name:"
echo "eg \"Hold Dump\""
read DUMP_NAME

echo "Running alphagonline to get dump timings..."

echo "alphagonline.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub0*.mid.lz4"
alphagonline.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub0*.mid.lz4

sleep 1

FUDGE_TIME=2

echo "
std::vector<TAGSpill> dump=Get_AG_Spills(${RUNNO}, {\"${DUMP_NAME}\"}, {0});

if (dump.size() != 1) { for (int i =0; i< 15; i++) std::cerr<<\"BAD DUMP NAME!!! Please give full dump name, given dump name not found!\" <<std::endl; }

double tmin = dump.front().GetStartTime();
double tmax = dump.front().GetStopTime();
std::cout<< \"START TIME: \" << tmin - ${FUDGE_TIME} << std::endl;
std::cout<< \"STOP TIME: \" << tmax + ${FUDGE_TIME}<< std::endl;

" | root -l -b > time_window_${RUNNO}.data

export START_TIME=`grep START time_window_${RUNNO}.data| awk '{ print $3}'`
export STOP_TIME=`grep STOP time_window_${RUNNO}.data | awk '{ print $3}'`

agana.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub*.mid.lz4 -- --anasettings ana/cern2021_2.json --usetimerange ${START_TIME} ${STOP_TIME} &> R${RUNNO}_"${DUMP_NAME}".log

#Advanced plotting style (the Plot_TPC behind the scenes)
#echo "
#  //TAGPlot* p=new TAGPlot(0); //Cuts off
#  
#  TAGPlot* p=new TAGPlot(); //Cuts on
#  std::vector<TAGSpill> dump=Get_AG_Spills(${RUNNO}, {\"${DUMP_NAME}\"}, {0});
#
#  p->SetTimeRange(dump.front().GetStartTime(),dump.front().GetStopTime());
#  p->AddEvents(runNumber,dump.front().GetStartTime(),dump.front().GetStopTime());
#  TString cname = TString::Format(\"cVTX_R%d\",runNumber);
#  std::cout<<\"NVerts:\"<<p->GetTotalVertices()<<std::endl;
#  p->Canvas(cname)->SaveAs(\"R${RUNNO}_Verts.png\");
#" | root -l

echo "Plot_TPC(5365, ${START_TIME} + ${FUDGE_TIME}, ${STOP_TIME}-${FUDGE_TIME}, true)->SaveAs(\"R${RUNNO}_Verts_WithCuts.png\");
" | root -l -b

echo "Plot_TPC(5365, ${START_TIME} + ${FUDGE_TIME}, ${STOP_TIME}-${FUDGE_TIME}, false)->SaveAs(\"R${RUNNO}_Verts_NoCuts.png\");
" | root -l -b
