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
   TSpillScalerData(const TSpillScalerData& a);
   TSpillScalerData& operator =(const TSpillScalerData& rhs);
   double GetStartTime() const { return StartTime; }
   double GetStopTime() const { return StopTime; }
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
   std::string ContentCSVTitle(std::vector<std::string> ChannelNames = {}) const
   {
      std::string title = "Start Time (s),Stop Time (s),Durations (s),";
      if (ChannelNames.size() == DetectorCounts.size())
      {
         for (size_t i=0; i<ChannelNames.size(); i++)
         {
            title += ChannelNames.at(i) + ",";
         }
      }
      else 
      {
         std::cout <<"Warning: " << __FILE__  << ":" << __LINE__ << "\tNo detector channel names given\n";
         for (size_t i=0; i<DetectorCounts.size(); i++)
         {
            title += "Scaler Channel " + std::to_string(i) + ",";
         }
      }
      //This is very verbose... probably a better way of doing this
      title += "First Vertex Event,LastVertexEvent,Vertex Events,Verticies,PassCuts,PassMVA,";
      return title;
   }
   std::string ContentCSV() const
   {
      std::string line;
      line += std::to_string(StartTime) + "," +
              std::to_string(StopTime) + "," +
              std::to_string(StopTime - StartTime) + ",";
      for (size_t i = 0; i<DetectorCounts.size(); i++)
      {
         if (ScalerFilled.at(i))
            line += std::to_string(DetectorCounts.at(i)) + ",";
         else
            line += "-1,";
      }
      //This is very verbose... probably a better way of doing this
      line += std::to_string(FirstVertexEvent) + ",";
      line += std::to_string(LastVertexEvent) + ",";
      line += std::to_string(VertexEvents) + ",";
      line += std::to_string(Verticies) + ",";
      line += std::to_string(PassCuts) + ",";
      line += std::to_string(PassMVA) + ",";
      return line;
   }
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
   TSpillSequencerData(const TSpillSequencerData& a);
   TSpillSequencerData& operator =(const TSpillSequencerData& rhs);
//   TSpillSequencerData(DumpPair* d);
   TSpillSequencerData* operator/(const TSpillSequencerData* b);
   using TObject::Print;
   virtual void Print();
   std::string ContentCSVTitle() const
   {
      std::string title = "Sequencer ID,Sequencer Name, Dump ID, Start State, Stop State,";
      return title;
   }
   std::string ContentCSV() const
   {
      std::string line = std::to_string(fSequenceNum) + "," +
                         fSeqName + "," +
                         std::to_string(fDumpID) + "," +
                         std::to_string(fStartState) + "," +
                         std::to_string(fStopState) + ",";
      return line;
   }
   ClassDef(TSpillSequencerData,1);
};
class TSpill: public TObject
{
public:
   int                   RunNumber;
   bool                  IsDumpType;
   bool                  IsInfoType;
   uint32_t              Unixtime;
   std::string           Name;

   TSpill();
   TSpill(int runno, uint32_t unixtime);
   ~TSpill();
   //TSpill(const char* name);
   void InitByName(const char* format, va_list args);
   TSpill(int runno, uint32_t unixtime, const char* format, ...);
   TSpill(const TSpill& a);
   TSpill& operator =(const TSpill& rhs);
private:
   //Recursive function to compare strings with wildcard support (called by IsMatchForDumpName)
   bool MatchWithWildCards(const char *first, const char * second);
public:
   bool IsMatchForDumpName(const std::string& dumpname);
   virtual double GetStartTime() const = 0;
   virtual double GetStopTime() const = 0;
   bool DumpHasMathSymbol() const;
   using TObject::Print;
   virtual void Print();


   virtual int AddToDatabase(sqlite3 *db, sqlite3_stmt * stmt);
   TString Content(std::vector<int>*, int& );
   std::string ContentCSVTitle() const
   {
      std::string title = "RunNumber, Sequencer Start Time (Unix Time), Dump Name, Is Dump Type, Is Info Type,";
      return title;
   }
   std::string ContentCSV() const
   {
      std::string line = std::to_string(RunNumber) + "," +
                         std::to_string(Unixtime) + "," +
                         Name + ",";
                         if (IsDumpType)
                            line +="1,";
                         else
                            line +="0,";
                         if (IsInfoType)
                            line +="1,";
                         else
                            line +="0,";
      return line;
   }
};

#endif
