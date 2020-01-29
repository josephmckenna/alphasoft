#include "TA2RunQOD.h"


ClassImp(TA2RunQOD);
TA2RunQOD::TA2RunQOD(TARunInfo* runinfo)
{
   RunNumber= runinfo->fRunNo;
#ifdef INCLUDE_VirtualOdb_H
   midas_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
   midas_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
   CosmicRunTrigger = runinfo->fOdb->odbReadBool("/Experiment/edit on start/Cosmic_Run");
#endif

#ifdef INCLUDE_MVODB_H
   runinfo->fOdb->RU32("/Runinfo/Start time binary",(uint32_t*) &midas_start_time);
   runinfo->fOdb->RU32("/Runinfo/Stop time binary",(uint32_t*) &midas_stop_time);
   runinfo->fOdb->RB("/Experiment/edit on start/Cosmic_Run",&CosmicRunTrigger);
#endif

}
