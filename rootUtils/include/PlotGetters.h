#include "RootUtils.h"
#include "TH1D.h"
#include "TSpline.h"

#ifndef _PlotGetters_
#define _PlotGetters_


#include "TA2Plot.h"

void Plot_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin=0., Double_t tmax=-1.);
void Plot_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, const char* description, Int_t repetition=1, Int_t offset=0);
void Plot_Chrono(Int_t runNumber, const char* ChannelName, Double_t tmin=0., Double_t tmax=-1.);
void Plot_Chrono(Int_t runNumber, const char* ChannelName, const char* description, Int_t repetition=1, Int_t offset=0);

void Plot_Delta_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin=0., Double_t tmax=-1.);
void Plot_Delta_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, const char* description, Int_t repetition=1, Int_t offset=0);
void Plot_Delta_Chrono(Int_t runNumber, const char* ChannelName, Double_t tmin=0., Double_t tmax=-1.);
void Plot_Delta_Chrono(Int_t runNumber, const char* ChannelName, const char* description, Int_t repetition=1, Int_t offset=0);

void PlotScintillators(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);
void PlotScintillators(Int_t runNumber, const char* description, Int_t repetition=1, Int_t offset=0);

void Plot_TPC(Int_t runNumber,  Double_t tmin=0., Double_t tmax=-1.);
void Plot_TPC(Int_t runNumber,  const char* description, Int_t repetition=1, Int_t offset=0);
void Plot_TPC(Int_t* runNumber, Int_t Nruns, const char* description, Int_t repetition=1, Int_t offset=0);
void Plot_Vertices_And_Tracks(Int_t runNumber, double tmin, double tmax);
void Plot_Vertices_And_Tracks(Int_t runNumber, const char* description, 
			      Int_t repetition=1, Int_t offset=0);
void Plot_Vertices_And_Tracks(Int_t* runNumber, Int_t Nruns, const char* description, 
			      Int_t repetition=1, Int_t offset=0);

void Plot_ClockDrift_TPC(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);
void Plot_ClockDrift_Chrono(Int_t runNumber, Double_t tmin=0., Double_t tmax=-1.);
void Plot_Chrono_Sync(Int_t runNumber, Double_t tmin=0., Double_t max=-1.);

//*************************************************************
// Energy Analysis
//*************************************************************
TCanvas* Plot_CT_ColdDump(Int_t runNumber, Int_t binNumber=1000, 
                          const char* dumpFile="ana/macros/ColdDumpE4E5.dump",
			  Double_t EnergyRangeFactor=10.);
TCanvas* Plot_AG_RCT_ColdDump(Int_t runNumber,Int_t binNumber=1000, 
                          const char* dumpFile="ana/macros/RCT_BOTg_rampfile.dump", 
                          Double_t EnergyRangeFactor=10.);


Double_t FitEnergyDump(Double_t Emin, Double_t Emax,TH1D* fit=NULL);

void SaveCanvas();
void SaveCanvas(Int_t runNumber, const char* Description);
void SaveCanvas(TString Description);
void SaveCanvas( TCanvas* iSaveCanvas, TString iDescription);


void Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax, double range = -1);
void Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<TA2Spill*> spills);
void Plot_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<std::string> description, std::vector<int> repetition);



void Plot_SVD(Int_t runNumber, std::vector<double> tmin, std::vector<double> tmax);
void Plot_SVD(Int_t runNumber, std::vector<TA2Spill*> spills);
void Plot_SVD(Int_t runNumber, std::vector<std::string> description, std::vector<int> repetition);


#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */