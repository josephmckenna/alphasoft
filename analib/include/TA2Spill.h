#ifndef _TA2Spill_
#define _TA2Spill_
#include "TSpill.h"


#ifdef BUILD_A2
#include "TSVD_QOD.h"
#include "TSISEvent.h"

#define DUMP_NAME_WIDTH 40

//Class to integrate SIS and VF48 event counts
class TA2SpillScalerData: public TSpillScalerData
{
   public:
   //TA2SpillScalerData();
   ~TA2SpillScalerData();
   TA2SpillScalerData(int n_scaler_channels=NUM_SIS_CHANNELS*NUM_SIS_MODULES);
   TA2SpillScalerData(const TA2SpillScalerData& a);
   //TA2SpillScalerData* operator/(const TA2SpillScalerData* b);
   TA2SpillScalerData(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d);
   using TObject::Print;
   virtual void Print();

   ClassDef(TA2SpillScalerData,1);
};

class TA2SpillSequencerData: public TSpillSequencerData
{
   public:
   TA2SpillSequencerData();
   ~TA2SpillSequencerData();
   TA2SpillSequencerData(const TA2SpillSequencerData& s);
   TA2SpillSequencerData(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d);


   ClassDef(TA2SpillSequencerData,1);
};

class TA2Spill: public TSpill
{
public:
   TA2SpillScalerData* ScalerData;
   TA2SpillSequencerData*  SeqData;
   TA2Spill();
   TA2Spill(int runno, uint32_t unixtime);
   TA2Spill(int runno, uint32_t unixtime, const char* format, ...);
   TA2Spill(int runno, DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d);
   TA2Spill* operator/( TA2Spill* b);
   TA2Spill* operator+( TA2Spill* b);
   TA2Spill(const TA2Spill& a);
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
   using TObject::Print;
   virtual void Print();


   int AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt);
   TString Content(const std::vector<int>);
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

   bool Ready( bool have_svd);
   ~TA2Spill();
   ClassDef(TA2Spill,1);
};
#endif




#endif
