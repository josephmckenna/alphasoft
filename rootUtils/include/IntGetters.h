#ifndef _IntGetters_
#define _IntGetters_
#include "TChronoChannel.h"
#include "TChronoChannelName.h"
#if BUILD_AG
#include "TStoreEvent.hh"
#endif
#include "TreeGetters.h"
#include "DoubleGetters.h"

#if BUILD_AG
Int_t Get_Chrono_Channel_In_Board(Int_t runNumber, const std::string& ChronoBoard, const char* ChannelName, Bool_t ExactMatch=kFALSE);
Int_t GetCountsInChannel(Int_t runNumber,  TChronoChannel channel, Double_t tmin=0., Double_t tmax=-1.);
Int_t GetCountsInChannel(Int_t runNumber,  const char* ChannelName, Double_t tmin=0., Double_t tmax=-1.);
Int_t ApplyCuts(TStoreEvent* e);

Int_t GetTPCEventNoBeforeOfficialTime(Double_t runNumber, Double_t tmin);
Int_t GetTPCEventNoBeforeDump(Double_t runNumber, const char* description, Int_t dumpIndex=0, Int_t offset=0);
Int_t GetTPCEventNoAfterDump(Double_t runNumber, const char* description, Int_t dumpIndex=0, Int_t offset=0);
#endif

#if BUILD_A2

int Count_SIS_Triggers(int runNumber, TSISChannel ch, std::vector<double> tmin, std::vector<double> tmax);

#endif
//*************************************************************
// Energy Analysis
//*************************************************************

Int_t LoadRampFile(const char* filename, Double_t* x, Double_t* y);
//*************************************************************

int GetRunNumber( TString fname );
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
