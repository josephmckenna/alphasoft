#include "RootUtils.h"
#ifndef _A2TH1DGetters_
#define _A2TH1DGetters_
#include "TH1D.h"
#include "TA2SpillGetters.h"

//ALPHA2
std::vector<TH1D*> Get_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax, double range = -1);
std::vector<TH1D*> Get_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<TA2Spill*> spills);


#endif
