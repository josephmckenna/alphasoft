#ifndef _TA2SPILLSEQUENCERDATA_
#define _TA2SPILLSEQUENCERDATA_


#include "TSpillSequencerData.h"
#include "TDumpMarkerPair.h"

class TSVD_QOD;
class TSISEvent;
#include "TSISChannel.h"

class TA2SpillSequencerData: public TSpillSequencerData
{
   public:
   TA2SpillSequencerData();
   ~TA2SpillSequencerData();
   TA2SpillSequencerData(const TA2SpillSequencerData& s);
   TA2SpillSequencerData(TDumpMarkerPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d);

   


   ClassDef(TA2SpillSequencerData,1);
};

#endif