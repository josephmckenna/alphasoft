#ifdef BUILD_A2
#include "TA2Plot_Filler.h"

TA2Plot_Filler::TA2Plot_Filler()
{
}
TA2Plot_Filler::~TA2Plot_Filler()
{
}

void TA2Plot_Filler::LoadfeGEMData(int runNumber, double first_time, double last_time)
{ 
   //feGEM isn't grouped... we could gain speed by reading only once and filling groups of feGEM
   //Fill the plot, a fine time cut is done inside

   for (TA2Plot* p: plots)
   {
      //If current runNumber isn't in plot... skip
      if ( std::find(p->GetArrayOfRuns().begin(), p->GetArrayOfRuns().end(), runNumber) == p->GetArrayOfRuns().end())
         continue;
      p->LoadfeGEMData(runNumber,first_time, last_time);
   }
}

void TA2Plot_Filler::LoadfeLVData(int runNumber, double first_time, double last_time)
{
   for (TA2Plot* p: plots)
   {
      //If current runNumber isn't in plot... skip
      if ( std::find(p->GetArrayOfRuns().begin(), p->GetArrayOfRuns().end(), runNumber) == p->GetArrayOfRuns().end())
         continue;
      p->LoadfeLVData(runNumber, first_time, last_time);
   }
}

void TA2Plot_Filler::LoadData(int runNumber, double first_time, double last_time)
{
   std::cout<<"TA2Plot_Filler Loading run data:"<<runNumber <<" over the range between "<< first_time << "s to "<< last_time << "s"<< std::endl;
   AlphaColourWheel colours;
   /*for (TA2Plot* p: plots)
   {
      for (size_t i=0; i<p->GetTimeWindows().size(); i++)
      {
         for (auto& f: p->feGEM)
         {
            for (auto& g: f.plots)
            {
               TGraph* graph = g->GetGraph();
               graph->SetLineColor(colours.GetNewColour());
               graph->SetNameTitle(f.GetName().c_str(),std::string( f.GetTitle() + "; t [s];").c_str());
               feGEMmg->Add(graph);
            }
         } 
         for (auto& f: p->feLV)
         {
            for (auto& g: f.plots)
            {
               TGraph* graph = g->GetGraph();
               graph->SetLineColor(colours.GetNewColour());
               graph->SetNameTitle(f.GetName().c_str(),std::string( f.GetTitle() + "; t [s];").c_str());
               feLVmg->Add(graph);
            }
         }
      }
   }*/
   LoadfeGEMData(runNumber, first_time, last_time);
   LoadfeLVData(runNumber, first_time, last_time);

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
   std::set<int> Runs;
   for (auto &plot: plots)
      for (auto& run : plot->GetArrayOfRuns())
         Runs.insert(run);
   std::vector<int> UniqueRuns(Runs.begin(),Runs.end());
   Runs.clear();
   for (auto &plot: plots)
   {
      for (auto& c: GEMChannels)
      {
         plot->SetGEMChannel(c.CombinedName,c.ArrayEntry,c.title);
      }
      for (auto& c: LVChannels)
      {
         plot->SetLVChannel(c.BankName,c.ArrayEntry,c.title);
      }
   }
   std::vector<double> last_times(UniqueRuns.size(),0.);
   std::vector<double> first_times(UniqueRuns.size(),1E99);
   for (auto& plot: plots)
   {
      //Calculate our list time... so we can stop early
      for (auto& t: plot->GetTimeWindows())
      {
         for (size_t i=0; i<UniqueRuns.size(); i++)
         if (t.runNumber==runNumbers[i])
         {
            if (t.tmax<0) last_times[i]=1E99;
            if (last_times[i]<t.tmax)
            {
               last_times[i]=t.tmax;
            }
            if (first_times[i]>t.tmin)
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
   //Give the TAPlots a timestamp so we can track how long processing has taken
   for (auto& plot: plots)
   {
      plot->LoadingDataLoadingDone();
   }
}

#endif
