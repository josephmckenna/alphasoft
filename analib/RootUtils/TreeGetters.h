#include "RootUtils.h"
#include "TFile.h"
#include "TSystem.h"

#ifndef _TreeGetters_
#define _TreeGetters_

TTree* Get_Tree_By_Name(Int_t runNumber,const char* name);


TTree* Get_Chrono_Tree_OfficialTime(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel);
TTree* Get_Chrono_Tree(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, double &official_time);
TTree* Get_Chrono_Tree(Int_t runNumber, const char* ChannelName, double &official_time);
TTree* Get_Chrono_Name_Tree(Int_t runNumber);
TTree* Get_Seq_Event_Tree(Int_t runNumber);
TTree* Get_StoreEvent_Tree(Int_t runNumber);
TTree* Get_StoreEvent_Tree(Int_t runNumber, Double_t &time);


// ALPHA 2 Getters:
TTreeReader* A2_SIS_Tree_Reader(Int_t runNumber);
//TTree* Get_A2_SVD_Tree(Int_t runNumber);
TTreeReader* Get_A2SpillTree(Int_t runNumber);

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
