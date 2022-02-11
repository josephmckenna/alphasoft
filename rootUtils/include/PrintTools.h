#ifndef _PrintTools_
#define _PrintTools_
#include "PairGetters.h"
#include "TStringGetters.h"
#include "BoolGetters.h"
#include "TSeq_Event.h"
#include "TreeGetters.h"

#ifdef BUILD_AG
void PrintSequences(int runNumber, int SeqNum=-1);
void PrintChronoBoards(int runNumber, Double_t tmin=0., Double_t tmax=-1.);
Int_t PrintTPCEvents(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);
Int_t PrintTPCEvents(Int_t runNumber,  const char* description, Int_t dumpIndex=0, Int_t offset=0);
Int_t PrintAGSequenceQOD(Int_t runNumber);
#endif


#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
