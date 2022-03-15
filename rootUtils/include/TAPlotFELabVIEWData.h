#ifndef _TAPLOTFELABVIEWDATA_
#define _TAPLOTFELABVIEWDATA_



//Specialise the above for feLabVIEW
class TAPlotFELabVIEWData: public TAPlotEnvData
{
   public:
      void AddLVEvent(int runNumber, TStoreLabVIEWEvent* labviewEvent, TAPlotTimeWindows& timeWindows)
      {
         double time=labviewEvent->GetRunTime();
         //O^2 complexity atleast... There isn't usually allot of feGEM data so maybe we can live with this...?
         //Hopefully now better than On^2
         int index = timeWindows.GetValidWindowNumber(time);
         if(index>=0)
         {
            TAPlotEnvDataPlot* plot = GetPlot(index);
            plot->AddPoint( time - timeWindows.fZeroTime[index], time, labviewEvent->GetArrayEntry(fArrayNumber));
         }
         return;
      }
      ClassDef(TAPlotFELabVIEWData, 1);
};

#endif