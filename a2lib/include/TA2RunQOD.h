#ifndef _TA2RunQOD_
#define _TA2RunQOD_
#include "manalyzer.h"

class TA2RunQOD: public TObject
{
   public:
   int RunNumber;
   bool CosmicRunTrigger;
   TString Git;
   int alphaStripsNumber;
   TString SequenceHash;
   TString RWHash;
   TString TrappingHash;
   time_t midas_start_time;
   time_t midas_stop_time;
   
   TA2RunQOD(TARunInfo* runinfo);
   //Run QOD?
//VF48 - SIS Drift
//VF48 - SIS Offset (s)
//VF48 - SIS Stdev

//Labview QOD?
//EURT1 at start
//EURT1 at stop
//EURT2 at start
//EURT2 at stop
//Hall probe T at start
//Hall probe T at stop
//Hall probe Field at start (x1.391)
//Hall probe Field at stop (x1.391)
   ClassDef(TA2RunQOD,1);

};
#endif
