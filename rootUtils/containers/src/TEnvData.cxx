#include "TEnvData.h"

ClassImp(TEnvData)

TEnvData::TEnvData()
{
   fArrayNumber = -1;
}

TEnvData::~TEnvData()
{

}

TEnvData::TEnvData(const TEnvData& envData) : TObject(envData)
{
   fVariableName = envData.fVariableName;
   fLabel = envData.fLabel;
   fArrayNumber = envData.fArrayNumber;
   for (TEnvDataPlot* plots: envData.fPlots)
      fPlots.push_back(new TEnvDataPlot(*plots));
}

TEnvData TEnvData::operator=(const TEnvData rhs)
{
   this->fVariableName = rhs.fVariableName;
   this->fLabel = rhs.fLabel;
   this->fArrayNumber = rhs.fArrayNumber;
   for (TEnvDataPlot* plots: rhs.fPlots)
      fPlots.push_back(new TEnvDataPlot(*plots));
   return *this;
}

std::pair<double,double> TEnvData::GetMinMax()
{
   double min = -1E99;
   double max = 1E99;
   for (auto& plot: fPlots)
   {
      for (double& data: plot->fData)
      {
         if (data < min)
            min = data;
         if (data > max )
            max = data;
      }
   }
   return std::pair<double,double>(min,max);
}

TEnvDataPlot* TEnvData::GetPlot(size_t index)
{
   while (index>=fPlots.size())
   {
      TEnvDataPlot* graph = new TEnvDataPlot();
      fPlots.push_back(graph);
   }
   return fPlots.at(index);
}

TGraph* TEnvData::BuildGraph(int index, bool zeroTime)
{
   TGraph* graph = GetPlot(index)->GetGraph(zeroTime);
   graph->SetNameTitle(
      GetVariable().c_str(),
      std::string( GetLabel() + "; t [s];").c_str()
   );
   return graph;
}
