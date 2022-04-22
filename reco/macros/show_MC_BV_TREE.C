///< Developed for the Alpha experiment [Mar. 2021]
///< ##############################################
#include "MC_BV_TREE_analysis.C"
MC_BV_TREE_analysis* gShow = 0;
//______________________________________________________________________________
void show_MC_BV_TREE(string file_name="") 
{
    gShow = new MC_BV_TREE_analysis(file_name);
    gShow->ShowBV();
    // ///< ======== Finished
    // delete gShow;
    // gROOT->ProcessLine(".q");
}
