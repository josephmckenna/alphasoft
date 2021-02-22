///< ##############################################
///< Developed for the Alpha experiment [Dec. 2020]
///< germano.bonomi@cern.ch
///< ##############################################
#include "a2mcToRAW.C"
a2mcToRAW* gToRAW = 0;

//______________________________________________________________________________
void runMCToRAW(Int_t runNumber=0) {
    
    gToRAW = new a2mcToRAW(runNumber);
    gToRAW->WriteRAW();
    delete gToRAW;
    gROOT->ProcessLine(".q");
}
