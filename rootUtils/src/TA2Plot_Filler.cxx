#include "TA2Plot_Filler.h"

TA2Plot_Filler::TA2Plot_Filler()
{
}
TA2Plot_Filler::~TA2Plot_Filler()
{
}
void TA2Plot_Filler::LoadData(int runNumber, double first_time, double last_time)
{

   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   TTreeReader* SVDReader=Get_A2_SVD_Tree(runNumber);
   TTreeReaderValue<TSVD_QOD> SVDEvent(*SVDReader, "OfficalTime");
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (SVDReader->Next())
   {
      double t=SVDEvent->t;
      //A rough cut on the time window is very fast...
      if (t<first_time)
         continue;
      if (t>last_time)
         break;
      //Fill the plot, a fine time cut is done inside
      for (auto& p: plots)
         p->AddSVDEvent(&(*SVDEvent));
   }

   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   TTreeReader* SISReader=A2_SIS_Tree_Reader(runNumber);
   TTreeReaderValue<TSISEvent> SISEvent(*SISReader, "TSISEvent");
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   for (auto& p: plots)
      p->SetSISChannels(runNumber);
   while (SISReader->Next())
   {
      double t=SISEvent->GetRunTime();
      //A rough cut on the time window is very fast... 
      if (t<first_time)
         continue;
      if (t>last_time)
         break;
      //Fill the plot, a fine time cut is done inside
      for (auto& p: plots)
         p->AddSISEvent(&(*SISEvent));
   }
}
void TA2Plot_Filler::LoadData()
{
   std::vector<double> last_times(plots.size(),0.);
   std::vector<double> first_times(plots.size(),1E99);
   for (size_t i=0; i<plots.size(); i++)
   {
      //Calculate our list time... so we can stop early
      for (auto& t: plots[i]->GetTimeWindows())
      {
         if (t.runNumber==runNumbers[i])
         {
            if (t.tmax<0) last_times[i]=1E99;
            if (last_times[i]<t.tmax)
            {
               last_times[i]=t.tmax;
            }
            if (first_times[i]<t.tmin)
            {
               first_times[i]=t.tmin;
            }
         }
      }
   }


   for (size_t i=0; i<runNumbers.size(); i++)
   {
      LoadData(runNumbers[i],first_times[i],last_times[i]);
   }
}

