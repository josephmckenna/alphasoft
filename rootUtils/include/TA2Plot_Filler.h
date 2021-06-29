#ifdef BUILD_A2
#ifndef _TA2Plot_Filler_
#define _TA2Plot_Filler_


#include "TA2Plot.h"
#include <set>
class GEMChannel
{
   public:
      std::string CombinedName;
      int ArrayEntry;
      std::string title;
   GEMChannel(const std::string& Category, const std::string& Varname, int _ArrayEntry, const std::string& _title)
   {
      CombinedName = TFEGEMData::CombinedName(Category,Varname);
      ArrayEntry =_ArrayEntry;
      title      =_title;
   }
   GEMChannel(const std::string& _CombinedName, int _ArrayEntry, const std::string& _title)
   {
      CombinedName = _CombinedName;
      ArrayEntry =_ArrayEntry;
      title      =_title;
   }
};

class LVChannel
{
   public:
      std::string BankName;
      int ArrayEntry;
      std::string title;
   LVChannel(const std::string& _BankName, int _ArrayEntry, const std::string& _title)
   {
      BankName   =_BankName;
      ArrayEntry =_ArrayEntry;
      title      =_title;
   }
};

class TA2Plot_Filler
{
private:
   std::vector<TA2Plot*> plots;
   std::vector<int>      runNumbers;
   std::vector<GEMChannel> GEMChannels;
   std::vector<LVChannel>  LVChannels;

   
   void AddRunNumber(int runNumber)
   {
      for (auto& no: runNumbers)
      {
         if (no==runNumber)
         {
            return;
         }
      }
      //std::cout<<"Add run number:"<<runNumber<<std::endl;
      runNumbers.push_back(runNumber);
   }

void LoadfeGEMData(int runNumber, double first_time, double last_time);
void LoadfeLVData(int runNumber, double first_time, double last_time);

void LoadData(int runNumber, double first_time, double last_time);
public:
   TA2Plot_Filler();
   ~TA2Plot_Filler();
   void BookPlot(TA2Plot* plot)
   {
      for (auto& no: plot->GetArrayOfRuns())
         AddRunNumber(no);
      for (auto& f: plot->GetGEMChannels())
         SetGEMChannel(f.first,f.second);
      for (auto& f: plot->GetLVChannels())
         SetLVChannel(f.first,f.second);
      for (TA2Plot* p: plots)
      {
          if (p==plot)
          {
             std::cout<<"Plot already booked"<<std::endl;
             return;
          }
      }
      plots.push_back(plot);
   }
   void SetGEMChannel(const std::string& CombinedName, int ArrayEntry, std::string title="")
   {
    for (auto& g: GEMChannels)
      {
         if (g.ArrayEntry == ArrayEntry)
         {
            if (g.CombinedName == CombinedName)
            {
               if (title.size() > g.title.size())
               {
                  //The latest SetGEMChannel may overwrite a new title
                  g.title = title;
               }
               return;
            }
         }
      }
      GEMChannels.push_back(GEMChannel(CombinedName,ArrayEntry,title));
   }
   void SetGEMChannel(const std::string& Category, const std::string& Varname, int ArrayEntry, std::string title="")
   {
      std::string name = TFEGEMData::CombinedName(Category,Varname);
      SetGEMChannel(name, ArrayEntry, title);
   }

   void SetLVChannel(const std::string& BankName, int ArrayEntry, std::string title="")
   {
      for (auto& g: LVChannels)
      {
         if (g.ArrayEntry == ArrayEntry)
         {
            if (g.BankName == BankName)
            {
               if (title.size() > g.title.size())
               {
                  //The latest SetGEMChannel may overwrite a new title
                  g.title = title;
               }
               return;
            }
         }
      }
      LVChannels.push_back(LVChannel(BankName,ArrayEntry,title));
   }

   void LoadData();

};
#endif
#endif
