#include "RootUtils.h"
#include "TH1D.h"

#ifndef _PlotGetters_
#define _PlotGetters_

void Plot_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin=0., Double_t tmax=-1.);
void Plot_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, const char* description, Int_t repetition=1, Int_t offset=0);

void Plot_Chrono(Int_t runNumber, const char* ChannelName, Double_t tmin=0., Double_t tmax=-1.);
void Plot_Chrono(Int_t runNumber, const char* ChannelName, const char* description, Int_t repetition=1, Int_t offset=0);

void Plot_TPC(Int_t runNumber,  Double_t tmin=0., Double_t tmax=-1.);
void Plot_TPC(Int_t runNumber,  const char* description, Int_t repetition=1, Int_t offset=0);
void Plot_ClockDrift_TPC(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);
void Plot_ClockDrift_Chrono(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);
#endif
