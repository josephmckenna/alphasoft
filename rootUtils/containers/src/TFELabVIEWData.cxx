#include "TFELabVIEWData.h"

ClassImp(TFELabVIEWData)
TFELabVIEWData::TFELabVIEWData()
{

}
TFELabVIEWData::~TFELabVIEWData()
{
   
}

void TFELabVIEWData::AddLVEvent(TStoreLabVIEWEvent* labviewEvent, TTimeWindows& timeWindows)
{
   double time=labviewEvent->GetRunTime();
   //O^2 complexity atleast... There isn't usually allot of feGEM data so maybe we can live with this...?
   //Hopefully now better than On^2
   int index = timeWindows.GetValidWindowNumber(time);
   if(index>=0)
   {
      TEnvDataPlot* plot = GetPlot(index);
      plot->AddPoint( time - timeWindows.fZeroTime[index], time, labviewEvent->GetArrayEntry(fArrayNumber));
   }
   return;
}
