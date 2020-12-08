#include "RootUtils.h"
#include "TTree.h"




#ifndef _DoubleGetter_
#define _DoubleGetter_
#ifdef BUILD_AG
Double_t GetTotalRunTimeFromChrono(Int_t runNumber, Int_t Board);
Double_t GetTotalRunTimeFromTPC(Int_t runNumber);
Double_t GetAGTotalRunTime(Int_t runNumber);
Double_t GetRunTimeOfChronoCount(Int_t runNumber, const char* ChannelName, Int_t repetition, Int_t offset=0);
Double_t GetRunTimeOfChronoCount(Int_t runNumber, Int_t Board, Int_t Channel, Int_t repetition=1, Int_t offset=0);
#endif
Double_t GetRunTimeOfEvent(Int_t runNumber, TSeq_Event* e, Int_t offset=0);

Double_t MatchEventToTime(Int_t runNumber,const char* description, const char* name, Int_t repetition=1, Int_t offset=0);//, Bool_t ExactMatch=true);
Double_t MatchEventToTime(Int_t runNumber,const char* description, Bool_t IsStart, Int_t repetition=1, Int_t offset=0);//, Bool_t ExactMatch)

#ifdef BUILD_AG

Double_t GetTrigTimeBefore(Int_t runNumber, Double_t mytime);
Double_t GetTrigTimeAfter(Int_t runNumber, Double_t mytime);
#endif
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
