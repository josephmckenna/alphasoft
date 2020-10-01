#ifndef _TA2Plot_Filler_
#define _TA2Plot_Filler_


#include "TA2Plot.h"
class TA2Plot_Filler
{
private:
   std::vector<TA2Plot*> plots;
   std::vector<int>      runNumbers;
   void AddRunNumber(int runNumber)
   {
      for (auto& no: runNumbers)
      {
         if (no==runNumber)
         {
            return;
         }
      }
      runNumbers.push_back(runNumber);
   }
   void LoadData(int runNumber, double first_time, double last_time);
public:
   TA2Plot_Filler();
   ~TA2Plot_Filler();
   void BookPlot(TA2Plot* plot)
   {
      for (auto& no: plot->GetArrayOfRuns())
         AddRunNumber(no);
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
   void LoadData();

};
#endif
