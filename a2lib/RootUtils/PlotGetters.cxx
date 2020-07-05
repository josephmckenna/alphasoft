#include "PlotGetters.h"


void Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax)
{
   std::vector<TH1D*> hh=Get_SIS(runNumber, SIS_Channel,tmin, tmax);
   for (size_t i=0; i<hh.size(); i++)
      hh[i]->Draw();
}
void Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<TA2Spill*> spills)
{
   std::vector<TH1D*> hh=Get_SIS(runNumber, SIS_Channel,spills);
   for (size_t i=0; i<hh.size(); i++)
      hh[i]->Draw();
}
void Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<std::string> description, std::vector<int> repetition)
{
   std::vector<TA2Spill*> s=Get_A2_Spills(runNumber,description,repetition);
   
   std::vector<TH1D*> hh=Get_SIS(runNumber,SIS_Channel,s);
   for (size_t i=0; i<hh.size(); i++)
      hh[i]->Draw();
}

