#ifndef _TSpill_
#define _TSpill_
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
   TSpillScalerData(TSpillScalerData* a);
   template <class ScalerData>
   ScalerData* operator/(const ScalerData* b)
   {
      ScalerData* c=new ScalerData(this->DetectorCounts.size());

      c->VertexFilled=this->VertexFilled;
      c->ScalerFilled=this->ScalerFilled;

      c->StartTime=b->StartTime;
      c->StopTime=this->StopTime;

      for (size_t i=0; i<DetectorCounts.size(); i++)
      {
         //std::cout<<this->DetectorCounts[i] << " / "<< b->DetectorCounts[i] <<std::endl;
         if (b->DetectorCounts[i])
            c->DetectorCounts[i]=100*(double)this->DetectorCounts[i]/(double)b->DetectorCounts[i];
         else
            c->DetectorCounts[i]=std::numeric_limits<int>::infinity();
      }
      if (b->VertexEvents)
         c->VertexEvents= 100*(double)this->VertexEvents / (double)b->VertexEvents;
      if (b->Verticies)
         c->Verticies   = 100*(double)this->Verticies  / (double)b->Verticies;
      if (b->PassCuts)
         c->PassCuts    = 100*(double)this->PassCuts   / (double)b->PassCuts;
      if (b->PassMVA)
         c->PassMVA     = 100*(double)this->PassMVA    / (double)b->PassMVA;
      return c;
   }
   template <class ScalerData>
   ScalerData* operator+(const ScalerData* b)
   {
      ScalerData* c=new ScalerData(this->DetectorCounts.size());

      c->VertexFilled=this->VertexFilled;
      c->ScalerFilled=this->ScalerFilled;

      c->StartTime =b->StartTime;
      c->StopTime  =this->StopTime;

      for (size_t i=0; i<DetectorCounts.size(); i++)
      {
         //std::cout<<this->DetectorCounts[i] << " / "<< b->DetectorCounts[i] <<std::endl;
         if (b->DetectorCounts[i])
            c->DetectorCounts[i]=this->DetectorCounts[i]+b->DetectorCounts[i];
         else
            c->DetectorCounts[i]=std::numeric_limits<int>::infinity();
      }
      if (b->VertexEvents)
         c->VertexEvents= this->VertexEvents + b->VertexEvents;
      if (b->Verticies)
         c->Verticies = this->Verticies  + b->Verticies;
      if (b->PassCuts)
         c->PassCuts  = this->PassCuts   + b->PassCuts;
      if (b->PassMVA)
        c->PassMVA   = this->PassMVA    + b->PassMVA;
      return c;
   }
   bool Ready( bool have_vertex_detector);
   double GetDumpLength() { return StopTime - StartTime; };

   ClassDef(TSpillScalerData,1);
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
   bool IsMatchForDumpName(std::string dumpname, bool exact = true);
   bool DumpHasMathSymbol() const;
   using TObject::Print;
   virtual void Print();


   virtual int AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt);
   TString Content(std::vector<int>*, int& );

   ClassDef(TSpill,1);

};

#endif
