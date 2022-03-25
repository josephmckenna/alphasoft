///< Developed for the Alpha experiment [Mar. 2021]
///< ##############################################
#include "read_MC_BV.C"
read_MC_BV* gRead = 0;
//______________________________________________________________________________
void run_read_MC_BV(string file_name="", Float_t EnergyCut=-999.0, Float_t DeltaPhiCut = -999.0, Int_t MultCut = 999, Float_t smearingTime = -999.0) {
    gRead = new read_MC_BV(file_name);
    gRead->AnalyzeMCinfo();
    gRead->AnalyzeBVBars(EnergyCut, DeltaPhiCut, MultCut, smearingTime);
    // ///< ======== Finished
    // delete gRead;
    // gROOT->ProcessLine(".q");
}