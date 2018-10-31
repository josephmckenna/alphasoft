#include "RootUtils.h"
#include "TSystem.h"


#ifndef _BinaryRunners_
#define _BinaryRunners_

void RunEventViewerInTime(Int_t runNumber, Double_t tmin, Double_t tmax);
void RunEventViewerInTime(Int_t runNumber,  const char* description, Int_t repetition=1, Int_t offset=0);
#endif
