#ifndef _TSpillScalerData_
#define _TSpillScalerData_
#include "TSpill.h"
//Base class for SIS and Chronobox integrals
class TSpillScalerData: public TObject
{
   public:
   double fStartTime;
   double fStopTime;

   //Chrono box support 2D array of values... (boards vs channels)
   //SIS only supports channels
   std::vector<int>   fDetectorCounts; //Think about changing this to a double
   std::vector<bool>  ScalerFilled;

   int            FirstVertexEvent;
   int            LastVertexEvent;
   int            VertexEvents;
   int            fVerticies;
   int            fPassCuts;
   int            fPassMVA;
   bool           VertexFilled;
   TSpillScalerData();
   ~TSpillScalerData();
   TSpillScalerData(int n_scaler_channels);
   TSpillScalerData(const TSpillScalerData& a);
   TSpillScalerData& operator =(const TSpillScalerData& rhs);
   double GetStartTime() const { return fStartTime; }
   double GetStopTime() const { return fStopTime; }
   template <class ScalerData>
   ScalerData* operator/(const ScalerData* b)
   {
      ScalerData* c=new ScalerData(this->fDetectorCounts.size());

      c->VertexFilled=this->VertexFilled;
      c->ScalerFilled=this->ScalerFilled;

      c->fStartTime=b->fStartTime;
      c->fStopTime=this->fStopTime;

      for (size_t i=0; i<fDetectorCounts.size(); i++)
      {
         //std::cout<<this->DetectorCounts[i] << " / "<< b->fDetectorCounts[i] <<std::endl;
         if (b->fDetectorCounts[i])
            c->fDetectorCounts[i] = std::round(100.*(double)this->fDetectorCounts[i]/(double)b->fDetectorCounts[i]);
         else
            c->fDetectorCounts[i] = std::numeric_limits<int>::infinity();
      }
      if (b->VertexEvents)
         c->VertexEvents= 100*(double)this->VertexEvents / (double)b->VertexEvents;
      if (b->fVerticies)
         c->fVerticies   = 100*(double)this->fVerticies  / (double)b->fVerticies;
      if (b->fPassCuts)
         c->fPassCuts    = 100*(double)this->fPassCuts   / (double)b->fPassCuts;
      if (b->fPassMVA)
         c->fPassMVA     = 100*(double)this->fPassMVA    / (double)b->fPassMVA;
      return c;
   }
   template <class ScalerData>
   ScalerData* operator+(const ScalerData* b)
   {
      ScalerData* c=new ScalerData(this->fDetectorCounts.size());

      c->VertexFilled=this->VertexFilled;
      c->ScalerFilled=this->ScalerFilled;

      c->fStartTime =b->fStartTime;
      c->fStopTime  =this->fStopTime;

      for (size_t i=0; i<fDetectorCounts.size(); i++)
      {
         //std::cout<<this->DetectorCounts[i] << " / "<< b->DetectorCounts[i] <<std::endl;
         if (b->fDetectorCounts[i])
            c->fDetectorCounts[i]=this->fDetectorCounts[i]+b->fDetectorCounts[i];
         else
            c->fDetectorCounts[i]=std::numeric_limits<int>::infinity();
      }
      if (b->VertexEvents)
         c->VertexEvents= this->VertexEvents + b->VertexEvents;
      if (b->fVerticies)
         c->fVerticies = this->fVerticies  + b->fVerticies;
      if (b->fPassCuts)
         c->fPassCuts  = this->fPassCuts   + b->fPassCuts;
      if (b->fPassMVA)
        c->fPassMVA   = this->fPassMVA    + b->fPassMVA;
      return c;
   }
   bool Ready( bool have_vertex_detector);
   double GetDumpLength() { return fStopTime - fStartTime; };
   std::string ContentCSVTitle(std::vector<std::string> ChannelNames = {}) const;
   std::string ContentCSV() const;
   ClassDef(TSpillScalerData,1);
};

#endif