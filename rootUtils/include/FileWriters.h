#ifndef _FILE_WRITERS_
#define _FILE_WRITERS_

#include "TreeGetters.h"
#include "TStoreGEMEvent.h"
#include <fstream>
std::string WriteTStoreGEMFile(TStoreGEMFile* file);
std::vector<std::string> DumpFilesSavedInMIDAS(Int_t runNumber, const char* category, const char* varname);

#ifdef BUILD_A2
#include "TA2Spill.h"
#include "TA2SpillGetters.h"
void DumpSpillLogsToCSV(std::vector<TA2Spill> dumps, std::string filename);
void DumpA2SpillLogToCSV(int runNumber);
#endif 

void DumpfeGEMDataToCSV(const int runNumber, const std::string category, const std::string varname, const double firstTime = 0., const double lastTime = 1E99);


#endif
