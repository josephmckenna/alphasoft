#ifndef __TEnvData__
#define __TEnvData__

#include "TEnvDataPlot.h"

//Collection of feLabVIEW / feGEM data with the same name (the same source)
class TEnvData: public TObject 
{
   public:
      std::string fVariableName;
      std::string fLabel;
      int fArrayNumber;
   private:
      std::vector<TEnvDataPlot*> fPlots;
   public:
      std::string GetVariable() { return fVariableName + "[" + std::to_string(fArrayNumber) + "]";}
      std::string GetVariableID() { return fVariableName + "_" + std::to_string(fArrayNumber);}
      std::string GetLabel() { return fLabel; }
      TEnvData();
      ~TEnvData();
      TEnvData(const TEnvData& envData);
      TEnvData operator=(const TEnvData rhs);
      std::pair<double,double> GetMinMax();
      TEnvDataPlot* GetPlot(size_t index);
      TGraph* BuildGraph(int index, bool zeroTime);
   ClassDef(TEnvData,1);
};

#endif
