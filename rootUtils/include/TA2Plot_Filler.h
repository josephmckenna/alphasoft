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
      CombinedName = TAPlot::CombinedName(Category,Varname);
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

class TA2Plot_Filler
{
private:
   std::vector<TA2Plot*> plots;
   std::vector<int>      runNumbers;
   std::vector<GEMChannel> GEMChannels;

   
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

template <typename T>
void LoadfeGEMData(feGEMdata& f, TA2Plot* p, TTreeReader* feGEMReader, const char* name, double first_time, double last_time);
void LoadfeGEMData(int runNumber, double first_time, double last_time);
void LoadData(int runNumber, double first_time, double last_time);
public:
   TA2Plot_Filler();
   ~TA2Plot_Filler();
   void BookPlot(TA2Plot* plot)
   {
      for (auto& no: plot->GetArrayOfRuns())
         AddRunNumber(no);
      for (auto& f: plot->feGEM)
         SetGEMChannel(f.GetName(),f.array_number);
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
      std::string name = TAPlot::CombinedName(Category,Varname);
      SetGEMChannel(name, ArrayEntry, title);
   }
   void LoadData();

};
#endif
#endif
