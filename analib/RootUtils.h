


#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include "TROOT.h"

#include "TChrono_Event.h"

#include "TH1D.h"


TFile* Get_File(Int_t run_number, Bool_t die=kFALSE);

TTree* Get_Chrono_Tree(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel);
TH1D* Get_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin=0., Double_t tmax=-1.);
void Plot_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin=0., Double_t tmax=-1.);
