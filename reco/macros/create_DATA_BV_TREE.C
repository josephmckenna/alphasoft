#include "DATA_BV_TREE.C"
#include <limits>
DATA_BV_TREE* gRead = NULL;
//______________________________________________________________________________
void create_DATA_BV_TREE(std::string file_name="", Double_t t_pbars = std::numeric_limits<double>::max())
{
    ///< t_pbars is the time (in s) in which there are pbars in the ALPHA-g traps
    ///< if t_pbars = std::numeric_limits<double>::max() => cosmics run
    gRead = new DATA_BV_TREE(file_name, t_pbars);
    gRead->Loop();
    // ///< ======== Finished
    delete gRead;
    gROOT->ProcessLine(".q");
}
