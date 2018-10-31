#include "RootUtils.h"


#ifndef _PrintTools_
#define _PrintTools_


void PrintSequences(int runNumber, int SeqNum=-1);
void PrintChronoBoards(int runNumber, Double_t tmin=0., Double_t tmax=-1.);
Int_t PrintTPCEvents(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);
Int_t PrintTPCEvents(Int_t runNumber,  const char* description, Int_t repetition=1, Int_t offset=0);
Int_t PrintSequenceQOD(Int_t runNumber);


#endif
