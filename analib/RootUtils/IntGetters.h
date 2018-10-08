#include "RootUtils.h"

#ifndef _IntGetters_
#define _IntGetters_



Int_t Get_Chrono_Channel(Int_t runNumber, Int_t ChronoBoard, const char* ChannelName, Bool_t ExactMatch=kFALSE);
Int_t GetCountsInChannel(Int_t runNumber,  Int_t ChronoBoard, Int_t Channel, Double_t tmin=0., Double_t tmax=-1.);
Int_t GetCountsInChannel(Int_t runNumber,  const char* ChannelName, Double_t tmin=0., Double_t tmax=-1.);
Int_t ApplyCuts(TStoreEvent* e);

#endif
