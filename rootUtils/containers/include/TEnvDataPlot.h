#ifndef __TEnvDataPlot__
#define __TEnvDataPlot__

#include "TObject.h"
#include "TGraph.h"
#include <vector>

//Generic feLabVIEW / feGEM data inside a time window
class TEnvDataPlot: public TObject
{
   public:
      std::vector<double> fTimes;
      std::vector<double> fRunTimes;
      std::vector<double> fData;
   public:
      TEnvDataPlot();
      ~TEnvDataPlot();
      TEnvDataPlot(const TEnvDataPlot& envDataPlot);
      TEnvDataPlot operator=(const TEnvDataPlot rhs);
      void AddPoint(double time, double runTime, double data);
      TGraph* GetGraph(bool zeroTime);
      
      ClassDef(TEnvDataPlot,1);
};

#endif
