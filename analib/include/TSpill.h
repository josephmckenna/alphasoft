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

   //Chrono box support 2D array of values... (boards vs channels)
   //SIS only supports channels
   std::vector<int>   DetectorCounts;
   std::vector<bool>  ScalerFilled;

   int            FirstVertexEvent;
   int            LastVertexEvent;
   int            VertexEvents;
   int            Verticies;
   int            PassCuts;
   int            PassMVA;
   bool           VertexFilled;
   TSpillScalerData();
   ~TSpillScalerData();
   TSpillScalerData(int n_scaler_channels);
   TSpillScalerData(TSpillScalerData* a);
   template <class ScalerData>
   ScalerData* operator/(const ScalerData* b);
   template <class ScalerData>
   ScalerData* operator+(const ScalerData* b);
   bool Ready( bool have_vertex_detector);


   ClassDef(TSpillScalerData,1);
};
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
   TAGSpillScalerData(TAGSpillScalerData* a);
   //TAGSpillScalerData* operator/(const TAGSpillScalerData* b);
   TAGSpillScalerData(DumpPair<TStoreEvent,ChronoEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d);
   using TObject::Print;
   virtual void Print();
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
   ~TSpillSequencerData();
   TSpillSequencerData(TSpillSequencerData* a);
//   TSpillSequencerData(DumpPair* d);
   TSpillSequencerData* operator/(const TSpillSequencerData* b);
   using TObject::Print;
   virtual void Print();

   ClassDef(TSpillSequencerData,1);
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

class TAGSpillSequencerData: public TSpillSequencerData
{
   public:
   TAGSpillSequencerData();
   ~TAGSpillSequencerData();
   TAGSpillSequencerData(TAGSpillSequencerData* s);
   TAGSpillSequencerData(DumpPair<TStoreEvent,ChronoEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d);


   ClassDef(TAGSpillSequencerData,1);
};

class TSpill: public TObject
{
public:
   const int             RunNumber;
   bool                  IsDumpType;
   bool                  IsInfoType;
   int                   Unixtime;
   std::string           Name;

   TSpill();
   TSpill(int runno);
   ~TSpill();
   //TSpill(const char* name);
   void InitByName(const char* format, va_list args);
   TSpill(int runno, const char* format, ...);
   TSpill* operator/(const TSpill* b);
   TSpill(TSpill* a);
   bool DumpHasMathSymbol() const;
   using TObject::Print;
   virtual void Print();


   virtual int AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt);
   virtual TString Content(std::vector<int>*, int& );

   ClassDef(TSpill,1);

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
