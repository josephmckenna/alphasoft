///< ##############################################
///< Developed for the Alpha experiment [Mar. 2021]
///< ##############################################
#include "a2mcReco.C"
a2mcReco* gReco = 0;
//______________________________________________________________________________
void runReco(Int_t runNumber=0, Double_t hit_threshold = 0., Int_t nMinHits=0, bool save_histos = false, bool verbose = false) {
    ///< ======== Create the a2mcReco (it reads the MC output)
    gReco = new a2mcReco(runNumber, hit_threshold, nMinHits);
    ///< ======== Reconstruct MC events
    gReco->Reco(verbose);
    ///< ======== Show histos
    gReco->ShowHistos(save_histos);
    ///< ======== Finished
    delete gReco;
//    gROOT->ProcessLine(".q");
}
