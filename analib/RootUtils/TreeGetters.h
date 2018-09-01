#include "RootUtils.h"
#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"

#ifndef _TreeGetters_
#define _TreeGetters_

TTree* Get_Chrono_Tree(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel);
TTree* Get_Chrono_Name_Tree(Int_t runNumber);
TTree* Get_Seq_Event_Tree(Int_t runNumber);
#endif
