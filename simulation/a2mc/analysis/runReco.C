///< ##############################################
///< Developed for the Alpha experiment [Mar. 2021]
///< ##############################################
#include "a2mcReco.C"
a2mcReco* gReco = 0;
//______________________________________________________________________________
void runReco(Int_t runNumber=0, bool save_histos = true, bool verbose = false) {
    ///< ======== Create the a2mcReco (it reads the MC output)
    gReco = new a2mcReco(runNumber);
    ///< ======== Reconstruct MC events
    gReco->Reco(verbose);
    ///< ======== Show histos
    gReco->ShowHistos(save_histos);
    ///< ======== Finished
    delete gReco;
//    gROOT->ProcessLine(".q");
}
