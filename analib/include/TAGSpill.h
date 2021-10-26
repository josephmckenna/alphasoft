#ifndef _TAGSPILL_
#define _TAGSPILL_
#include "TSpill.h"



#ifdef BUILD_AG
#include "TStoreEvent.hh"
#include "store_cb.h"
#include "TChronoChannel.h"
#include "TChronoChannelName.h"

#define DUMP_NAME_WIDTH 40


struct CbHitCounter
{
   double   time;
   uint32_t epoch;
   uint32_t timestamp;
   uint32_t channel;
   uint32_t flags;
   int      Counts;
   CbHitCounter(const CbHit& c)
   {
      time = c.time;
      epoch = c.epoch;
      timestamp = c.timestamp;
      channel = c.channel;
      flags = c.flags;
      if (!(c.flags & CB_HIT_FLAG_TE))
         Counts = 1;
      else
         Counts = 0;
   }
};


//Class to inegrate AG scaler and data counts
class TAGSpillScalerData: public TSpillScalerData
{
   public:
   //TAGSpillScalerData();
   ~TAGSpillScalerData();
   TAGSpillScalerData(int n_scaler_channels=CHRONO_N_BOARDS*CHRONO_N_CHANNELS);
   TAGSpillScalerData(const TAGSpillScalerData& a);
   //TAGSpillScalerData* operator/(const TAGSpillScalerData* b);
   TAGSpillScalerData(DumpPair<TStoreEvent,TCbFIFOEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d);
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
   TAGSpillSequencerData(DumpPair<TStoreEvent,TCbFIFOEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d);


   ClassDef(TAGSpillSequencerData,1);
};

class TAGSpill: public TSpill
{
public:
   TAGSpillScalerData* ScalerData;
   TAGSpillSequencerData*  SeqData;
   TAGSpill();
   TAGSpill(int runno, uint32_t unixtime);
   TAGSpill(int runno, uint32_t unixtime, const char* format, ...);
   TAGSpill(int runno, DumpPair<TStoreEvent,TCbFIFOEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d);
   TAGSpill* operator/( TAGSpill* b);
   TAGSpill* operator+( TAGSpill* b);
   TAGSpill(const TAGSpill& a);
   double GetStartTime() const
   {
      if (ScalerData)
         return ScalerData->GetStartTime();
      else
         return -1.;
   }
   double GetStopTime() const
   {
      if (ScalerData)
         return ScalerData->GetStopTime();
      else
         return -1;
   }
   std::string GetSequenceName() const
   {
      if (SeqData)
         return SeqData->fSeqName;
      else
         return "none";
   }
   using TObject::Print;
   virtual void Print();

   
   TString Content(const std::vector<TChronoChannel> );
   std::string ContentCSVTitle(std::vector<std::string> ChannelNames = {}) const
   {
      std::string title = TSpill::ContentCSVTitle();
      if (SeqData)
         title += SeqData->ContentCSVTitle();
      if (ScalerData)
         title += ScalerData->ContentCSVTitle(ChannelNames);
      return title;
   }
      std::string ContentCSV() const
   {
      std::string line = TSpill::ContentCSV();
      if (SeqData)
         line += SeqData->ContentCSV();
      if (ScalerData)
         line += ScalerData->ContentCSV();
      return line;
   }
   bool Ready(bool have_BV);
   ~TAGSpill();
   
   ClassDef(TAGSpill,1);
};
#endif
#endif
