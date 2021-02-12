#ifndef _FILE_WRITERS_
#define _FILE_WRITERS_

#include "TreeGetters.h"
#include "TStoreGEMEvent.h"
void WriteTStoreGEMFile(TStoreGEMFile* file);
void DumpFilesSavedInMIDAS(Int_t runNumber, const char* category, const char* varname);











#endif