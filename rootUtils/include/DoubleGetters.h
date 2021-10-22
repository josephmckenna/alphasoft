#include "RootUtils.h"
#include "TTree.h"




#ifndef _DoubleGetter_
#define _DoubleGetter_

#include "TChronoChannelGetters.h"

#ifdef BUILD_AG
Double_t GetTotalRunTimeFromChrono(Int_t runNumber, Int_t Board);
Double_t GetTotalRunTimeFromTPC(Int_t runNumber);
Double_t GetAGTotalRunTime(Int_t runNumber);
Double_t GetRunTimeOfChronoCount(Int_t runNumber, const char* ChannelName, Int_t event_index);
Double_t GetRunTimeOfChronoCount(Int_t runNumber, TChronoChannel chan, Int_t event_index=0);
Double_t GetRunTimeOfEvent(Int_t runNumber, TSeq_Event* e, Int_t offset = 0);
#endif

//Double_t MatchEventToTime(Int_t runNumber,const char* description, const char* name, Int_t dumpIndex=0, Int_t offset=0);//, Bool_t ExactMatch=true);
//Double_t MatchEventToTime(Int_t runNumber,const char* description, Bool_t IsStart, Int_t dumpIndex=0, Int_t offset=0);//, Bool_t ExactMatch)

#ifdef BUILD_AG

Double_t GetTrigTimeBefore(Int_t runNumber, Double_t mytime);
Double_t GetTrigTimeAfter(Int_t runNumber, Double_t mytime);
#endif

#ifdef BUILD_A2
Double_t GetTotalRunTimeFromSIS(Int_t runNumber);
Double_t GetTotalRunTimeFromSVD(Int_t runNumber);
Double_t GetA2TotalRunTime(Int_t runNumber);
#endif

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
