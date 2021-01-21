#ifndef _TA2Spill_
#define _TA2Spill_
#include "TSpill.h"


#ifdef BUILD_A2
#include "TSVD_QOD.h"
#include "TSISEvent.h"

//Class to integrate SIS and VF48 event counts
class TA2SpillScalerData: public TSpillScalerData
{
   public:
   //TA2SpillScalerData();
   ~TA2SpillScalerData();
   TA2SpillScalerData(int n_scaler_channels=NUM_SIS_CHANNELS*NUM_SIS_MODULES);
   TA2SpillScalerData(TA2SpillScalerData* a);
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
   TA2SpillSequencerData(TA2SpillSequencerData* s);
   TA2SpillSequencerData(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d);


   ClassDef(TA2SpillSequencerData,1);
};

class TA2Spill: public TSpill
{
public:
   TA2SpillScalerData* ScalerData;
   TA2SpillSequencerData*  SeqData;
   TA2Spill();
   TA2Spill(int runno);
   TA2Spill(int runnno, const char* format, ...);
   TA2Spill(int runnno, DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d);
   TA2Spill* operator/( TA2Spill* b);
   TA2Spill* operator+( TA2Spill* b);
   TA2Spill(const TA2Spill* a);
   using TObject::Print;
   virtual void Print();


   int AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt);
   TString Content(std::vector<int>*, int& );
   
   bool Ready( bool have_svd);
   ~TA2Spill();
   ClassDef(TA2Spill,1);
};
#endif




#endif