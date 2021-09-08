
#ifndef _TScalerPlot_
#define _TScalerPlot_

#include "TTimeWindows.h"
#include "TSISPlotEvents.h"

class TScalerPlot: public TTimeWindows
{
    protected:

      std::vector<int> fRuns; //check dupes - ignore copies. AddRunNumber       
      //TTimeWindows fTimeWindows; Now inherited
      const bool fSetZeroTime;

    public:
    
       TScalerPlot(bool zeroTime = true);
       ~TScalerPlot();
       
       //We need add operators etc
       
    // I'd like basic operators
    ClassDef(TScalerPlot,1);
};

#endif