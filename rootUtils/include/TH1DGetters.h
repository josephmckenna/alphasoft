#ifndef _TH1DGetter_
#define _TH1DGetter_

#include "TH1D.h"

#include "DoubleGetters.h"
#include "TStringGetters.h"

#include "RootUtilGlobals.h"

#ifdef BUILD_AG
#include "TAGSpillGetters.h"

std::vector<TH1D*> Get_Summed_Chrono(Int_t runNumber, std::vector<TChronoChannel> chan, std::vector<double> tmin, std::vector<double> tmax, double range = -1);
std::vector<TH1D*> Get_Summed_Chrono(Int_t runNumber, std::vector<TChronoChannel> chan, std::vector<TAGSpill> spills);
std::vector<TH1D*> Get_Summed_Chrono(Int_t runNumber, std::vector<std::string> description, std::vector<int> dumpIndex );
std::vector<std::vector<TH1D*>> Get_Chrono(Int_t runNumber, std::vector<TChronoChannel> chan, std::vector<double> tmin, std::vector<double> tmax, double range = -1);
std::vector<std::vector<TH1D*>> Get_Chrono(Int_t runNumber, std::vector<TChronoChannel> chan, std::vector<TAGSpill> spills);
std::vector<std::vector<TH1D*>> Get_Chrono(Int_t runNumber, std::vector<std::string> description, std::vector<int> dumpIndex );

TH1D* Get_Delta_Chrono(Int_t runNumber, TChronoChannel chan, Double_t tmin=0., Double_t tmax=-1., Double_t PlotMin=0., Double_t PlotMax=2.);
TH1D* Get_Delta_Chrono(Int_t runNumber, TChronoChannel chan, const char* description, Int_t dumpIndex=0);
TH1D* Get_Delta_Chrono(Int_t runNumber, const char* ChannelName, Double_t tmin=0., Double_t tmax=-1.);
TH1D* Get_Delta_Chrono(Int_t runNumber, const char* ChannelName, const char* description, Int_t dumpIndex=0);
#endif

#ifdef BUILD_A2
#include "TA2SpillGetters.h"
#include "TSISEvent.h"
#include "TSISChannels.h"
//ALPHA2
std::vector<TH1D*> Get_Summed_SIS(Int_t runNumber, std::vector<TSISChannel> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax, double range = -1);
std::vector<TH1D*> Get_Summed_SIS(Int_t runNumber, std::vector<TSISChannel> SIS_Channel, std::vector<TA2Spill> spills);
std::vector<std::vector<TH1D*>> Get_SIS(Int_t runNumber, std::vector<TSISChannel> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax);
std::vector<std::vector<TH1D*>> Get_SIS(Int_t runNumber, std::vector<TSISChannel> SIS_Channel, std::vector<TA2Spill> spills);
#endif



#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
