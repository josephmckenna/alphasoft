#ifndef _TA2Spill_
#define _TA2Spill_
#include "TObject.h"
#include <iostream>
#include <bitset>
#include "TString.h"
#include "sqlite3.h"
#include "DumpHandling.h"
#include "Sequencer_Channels.h"

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
   TSpillScalerData(const TSpillScalerData& a);
   TSpillScalerData& operator =(const TSpillScalerData& rhs);
   double GetStartTime() const { return StartTime; }
   double GetStopTime() const { return StopTime; }
   template <class ScalerData>
   ScalerData* operator/(const ScalerData* b);
   template <class ScalerData>
   ScalerData* operator+(const ScalerData* b);
   bool Ready( bool have_vertex_detector);
   double GetDumpLength() { return StopTime - StartTime; };

   ClassDef(TSpillScalerData,1);
};
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
   TA2SpillScalerData(const TA2SpillScalerData& a);
   TA2SpillScalerData& operator =(const TA2SpillScalerData& rhs);
   //TA2SpillScalerData* operator/(const TA2SpillScalerData* b);
   TA2SpillScalerData(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d);
   using TObject::Print;
   virtual void Print();

   ClassDef(TA2SpillScalerData,1);
};
#endif
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
   TAGSpillScalerData& operator =(const TAGSpillScalerData& rhs);
   //TAGSpillScalerData* operator/(const TAGSpillScalerData* b);
   TAGSpillScalerData(DumpPair<TStoreEvent,ChronoEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d);
   using TObject::Print;
   virtual void Print();
   ClassDef(TAGSpillScalerData,1);
};
#endif

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
   TSpillSequencerData(const TSpillSequencerData& a);
   TSpillSequencerData& operator =(const TSpillSequencerData& rhs);
//   TSpillSequencerData(DumpPair* d);
   TSpillSequencerData* operator/(const TSpillSequencerData* b);
   using TObject::Print;
   virtual void Print();

   ClassDef(TSpillSequencerData,1);
};
#ifdef BUILD_A2
class TA2SpillSequencerData: public TSpillSequencerData
{
   public:
   TA2SpillSequencerData();
   ~TA2SpillSequencerData();
   TA2SpillSequencerData(const TA2SpillSequencerData& s);
   TA2SpillSequencerData& operator =(const TA2SpillSequencerData& rhs);
   TA2SpillSequencerData(DumpPair<TSVD_QOD,TSISEvent,NUM_SIS_MODULES>* d);


   ClassDef(TA2SpillSequencerData,1);
};
#endif
#ifdef BUILD_AG
class TAGSpillSequencerData: public TSpillSequencerData
{
   public:
   TAGSpillSequencerData();
   ~TAGSpillSequencerData();
   TAGSpillSequencerData(const TAGSpillSequencerData& s);
   TAGSpillSequencerData& operator =(const TAGSpillSequencerData& rhs);
   TAGSpillSequencerData(DumpPair<TStoreEvent,ChronoEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d);


   ClassDef(TAGSpillSequencerData,1);
};
#endif
class TSpill: public TObject
{
public:
   int                   RunNumber;
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
   TSpill(const TSpill& a);
   TSpill& operator =(const TSpill& rhs);
   bool IsMatchForDumpName(std::string dumpname, bool exact = true);
   virtual double GetStartTime() const = 0;
   virtual double GetStopTime() const = 0;
   bool DumpHasMathSymbol() const;
   using TObject::Print;
   virtual void Print();


   virtual int AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt);
   TString Content(std::vector<int>*, int& );

   ClassDef(TSpill,1);

};
#ifdef BUILD_A2
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
   TA2Spill(const TA2Spill& a);
   TA2Spill& operator =(const TA2Spill& rhs);
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
   TString Content(std::vector<int>*, int& );
   
   bool Ready( bool have_svd);
   ~TA2Spill();
   ClassDef(TA2Spill,1);
};
#endif
#ifdef BUILD_AG
class TAGSpill: public TSpill
{
public:
   TAGSpillScalerData* ScalerData;
   TAGSpillSequencerData*  SeqData;
   TAGSpill();
   TAGSpill(int runno, const char* format, ...);
   TAGSpill(int runno, DumpPair<TStoreEvent,ChronoEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS>* d);
   ~TAGSpill();
   TAGSpill(const TAGSpill& a);
   TAGSpill& operator =(const TAGSpill& rhs);
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
   TString Content(std::vector<std::pair<int,int>>*, int& );
   ClassDef(TAGSpill,1);
};
#endif

#endif
