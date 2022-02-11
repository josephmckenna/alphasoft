#ifndef _TStringGetters_
#define _TStringGetters_

#include "TSeq_Event.h"
#include "TreeGetters.h"
#include "Sequencer_Channels.h"
#include "IntGetters.h"
#include "BoolGetters.h"

#ifdef BUILD_AG
#include "TChronoChannel.h"
#include "TChronoChannelName.h"
TString Get_Chrono_Name(Int_t runNumber, TChronoChannel chan);
TString Get_Chrono_Name(TSeq_Event* e);
TString SequenceAGQODDetectorLine(Int_t runNumber,Double_t tmin, Double_t tmax, Int_t* boards[], Int_t* channels[], Int_t nChannels);
#endif

#ifdef BUILD_A2
#include "TSISChannel.h"
TString Get_SIS_Name(Int_t runNumber, Int_t SIS_Channel);
#endif

TString MakeAutoPlotsFolder(TString subFolder);
TString MakeAutoPlotsFolder(TString subFolder,TString rootdir);


#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
