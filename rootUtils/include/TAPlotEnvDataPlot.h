#ifndef _TEnvDataPlot_
#define _TEnvDataPlot_


//Generic feLabVIEW / feGEM data inside a time window
class TAPlotEnvDataPlot: public TObject
{
   public:
      std::vector<double> fTimes;
      std::vector<double> fRunTimes;
      std::vector<double> fData;
   public:
      TAPlotEnvDataPlot()
      {
      }
      ~TAPlotEnvDataPlot()
      {
      }
      TAPlotEnvDataPlot(const TAPlotEnvDataPlot& envDataPlot) : TObject(envDataPlot)
      {
         fTimes = envDataPlot.fTimes;
         fRunTimes = envDataPlot.fRunTimes;
         fData = envDataPlot.fData;
      }
      TAPlotEnvDataPlot operator=(const TAPlotEnvDataPlot rhs)
      {
         this->fTimes = rhs.fTimes;
         this->fRunTimes = rhs.fRunTimes;
         this->fData = rhs.fData;

         return *this;
      }
      void AddPoint(double time, double runTime, double data)
      {
         fTimes.push_back(time);
         fRunTimes.push_back(runTime);
         fData.push_back(data);
      }
      TGraph* GetGraph(bool zeroTime)
      {
         if (zeroTime)
            return new TGraph(fData.size(),fTimes.data(),fData.data());
         else
            return new TGraph(fData.size(),fRunTimes.data(),fData.data());
      }
      ClassDef(TAPlotEnvDataPlot, 1);
};

#endif