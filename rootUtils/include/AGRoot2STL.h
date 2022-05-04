#ifndef _AGROOT2STL_
#define _AGROOT2STL_



#include <vector>

#ifdef BUILD_AG_SIM

#include "TClonesArray.h"
#include "SignalsType.hh"
#include "TWaveform.hh"

std::vector<ALPHAg::wf_ref> ConvertWaveformArray(TClonesArray *wfarray);
#endif

#endif
