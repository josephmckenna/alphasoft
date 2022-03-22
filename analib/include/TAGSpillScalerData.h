#ifndef _TAGSPILLSCALERDATA_
#define _TAGSPILLSCALERDATA_


#if BUILD_AG

#include "TSpillScalerData.h"
#include "TAGDetectorEvent.hh"
#include "TChronoBoardCounter.h"
#include "ChronoUtil.h"
#include "TDumpMarkerPair.h"
//Class to inegrate AG scaler and data counts
class TAGSpillScalerData: public TSpillScalerData
{
   public:
   TAGSpillScalerData();
   ~TAGSpillScalerData();
   TAGSpillScalerData(int n_scaler_channels);
   TAGSpillScalerData(const TAGSpillScalerData& a);
   //TAGSpillScalerData* operator/(const TAGSpillScalerData* b);
   TAGSpillScalerData(TDumpMarkerPair<TAGDetectorEvent,TChronoBoardCounter,CHRONO_N_BOARDS>* d);
   using TObject::Print;
   virtual void Print();

   ClassDef(TAGSpillScalerData,1);
};

#endif
#endif