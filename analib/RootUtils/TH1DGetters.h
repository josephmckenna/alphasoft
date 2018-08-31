#include "RootUtils.h"
#include "TH1D.h"


#ifndef _TH1DGetter_
#define _TH1DGetter_

TH1D* Get_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin=0., Double_t tmax=-1.);

#endif
