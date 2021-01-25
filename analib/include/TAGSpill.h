#ifndef _TAGSPILL_
#define _TAGSPILL_
#include "TSpill.h"



#ifdef BUILD_AG
#include "TStoreEvent.hh"
#include "TChrono_Event.h"
#include "chrono_module.h"
//Class to inegrate AG scaler and data counts
class TAGSpillScalerData: public TSpillScalerData
{
   public:
   //TAGSpillScalerData();
   ~TAGSpillScalerData();
   TAGSpillScalerData(int n_scaler_channels=CHRONO_N_BOARDS*CHRONO_N_CHANNELS);
   TAGSpillScalerData(const TAGSpillScalerData& a);
   //TAGSpillScalerData* operator/(const TAGSpillScalerData* b);
   TAGSpillScalerData(DumpPair<TStoreEvent,ChronoEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d);
   using TObject::Print;
   virtual void Print();
   ClassDef(TAGSpillScalerData,1);
};

class TAGSpillSequencerData: public TSpillSequencerData
{
   public:
   TAGSpillSequencerData();
   ~TAGSpillSequencerData();
   TAGSpillSequencerData(const TAGSpillSequencerData& s);
   TAGSpillSequencerData(DumpPair<TStoreEvent,ChronoEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d);


   ClassDef(TAGSpillSequencerData,1);
};

class TAGSpill: public TSpill
{
public:
   TAGSpillScalerData* ScalerData;
   TAGSpillSequencerData*  SeqData;
   TAGSpill();
   TAGSpill(int runno, const char* format, ...);
   TAGSpill(int runno, DumpPair<TStoreEvent,ChronoEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d);
   ~TAGSpill();
   TAGSpill(TAGSpill* a);
   TString Content(std::vector<std::pair<int,int>>*, int& );
   ClassDef(TAGSpill,1);
};
#endif
#endif