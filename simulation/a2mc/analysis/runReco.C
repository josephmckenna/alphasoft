///< ##############################################
///< Developed for the Alpha experiment [Dec. 2020]
///< germano.bonomi@cern.ch
///< ##############################################
#include "a2mcReco.C"
a2mcReco* gReco = 0;

//______________________________________________________________________________
void runReco(Int_t runNumber=0) {
    
    gReco = new a2mcReco(runNumber);
    gReco->Reco();
    delete gReco;
    gROOT->ProcessLine(".q");
}
