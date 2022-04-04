#ifndef _TAPlotFEGEMDATA_
#define _TAPlotFEGEMDATA_

#include "TStoreGEMEvent.h"

//Specialise the above for feGEM
class TAPlotFEGEMData: public TAPlotEnvData
{
   public:
      template<typename T>
      void AddGEMEvent(TStoreGEMData<T>* gemEvent, TAPlotTimeWindows& timeWindows)
      {
         double time = gemEvent->GetRunTime();
         //O^2 complexity atleast... There isn't usually allot of feGEM data so maybe we can live with this...?
         //Hopefully now better than On^2
         int index = timeWindows.GetValidWindowNumber(time);
         if(index>=0)
         {
            TAPlotEnvDataPlot* plot = GetPlot(index);
            plot->AddPoint(time - timeWindows.fZeroTime[index], time, (double)gemEvent->GetArrayEntry(fArrayNumber));
         }
         return;
      }
      static std::string CombinedName(const std::string& category, const std::string& varName)
      {
         return std::string(category + "\\" + varName);
      }
      ClassDef(TAPlotFEGEMData, 1);
};


#endif