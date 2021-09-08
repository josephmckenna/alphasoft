#include "TSISPlot.h"

#ifdef BUILD_A2

ClassImp(TSISPlot)

TSISPlot::TSISPlot(bool zeroTime): TScalerPlot(zeroTime)
{
       
}

TSISPlot::~TSISPlot()
{

}


TString TSISPlot::GetListOfRuns()
{
   TString runsString="";
   std::sort(fRuns.begin(), fRuns.end());
   for (size_t i = 0; i < fRuns.size(); i++)
   {
      //std::cout <<"Run: "<<Runs[i] <<std::endl;
      if (i > 0)
         runsString += ",";
      runsString += fRuns[i];
   }
   return runsString;
}


void TSISPlot::AddDumpGates(int runNumber, std::vector<std::string> description, std::vector<int> repetition )
{
   std::vector<TA2Spill> spills=Get_A2_Spills(runNumber, description, repetition);
   return AddDumpGates(runNumber, spills);
}

void TSISPlot::AddDumpGates(int runNumber, std::vector<TA2Spill> spills)
{
   std::vector<double> minTime;
   std::vector<double> maxTime;
   
   for (auto & spill: spills)
   {
      if (spill.ScalerData)
      {
         minTime.push_back(spill.ScalerData->StartTime);
         maxTime.push_back(spill.ScalerData->StopTime);
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   if (fSetZeroTime)
      for (int i = 0; i< minTime.size() ; i++)
         AddTimeWindow(runNumber, minTime[i], maxTime[i],minTime[i]);
   else
      for (int i = 0; i< minTime.size() ; i++)
         AddTimeWindow(runNumber, minTime[i], maxTime[i],0.);
   
}
void TSISPlot::AddEvent(TSISEvent* event, int channel, double timeOffset)
{
   SISEvents.AddEvent(event->GetRunNumber(), event->GetRunTime() - timeOffset, 
      event->GetRunTime(), event->GetCountsInChannel(channel), channel);
}

void TSISPlot::AddSISEvent(TSISEvent* SISEvent)
{
   size_t numSISChannels=fSISChannels.size();
   double time=SISEvent->GetRunTime();

   //Loop over all time windows
   int index = GetValidWindowNumber(time);
   if(index>=0)
   {
      for (size_t i=0; i<numSISChannels; i++)
      {
         int counts = SISEvent->GetCountsInChannel(fSISChannels[i]);
         if (counts)
         {
            AddEvent(SISEvent, fSISChannels[i], fZeroTime[index]);
         }
      }
   }
}

//If spills are from one run, it is faster to call the function above
void TSISPlot::AddDumpGates(std::vector<TA2Spill> spills)
{
   for (TA2Spill& spill: spills)
   {
      if (spill.ScalerData)
      {
         if (fSetZeroTime)
            AddTimeWindow(spill.RunNumber, spill.GetStartTime(), spill.GetStopTime(),spill.GetStartTime());
         else
            AddTimeWindow(spill.RunNumber, spill.GetStartTime(), spill.GetStopTime(),0.);
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return;
}

void TSISPlot::LoadRun(int runNumber)
{
   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   
   //SetSISChannels(runNumber);?
   
   TTreeReader* SISReader=A2_SIS_Tree_Reader(runNumber);
   TTreeReaderValue<TSISEvent> SISEvent(*SISReader, "TSISEvent");
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (SISReader->Next())
   {
      double t = SISEvent->GetRunTime();
      
      //TODO! Add this optimisation, make it a member of TTimeWindows...
      //if (t < firstTime)
      //   continue;
      //if (t > lastTime)
      //   break;
      AddSISEvent(&(*SISEvent));
   }
}

void TSISPlot::LoadData()
{
   for (const int& run: fRuns)
      LoadRun(run);
}
#endif