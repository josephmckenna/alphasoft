#include "RootUtils.h"
#include "TSystem.h"


#ifndef _BinaryRunners_
#define _BinaryRunners_

#ifdef BUILD_AG

void RunAGEventViewerInTime(Int_t runNumber, Double_t tmin, Double_t tmax);
void RunAGEventViewerInTime(Int_t runNumber,  const char* description, Int_t repetition=1, Int_t offset=0);

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
