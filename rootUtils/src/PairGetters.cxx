#include "PairGetters.h"

std::pair<Int_t,Int_t> GetChronoBoardChannel(Int_t runNumber, const char* ChannelName)
{
   Int_t chan=-1;
   Int_t board=-1;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
       chan=Get_Chrono_Channel(runNumber, board, ChannelName);
       if (chan>-1) break;
   }
   return {board,chan};
}


std::vector<std::pair<double,int>> GetSISTimeAndCounts(Int_t runNumber, int SIS_Channel, std::vector<double> tmin, std::vector<double> tmax)
{
   std::vector<std::pair<double,int>> TimeCounts;

   assert(tmin.size() == tmax.size());
   const int entries = tmin.size();

   TTreeReader* SISReader=A2_SIS_Tree_Reader( runNumber);
   TTreeReaderValue<TSISEvent> SISEvent(*SISReader, "TSISEvent");
   //Set broad range we are looking for
   double first_time = *std::min_element( std::begin(tmin), std::end(tmin));
   double last_time = *std::max_element( std::begin(tmax), std::end(tmax));

   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (SISReader->Next())
   {
      double t = SISEvent->GetRunTime();
      if (t < first_time)
         continue;
      if (t > last_time)
         break;
      for (int i=0; i<entries; i++)
      {
          if (t > tmin[i])
             if (t < tmax[i])
                TimeCounts.push_back(
                    {t, SISEvent->GetCountsInChannel(SIS_Channel)}
                    );
      }
   }
   return TimeCounts;
}

std::vector<std::pair<double,int>> GetSISTimeAndCounts(Int_t runNumber, const char* ChannelName, std::vector<double> tmin, std::vector<double> tmax)
{
   return GetSISTimeAndCounts(runNumber,GetSISChannel(runNumber,ChannelName),tmin,tmax);
}

std::vector<std::pair<double,int>> GetSISTimeAndCounts(Int_t runNumber, int SIS_Channel, const std::vector<TA2Spill>& spills)
{
    std::vector<double> tmin;
    std::vector<double> tmax;
    for (auto& s: spills)
    {
        tmin.push_back(s.GetStartTime());
        tmax.push_back(s.GetStopTime());
    }
    return GetSISTimeAndCounts(runNumber, SIS_Channel, tmin, tmax);
}

std::vector<std::pair<double,int>> GetSISTimeAndCounts(Int_t runNumber, const char* ChannelName, const std::vector<TA2Spill>& spills)
{
    return GetSISTimeAndCounts(runNumber,GetSISChannel(runNumber,ChannelName),spills);
}