#include "RootUtils.h"
#include "TSystem.h"


#ifndef _BinaryRunners_
#define _BinaryRunners_

void RunEventViewerInTime(Int_t runNumber, Double_t tmin, Double_t tmax);
void RunEventViewerInTime(Int_t runNumber,  const char* description, Int_t repetition=1, Int_t offset=0);
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
