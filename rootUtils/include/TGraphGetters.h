#include "RootUtils.h"
#include "TGraph.h"
#include "TTree.h"

#ifndef _TGraphGetters_
#define _TGraphGetters_
#ifdef BUILD_AG
TGraph* Get_TPC_EventTime_vs_OfficialTime(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);
TGraph* Get_TPC_EventTime_vs_OfficialTime_Drift(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);
TGraph* Get_TPC_EventTime_vs_OfficialTime_Matching(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);

// 3 comparisons of Chronobox run time with its own official time
TGraph* Get_Chrono_EventTime_vs_OfficialTime(Int_t runNumber, Int_t b, Double_t tmin=0., Double_t tmax=-1.);
TGraph* Get_Chrono_EventTime_vs_OfficialTime_Drift(Int_t runNumber, Int_t b, Double_t tmin=0., Double_t tmax=-1.);
TGraph* Get_Chrono_EventTime_vs_OfficialTime_Matching(Int_t runNumber, Int_t b, Double_t tmin=0., Double_t tmax=-1.);
#endif

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
