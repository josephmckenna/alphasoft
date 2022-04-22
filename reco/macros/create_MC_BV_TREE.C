///< Developed for the Alpha experiment [Mar. 2021]
///< ##############################################
#include "MC_BV_TREE.C"
MC_BV_TREE* gRead = 0;
//______________________________________________________________________________
void create_MC_BV_TREE(string file_name="") {
    gROOT->ProcessLine( "gErrorIgnoreLevel = kError");
    gRead = new MC_BV_TREE(file_name);
    gRead->AnalyzeBVBars();
    // ///< ======== Finished
    delete gRead;
    gROOT->ProcessLine(".q");
}
