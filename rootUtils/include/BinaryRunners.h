#ifndef _BinaryRunners_
#define _BinaryRunners_

#include "TSystem.h"
#include <iostream>
#include "IntGetters.h"

#ifdef BUILD_AG

#include "TAGSpillGetters.h"

void RunAGEventViewerInTime(Int_t runNumber, Double_t tmin, Double_t tmax);
void RunAGEventViewerInTime(Int_t runNumber,  const char* description, Int_t dumpIndex=0, Int_t offset=0);

#endif
void AnnounceOnSpeaker(Int_t runNumber, TString Phrase);
void AnnounceOnSpeaker(TString Phrase);
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
