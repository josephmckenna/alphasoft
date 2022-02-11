#ifndef _TAPlotEnvData_
#define _TAPlotEnvData_


//Collection of feLabVIEW / feGEM data with the same name (the same source)
class TAPlotEnvData: public TObject 
{
   public:
      std::string fVariableName;
      std::string fLabel;
      int fArrayNumber;
   private:
      std::vector<TAPlotEnvDataPlot*> fPlots;
   public:
      std::string GetVariable() { return fVariableName + "[" + std::to_string(fArrayNumber) + "]";}
      std::string GetVariableID() { return fVariableName + "_" + std::to_string(fArrayNumber);}
      std::string GetLabel() { return fLabel; }
      TAPlotEnvData()
      {
         fArrayNumber = -1;
      }
      ~TAPlotEnvData()
      {
      }
      TAPlotEnvData(const TAPlotEnvData& envData) : TObject(envData)
      {
         fVariableName = envData.fVariableName;
         fLabel = envData.fLabel;
         fArrayNumber = envData.fArrayNumber;
         for (TAPlotEnvDataPlot* plots: envData.fPlots)
            fPlots.push_back(new TAPlotEnvDataPlot(*plots));

      }
      TAPlotEnvData operator=(const TAPlotEnvData rhs)
      {
         this->fVariableName = rhs.fVariableName;
         this->fLabel = rhs.fLabel;
         this->fArrayNumber = rhs.fArrayNumber;
         for (TAPlotEnvDataPlot* plots: rhs.fPlots)
            fPlots.push_back(new TAPlotEnvDataPlot(*plots));
         return *this;
      }
      std::pair<double,double> GetMinMax()
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
      TAPlotEnvDataPlot* GetPlot(size_t index)
      {
         while (index>=fPlots.size())
         {
            TAPlotEnvDataPlot* graph = new TAPlotEnvDataPlot();
            fPlots.push_back(graph);
         }
         return fPlots.at(index);
      }
      TGraph* BuildGraph(int index, bool zeroTime)
      {
         TGraph* graph = GetPlot(index)->GetGraph(zeroTime);
         graph->SetNameTitle(
            GetVariable().c_str(),
            std::string( GetLabel() + "; t [s];").c_str()
            );
         return graph;
      }
      ClassDef(TAPlotEnvData, 1);
};

#endif