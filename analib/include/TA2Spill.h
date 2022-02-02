#ifndef _TA2Spill_
#define _TA2Spill_
#include "TSpillScalerData.h"
#include "TA2SpillSequencerData.h"

#ifdef BUILD_A2
#include "TSVD_QOD.h"
#include "TSISChannel.h"
#include "TSISEvent.h"
#include "TA2SpillScalerData.h"

#define DUMP_NAME_WIDTH 40

class TA2Spill: public TSpill
{
public:
   TA2SpillScalerData* ScalerData;
   TA2SpillSequencerData*  SeqData;
   TA2Spill();
   TA2Spill(int runno, uint32_t unixtime);
   TA2Spill(int runno, uint32_t unixtime, const char* format, ...);
   TA2Spill(int runno, TDumpMarkerPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d);
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
   std::string GetSequenceName() const
   {
      if (SeqData)
         return SeqData->fSeqName;
      else
         return "none";
   }
   using TObject::Print;
   virtual void Print();


   int AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt);
   TString Content(const std::vector<TSISChannel>);
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
