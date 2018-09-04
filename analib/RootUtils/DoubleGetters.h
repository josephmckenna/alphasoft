#include "RootUtils.h"
#include "TTree.h"




#ifndef _DoubleGetter_
#define _DoubleGetter_
Double_t GetTotalRunTime(Int_t runNumber);
Double_t GetRunTimeOfCount(Int_t runNumber, const char* ChannelName, Int_t repetition, Int_t offset=0);
Double_t GetRunTimeOfCount(Int_t runNumber, Int_t Board, Int_t Channel, Int_t repetition=1, Int_t offset=0);

Double_t GetRunTimeOfEvent(Int_t runNumber, TSeq_Event* e, Int_t offset=0);
Double_t MatchEventToTime(Int_t runNumber,const char* description, const char* name, Int_t repetition=1, Int_t offset=0);//, Bool_t ExactMatch=true);
Double_t MatchEventToTime(Int_t runNumber,const char* description, Bool_t IsStart, Int_t repetition=1, Int_t offset=0);//, Bool_t ExactMatch)
#endif
