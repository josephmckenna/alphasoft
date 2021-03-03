///< ##############################################
///< Developed for the Alpha experiment [Mar. 2021]
///< ##############################################
#include "a2mcReco.C"
a2mcReco* gReco = 0;
bool verbose = false;
//______________________________________________________________________________
void runReco(Int_t runNumber=0) {
    ///< ======== Create the a2mcReco (it reads the MC output)
    gReco = new a2mcReco(runNumber);
    ///< ======== Reconstruct MC events
    gReco->Reco(verbose);
    ///< ======== Show histos
    gReco->ShowHistos();
    ///< ======== Finished
    delete gReco;
//    gROOT->ProcessLine(".q");
}
