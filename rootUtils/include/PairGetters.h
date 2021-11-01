#ifndef _PAIRGETTERS_
#define _PAIRGETTERS_
#include "IntGetters.h"
#include <utility>



#ifdef BUILD_A2
#include "TSISEvent.h"
std::vector<std::pair<double,int>> GetSISTimeAndCounts(Int_t runNumber, int SIS_Channel,         std::vector<double> tmin, std::vector<double> tmax);
std::vector<std::pair<double,int>> GetSISTimeAndCounts(Int_t runNumber, const char* ChannelName, std::vector<double> tmin, std::vector<double> tmax);
std::vector<std::pair<double,int>> GetSISTimeAndCounts(Int_t runNumber, int SIS_Channel,         const std::vector<TA2Spill>& spills);
std::vector<std::pair<double,int>> GetSISTimeAndCounts(Int_t runNumber, const char* ChannelName, const std::vector<TA2Spill>& spills);

#endif


std::vector<std::pair<double,double>> GetLVData(Int_t runNumber, const char* BankName, int ArrayNo, double tmin, double tmax);
#ifdef BUILD_A2
std::vector<std::pair<double,double>> GetLVData(Int_t runNumber, const char* BankName, int ArrayNo, const TA2Spill& spill);
#endif
#endif