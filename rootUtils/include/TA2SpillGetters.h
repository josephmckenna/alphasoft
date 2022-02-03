
#ifdef BUILD_A2
#include "RootUtils.h"
#ifndef _TA2SpillGetters_
#define _TA2SpillGetters_
#include <vector>
#include "TA2Spill.h"
#include "TreeGetters.h"

std::vector<TA2Spill> Get_A2_Spills(int runNumber, std::vector<std::string> description, std::vector<int> dumpIndex);
std::vector<TA2Spill> Get_All_A2_Spills(int runNumber);

#endif

#endif
