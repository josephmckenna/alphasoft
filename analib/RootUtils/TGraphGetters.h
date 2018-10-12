#include "RootUtils.h"
#include "TGraph.h"
#include "TTree.h"

#ifndef _TGraphGetters_
#define _TGraphGetters_

TGraph* Get_TPC_EventTime_vs_OfficialTime(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);
TGraph* Get_TPC_EventTime_vs_OfficialTime_Drift(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);
TGraph* Get_TPC_EventTime_vs_OfficialTime_Matching(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);
#endif
