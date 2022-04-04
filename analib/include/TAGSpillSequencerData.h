#ifndef _TAGSPILLSEQUENCERDATA_
#define _TAGSPILLSEQUENCERDATA_

#if BUILD_AG

#include "TSpillSequencerData.h"
#include "ChronoUtil.h"
#include "TDumpMarkerPair.h"
class TAGDetectorEvent;
class TChronoBoardCounter;

class TAGSpillSequencerData: public TSpillSequencerData
{
   public:
   TAGSpillSequencerData();
   ~TAGSpillSequencerData();
   TAGSpillSequencerData(const TAGSpillSequencerData& s);
   TAGSpillSequencerData(TDumpMarkerPair<TAGDetectorEvent,TChronoBoardCounter,CHRONO_N_BOARDS>* d);


   ClassDef(TAGSpillSequencerData,1);
};

#endif
#endif