#!/bin/bash


#Global vars:
MIDAS_PATH="/alpha/agdaq/data/"


MainMenu()
{

   #check run number against current run:
   if [ `hostname` == "alphagdaq.cern.ch" ]; then
      Run_number=`odbedit -e ${MIDAS_EXPT_NAME} -c 'ls "/Runinfo/Run number"'`
      number=`echo $Run_number | awk '{print $3}'`
      echo "Current run according to midas: ${number}"
      if [ ${RUNNO} -gt ${number} ]; then
         echo "Run number ${RUNNO} hasn't happened yet... go back to working on a time machine"
         exit
      fi
      if [ ${RUNNO} -eq ${number} ]; then
         STATE=`odbedit -e ${MIDAS_EXPT_NAME} -c 'ls "/Runinfo/state"'`
         STATE_NO=`echo $STATE | awk '{print $2}'`
         if [ ${STATE_NO} -gt 1 ]; then
            echo "Run number ${RUNNO} is still going... be patient!"
            exit
         fi
      fi
   fi
   if [ ${RUNNO} -lt 10000 ]; then
      RUNNO_FILE="0${RUNNO}"
      echo "${RUNNO_FILE} reformatted"
   fi
echo "
################################################################################
			MENU: Please input number below:
################################################################################

0. Run agana in dump
1. Run no reco
2. Pbar Verticies
work: More tests... faster running...
"
read MENUVAR
case $MENUVAR in
0  ) RUNAGANA;;
1  ) runNoRecoAnalysis;;
2  ) PbarVertexMenu;;

esac

}

RUNAGANA()
{
   echo "Give me a dump name, or insert number for start time (TPC time scale):"
   echo "Example:"
   echo "FastRampDown"
   echo "Pbar"
   read DUMPNAME
   if [ "$DUMPNAME" -eq "$DUMPNAME" ]; then
      START=${DUMPNAME}
      echo "Give me a stop time (TPC time scale)"
      read STOP
      echo "./agana.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub*.mid.lz4 -- --usetimerange ${START} ${STOP}"
      sleep 1
      ./agana.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub*.mid.lz4 -- --usetimerange ${START} ${STOP} > R${RUNNO}_timerange.log
      root 'Plot_TPC(${RUNNO},"Pbar")'
   else
      runNoRecoAnalysis
      ROUGHNESS="1."
      TEXT=`echo "double offset=GetRunTimeOfCount(${RUNNO},\"TPC\",1);
      double tmin=MatchEventToTime(${RUNNO},\"${DUMPNAME}\",true);
      double tmax=MatchEventToTime(${RUNNO},\"${DUMPNAME}\",false);
      std::cout <<tmin<<\"-\"<<tmax<<std::endl;
      std::cout <<tmin-offset<<\"-\"<<tmax-offset<<std::endl;
      std::cout<<\"ROUGHSTART:\t\"<<tmin-offset-${ROUGHNESS}<<\"\t\"<<\"ROUGHSTOP:\t\"<<tmax-offset+${ROUGHNESS}<<std::endl;
      .q
      " | root -l | grep ROUGH`
      echo "${TEXT}"
      echo ""
      START=`echo "${TEXT}" |  awk 'BEGIN {FS="\t"}; {print $2}'`
      STOP=`echo "${TEXT}" |  awk 'BEGIN {FS="\t"}; {print $4}'`
      echo "START:${START}"
      echo "STOP :${STOP}"
      echo "./agana.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub*.mid.lz4 -- --usetimerange ${START} ${STOP}"
      sleep 1
      ./agana.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub*.mid.lz4 -- --usetimerange ${START} ${STOP} > R${RUNNO}_timerange.log
      root 'Plot_TPC(${RUNNO},"Pbar")'
   fi
}


runNoRecoAnalysis()
{
   if [ -f output${RUNNO_FILE}.root ]; then
      echo "output${RUNNO_FILE}.root file found!"
      sleep 1
   else
      echo "./agana.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub*.mid.lz4 -- --recoff"
      sleep 1
      ./agana.exe ${MIDAS_PATH}/run${RUNNO_FILE}sub*.mid.lz4 -- --recoff
   fi
}





export RUNNO=$@
if [[ $RUNNO -gt 3000 ]]
then
   echo "Run number given seems realistic"
   MainMenu
else
   echo "
Arguments given not recognised or run number is bad...

Please input Run Number... or press ctrl + c to exit
"
   read RUNNO
   MainMenu
fi
