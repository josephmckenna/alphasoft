#ifndef _TA2Spill_
#define _TA2Spill_
#include "TObject.h"
#include <iostream>
#include <bitset>
#include "TString.h"
#include "sqlite3.h"
#include "DumpHandling.h"

//Base class for SIS and Chronobox integrals
class TSpillScalerData: public TObject
{
   public:
   double StartTime;
   double StopTime;
   TSpillScalerData();
   ~TSpillScalerData();
   ClassDef(TSpillScalerData,1);
};
#include "../a2lib/include/TSVD_QOD.h"
#include "../a2lib/include/TSISEvent.h"
//Class to integrate SIS and VF48 event counts
class TA2SpillScalerData: public TSpillScalerData
{
   public:
   int            DetectorCounts[NUM_SIS_CHANNELS*NUM_SIS_MODULES];
   unsigned long  SISFilled; //bit mask for each channel (NUM_SIS_CHANNELS*NUM_SIS_MODULES)

   int            FirstVF48Event;
   int            LastVF48Event;
   int            VF48Events;
   int            Verticies;
   int            PassCuts;
   int            PassMVA;
   bool           SVDFilled;

   TA2SpillScalerData();
   TA2SpillScalerData(TA2SpillScalerData* a);
   TA2SpillScalerData(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_CHANNELS*NUM_SIS_MODULES>* d);
   TA2SpillScalerData* operator/(const TA2SpillScalerData* b);
   bool Ready( bool have_svd);
   using TObject::Print;
   virtual void Print();
   ~TA2SpillScalerData();
   ClassDef(TA2SpillScalerData,1);
};

//Class to inegrate AG scaler and data counts
class TAGSpillScalerData: public TSpillScalerData
{
   TAGSpillScalerData();
   TAGSpillScalerData(TAGSpillScalerData* a);
   TAGSpillScalerData* operator/(const TAGSpillScalerData* b);
   ClassDef(TAGSpillScalerData,1);
};

class TSpillSequencerData: public TObject
{
   public:
   int          fSequenceNum; //Sequence number 
   int          fDumpID; //Row number 
   std::string  fSeqName;
   int          fStartState;
   int          fStopState;
   TSpillSequencerData();
   TSpillSequencerData(TSpillSequencerData* a);
//   TSpillSequencerData(DumpPair* d);
   TSpillSequencerData* operator/(const TSpillSequencerData* b);
   using TObject::Print;
   virtual void Print();
   ~TSpillSequencerData();
   ClassDef(TSpillSequencerData,1);
};
class TA2SpillSequencerData: public TSpillSequencerData
{
   public:
   TA2SpillSequencerData(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_CHANNELS*NUM_SIS_MODULES>* d);
   TA2SpillSequencerData(TA2SpillSequencerData* s);
   TA2SpillSequencerData();
   ~TA2SpillSequencerData();
   ClassDef(TA2SpillSequencerData,1);
};

class TSpill: public TObject
{
public:
   int                   RunNumber;
   bool                  IsDumpType;
   bool                  IsInfoType;
   int                   Unixtime;
   std::string           Name;

   TSpill();
   //TSpill(const char* name);
   void InitByName(const char* format, va_list args);
   TSpill( const char* format, ...);
   TSpill* operator/(const TSpill* b);
   TSpill(TSpill* a);
   using TObject::Print;
   virtual void Print();

   virtual int AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt);
   virtual TString Content(std::vector<int>*, int& );
   ~TSpill();
   ClassDef(TSpill,1);

};

class TA2Spill: public TSpill
{
public:
   TA2SpillScalerData* ScalerData;
   TA2SpillSequencerData*  SeqData;
   TA2Spill();
   TA2Spill(const char* format, ...);
   TA2Spill(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_CHANNELS*NUM_SIS_MODULES>* d);
   TA2Spill* operator/(const TA2Spill* b);
   TA2Spill(const TA2Spill* a);
   using TObject::Print;
   virtual void Print();


   int AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt);
   TString Content(std::vector<int>*, int& );
   
   bool Ready( bool have_svd);
   ~TA2Spill();
   ClassDef(TA2Spill,1);
};

class TAGSpill: public TSpill
{
public:
   TAGSpillScalerData* ScalerData;
   TAGSpill(TAGSpill* a);
   ClassDef(TAGSpill,1);
};


#endif
