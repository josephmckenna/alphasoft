#include "RootUtils.h"
//#include "Rtypes.h"
#include <sys/stat.h>

#ifndef _BoolGetters_
#define _BoolGetters_
#include "TChronoChannelGetters.h"
#ifdef BUILD_AG
Bool_t ChronoboxesHaveChannel(Int_t runNumber, const char* Name);
#endif
Bool_t IsPathExist(const TString &s);
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
