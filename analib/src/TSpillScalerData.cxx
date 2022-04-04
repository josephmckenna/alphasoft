#include "TSpillScalerData.h"


ClassImp(TSpillScalerData)

TSpillScalerData::TSpillScalerData()
{
   fStartTime         =-1;
   fStopTime          =-1;
   FirstVertexEvent  =-1;
   LastVertexEvent   =-1;
   VertexEvents      =-1;
   fVerticies         =0;
   fPassCuts          =0;
   fPassMVA           =0;
   VertexFilled      =false;
}

TSpillScalerData::~TSpillScalerData()
{

}

TSpillScalerData::TSpillScalerData(int n_scaler_channels) : fDetectorCounts(n_scaler_channels,0),ScalerFilled(n_scaler_channels,0)
{
   fStartTime         =-1;
   fStopTime          =-1;
   FirstVertexEvent  =-1;
   LastVertexEvent   =-1;
   VertexEvents      =-1;
   fVerticies         =0;
   fPassCuts          =0;
   fPassMVA           =0;
   VertexFilled      =false;
}

TSpillScalerData::TSpillScalerData(const TSpillScalerData& a) : TObject(a)
{
   fStartTime         =a.fStartTime;
   fStopTime          =a.fStopTime;
   fDetectorCounts    =a.fDetectorCounts;
   ScalerFilled      =a.ScalerFilled;
   
   FirstVertexEvent  =a.FirstVertexEvent;
   LastVertexEvent   =a.LastVertexEvent;
   VertexEvents      =a.VertexEvents;
   fVerticies         =a.fVerticies;
   fPassCuts          =a.fPassCuts;
   fPassMVA           =a.fPassMVA;
   VertexFilled      =a.VertexFilled;
}

TSpillScalerData& TSpillScalerData::operator=(const TSpillScalerData& rhs)
{
   if (this == &rhs)
      return *this;
   fStartTime         =rhs.fStartTime;
   fStopTime          =rhs.fStopTime;
   fDetectorCounts    =rhs.fDetectorCounts;
   ScalerFilled      =rhs.ScalerFilled;
   
   FirstVertexEvent  =rhs.FirstVertexEvent;
   LastVertexEvent   =rhs.LastVertexEvent;
   VertexEvents      =rhs.VertexEvents;
   fVerticies         =rhs.fVerticies;
   fPassCuts          =rhs.fPassCuts;
   fPassMVA           =rhs.fPassMVA;
   VertexFilled      =rhs.VertexFilled;
   return *this;
}


bool TSpillScalerData::Ready(bool have_vertex_detector)
{
   bool SomeScalerFilled=false;
   for (auto chan: ScalerFilled)
   {
      if (chan)
      {
         SomeScalerFilled=true;
         break;
      }
   }
   if (fStartTime>0 &&
        fStopTime>0 &&
        //Some SIS channels filled test
       SomeScalerFilled )
   {  
      if ( VertexFilled || !have_vertex_detector)
         return true;
      //If there is no SVD data, then SVDFilled is false... 
      //wait for the SIS data to be atleast 'data_buffer_time' seconds
      //ahead to check there is no SVD data...
      //if ( !SVDFilled && T>StopTime+data_buffer_time )
      //   return true;
      return false;
   }
   else
   {
      return false;
   }
}


std::string TSpillScalerData::ContentCSVTitle(std::vector<std::string> ChannelNames) const
{
   std::string title = "Start Time (s),Stop Time (s),Durations (s),";
   if (ChannelNames.size() == fDetectorCounts.size())
   {
      for (size_t i=0; i<ChannelNames.size(); i++)
      {
         title += ChannelNames.at(i) + ",";
      }
   }
   else 
   {
      std::cout <<"Warning: " << __FILE__  << ":" << __LINE__ << "\tNo detector channel names given\n";
      for (size_t i=0; i<fDetectorCounts.size(); i++)
      {
         title += "Scaler Channel " + std::to_string(i) + ",";
      }
   }
   //This is very verbose... probably a better way of doing this
   title += "First Vertex Event,LastVertexEvent,Vertex Events,fVerticies,fPassCuts,fPassMVA,";
   return title;
}

std::string TSpillScalerData::ContentCSV() const
{
   std::string line;
   line += std::to_string(fStartTime) + "," +
           std::to_string(fStopTime) + "," +
           std::to_string(fStopTime - fStartTime) + ",";
   for (size_t i = 0; i<fDetectorCounts.size(); i++)
   {
      if (ScalerFilled.at(i))
         line += std::to_string(fDetectorCounts.at(i)) + ",";
      else
         line += "-1,";
   }
   //This is very verbose... probably a better way of doing this
   line += std::to_string(FirstVertexEvent) + ",";
   line += std::to_string(LastVertexEvent) + ",";
   line += std::to_string(VertexEvents) + ",";
   line += std::to_string(fVerticies) + ",";
   line += std::to_string(fPassCuts) + ",";
   line += std::to_string(fPassMVA) + ",";
   return line;
}