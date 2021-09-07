#ifndef __TFEGEMData__
#define __TFEGEMData__


#include "TEnvData.h"

#include "TStoreGEMEvent.h"
#include "TTimeWindows.h"

//Specialise the above for feGEM
class TFEGEMData: public TEnvData
{
   public:
      TFEGEMData();
      ~TFEGEMData();
      template<typename T>
      void AddGEMEvent(TStoreGEMData<T>* gemEvent, TTimeWindows& timeWindows)
      {
         double time = gemEvent->GetRunTime();
         //O^2 complexity atleast... There isn't usually allot of feGEM data so maybe we can live with this...?
         //Hopefully now better than On^2
         int index = timeWindows.GetValidWindowNumber(time);
         if(index>=0)
         {
            TEnvDataPlot* plot = GetPlot(index);
            plot->AddPoint(time - timeWindows.fZeroTime[index], time, (double)gemEvent->GetArrayEntry(fArrayNumber));
         }
         return;
      }

      static std::string CombinedName(const std::string& category, const std::string& varName)
      {
         return std::string(category + "\\" + varName);
      }
      
   ClassDef(TFEGEMData,1);
};


#endif
