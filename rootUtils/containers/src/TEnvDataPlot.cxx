#include "TEnvDataPlot.h"

ClassImp(TEnvDataPlot)

TEnvDataPlot::TEnvDataPlot()
{

}

TEnvDataPlot::~TEnvDataPlot()
{

}

TEnvDataPlot::TEnvDataPlot(const TEnvDataPlot& envDataPlot) : TObject(envDataPlot)
{
   fTimes = envDataPlot.fTimes;
   fRunTimes = envDataPlot.fRunTimes;
   fData = envDataPlot.fData;
}

TEnvDataPlot TEnvDataPlot::operator=(const TEnvDataPlot rhs)
{
   this->fTimes = rhs.fTimes;
   this->fRunTimes = rhs.fRunTimes;
   this->fData = rhs.fData;
   return *this;
}

void TEnvDataPlot::AddPoint(double time, double runTime, double data)
{
   fTimes.push_back(time);
   fRunTimes.push_back(runTime);
   fData.push_back(data);
}

TGraph* TEnvDataPlot::GetGraph(bool zeroTime)
{
   if (zeroTime)
      return new TGraph(fData.size(),fTimes.data(),fData.data());
   else
      return new TGraph(fData.size(),fRunTimes.data(),fData.data());
}
