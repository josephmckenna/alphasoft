
#ifdef BUILD_AG
#ifndef _TAGSpillGetters_
#define _TAGSpillGetters_
#include <vector>
#include "TAGSpill.h"
#include "TreeGetters.h"

std::vector<TAGSpill> Get_AG_Spills(int runNumber, std::vector<std::string> description, std::vector<int> repetition);
std::vector<TAGSpill> Get_All_AG_Spills(int runNumber);

#endif

#endif
