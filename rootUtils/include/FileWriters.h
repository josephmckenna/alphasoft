#ifndef _FILE_WRITERS_
#define _FILE_WRITERS_

#include "TreeGetters.h"
#include "TStoreGEMEvent.h"
std::string WriteTStoreGEMFile(TStoreGEMFile* file);
std::vector<std::string> DumpFilesSavedInMIDAS(Int_t runNumber, const char* category, const char* varname);











#endif