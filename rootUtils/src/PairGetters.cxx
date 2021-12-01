#include "PairGetters.h"

#ifdef BUILD_AG
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

#endif 


#ifdef BUILD_A2
std::vector<std::pair<double,int>> GetSISTimeAndCounts(Int_t runNumber, int SIS_Channel, std::vector<double> tmin, std::vector<double> tmax)
{
   assert(SIS_Channel > 0 );
   std::vector<std::pair<double,int>> TimeCounts;
   if (SIS_Channel<0)
   {
      std::cout<<"Unkown SIS channel!"<<std::endl;
      return TimeCounts;
   }
   
   //TimeCounts.reserve(1000000); //Ready for 1M results
   assert(tmin.size() == tmax.size());
   const int entries = tmin.size();

   //Set broad range we are looking for
   double first_time = *std::min_element( std::begin(tmin), std::end(tmin));
   double last_time = *std::max_element( std::begin(tmax), std::end(tmax));

   if(last_time < 0) last_time = GetTotalRunTimeFromSIS(runNumber);

   for (int sis_module_no = 0; sis_module_no < NUM_SIS_MODULES; sis_module_no++)
   {
      TTreeReader* SISReader = A2_SIS_Tree_Reader( runNumber, sis_module_no);
      TTreeReaderValue<TSISEvent> SISEvent(*SISReader, "TSISEvent");

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
                {
                   int counts = SISEvent->GetCountsInChannel(SIS_Channel);
                   if (counts)
                   {
                     TimeCounts.push_back(std::make_pair(t, counts));
                   }
                }
         }
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
    assert(SIS_Channel > 0 );
    std::vector<double> tmin;
    std::vector<double> tmax;
    tmin.reserve(spills.size());
    tmax.reserve(spills.size());
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

#endif


std::vector<std::pair<double,double>> GetLVData(Int_t runNumber, const char* BankName, int ArrayNo, double tmin, double tmax)
{
   std::vector<std::pair<double,double>> lvdata;
   TTreeReader* feLVReader=Get_feLV_Tree(runNumber,BankName);
   TTree* tree = feLVReader->GetTree();
   if  (!tree)
   {
      std::cout<<"Warning: " << BankName << " not found for run " << runNumber << std::endl;
      return lvdata;
   }
   if (tree->GetBranchStatus("TStoreLabVIEWEvent"))
   {
      TTreeReaderValue<TStoreLabVIEWEvent> LVEvent(*feLVReader, "TStoreLabVIEWEvent");
      // I assume that file IO is the slowest part of this function... 
      // so get multiple channels and multiple time windows in one pass
      while (feLVReader->Next())
      { 
         double t=LVEvent->GetRunTime();
         //A rough cut on the time window is very fast...
         if (t < tmin)
            continue;
         if (t > tmax)
            break;
         lvdata.push_back(
            std::make_pair(t, LVEvent->GetArrayEntry(ArrayNo))
            );
      }
   }
   return lvdata;
}
#ifdef BUILD_A2
std::vector<std::pair<double,double>> GetLVData(Int_t runNumber, const char* BankName, int ArrayNo, const TA2Spill& spill)
{
    return GetLVData(runNumber,BankName,ArrayNo,spill.GetStartTime(), spill.GetStopTime());
}
#endif
