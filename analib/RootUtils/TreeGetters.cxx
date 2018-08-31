#include "TreeGetters.h"


TTree* Get_Chrono_Tree(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel)
{
  TFile* f=Get_File(runNumber);
  
  TTree *chrono_tree = NULL;
  TString Name="ChronoEventTree_";
            Name+=Chronoboard;
            Name+="_";
            Name+=ChronoChannel;
  chrono_tree = (TTree *)f->Get(Name);
  if (chrono_tree == NULL)
  {
    Error("Get_Chrono_Tree", "\033[31mChrono Tree for run number %d not found\033[00m", runNumber);
    chrono_tree->GetName(); // This is to crash the CINT interface  instead of exiting (deliberately)
  }
  return chrono_tree;
}
