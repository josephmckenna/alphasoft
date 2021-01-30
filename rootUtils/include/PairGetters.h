#ifndef _PAIRGETTERS_
#define _PAIRGETTERS_
#include "IntGetters.h"
#include <utility>

#if BUILD_AG
std::pair<Int_t,Int_t> GetChronoBoardChannel(Int_t runNumber, const char* ChannelName);

#endif

#if BUILD_A2
#include "TSISEvent.h"
std::vector<std::pair<double,int>> GetSISTimeAndCounts(Int_t runNumber, int SIS_Channel, std::vector<double> tmin, std::vector<double> tmax);
std::vector<std::pair<double,int>> GetSISTimeAndCounts(Int_t runNumber, const char* ChannelName, std::vector<double> tmin, std::vector<double> tmax);

#endif
#endif