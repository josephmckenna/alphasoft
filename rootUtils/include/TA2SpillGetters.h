
#ifdef BUILD_A2
#include "RootUtils.h"
#ifndef _TA2SpillGetters_
#define _TA2SpillGetters_
#include <vector>
#include "TSpill.h"
#include "TreeGetters.h"
#include "PythonTools.h"

std::vector<TA2Spill> Get_A2_Spills(int runNumber, std::vector<std::string> description, std::vector<int> repetition);
#ifdef HAVE_PYTHON
//Wrapper for above function...
std::vector<TA2Spill> Get_A2_Spills(int runNumber, PyObject* description, PyObject* repetition);
#endif



#endif

#endif
