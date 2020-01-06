#include "TA2RunQOD.h"


ClassImp(TA2RunQOD);
TA2RunQOD::TA2RunQOD(TARunInfo* runinfo)
{
   RunNumber= runinfo->fRunNo;
   midas_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
   midas_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
   CosmicRunTrigger = runinfo->fOdb->odbReadBool("/Experiment/edit on start/Cosmic_Run");
}
