#include "RootUtils.h"
#include "TH1D.h"


#ifndef _TH1DGetter_
#define _TH1DGetter_

TH1D* Get_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin=0., Double_t tmax=-1.);
TH1D* Get_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, const char* description, Int_t repetition=1, Int_t offset=0);
TH1D* Get_Chrono(Int_t runNumber, const char* ChannelName, Double_t tmin=0., Double_t tmax=-1.);
TH1D* Get_Chrono(Int_t runNumber, const char* ChannelName, const char* description, Int_t repetition=1, Int_t offset=0);

TH1D* Get_Delta_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin=0., Double_t tmax=-1., Double_t PlotMin=0., Double_t PlotMax=2.);
TH1D* Get_Delta_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, const char* description, Int_t repetition=1, Int_t offset=0);
TH1D* Get_Delta_Chrono(Int_t runNumber, const char* ChannelName, Double_t tmin=0., Double_t tmax=-1.);
TH1D* Get_Delta_Chrono(Int_t runNumber, const char* ChannelName, const char* description, Int_t repetition=1, Int_t offset=0);

#include "TA2SpillGetters.h"
//ALPHA2
std::vector<TH1D*> Get_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax, double range = -1);
std::vector<TH1D*> Get_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<TA2Spill*> spills);



#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
