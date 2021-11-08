
#ifndef _TChronoChannelGetters_
#define _TChronoChannelGetters_
#include "TChronoChannel.h"

#ifdef BUILD_AG
TChronoChannel Get_Chrono_Channel(Int_t runNumber, const char* ChannelName, Bool_t ExactMatch = kFALSE);

#endif

#endif
