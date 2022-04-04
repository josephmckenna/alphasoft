#ifndef _TA2SpillScalerData_
#define _TA2SpillScalerData_
#include "TSpillScalerData.h"
#include "TSISChannel.h"
#include "TSVD_QOD.h"
#include "TSISEvent.h"
//Class to integrate SIS and VF48 event counts
class TA2SpillScalerData: public TSpillScalerData
{
   public:
   //TA2SpillScalerData();
   ~TA2SpillScalerData();
   TA2SpillScalerData(int n_scaler_channels=NUM_SIS_CHANNELS*NUM_SIS_MODULES);
   TA2SpillScalerData(const TA2SpillScalerData& a);
   //TA2SpillScalerData* operator/(const TA2SpillScalerData* b);
   TA2SpillScalerData(TDumpMarkerPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d);
   using TObject::Print;
   virtual void Print();

   ClassDef(TA2SpillScalerData,1);
};
#endif