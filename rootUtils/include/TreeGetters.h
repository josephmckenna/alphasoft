#ifndef _TreeGetters_
#define _TreeGetters_
#include "TTreeReader.h"
#include "FileGetters.h"
#include "TSystem.h"



TTree* Get_Tree_By_Name(Int_t runNumber,const char* name);

#ifdef BUILD_AG
#include "TChrono_Event.h"

TTree* Get_Chrono_Tree_OfficialTime(Int_t runNumber, std::pair<Int_t,Int_t> ChronoBoardChannel);
TTree* Get_Chrono_Tree(Int_t runNumber, std::pair<Int_t,Int_t> ChronoBoardChannel, double &official_time);
//TTree* Get_Chrono_Tree(Int_t runNumber, const char* ChannelName, double &official_time);
TTree* Get_Chrono_Name_Tree(Int_t runNumber);

TTree* Get_StoreEvent_Tree(Int_t runNumber);
TTree* Get_StoreEvent_Tree(Int_t runNumber, Double_t &time);


// ALPHA 2 Getters:
TTreeReader* A2_SIS_Tree_Reader(Int_t runNumber, Int_t SIS_Module);
TTreeReader* Get_A2_SVD_Tree(Int_t runNumber);
TTreeReader* Get_A2SpillTree(Int_t runNumber);

#endif
TTree* Get_Seq_Event_Tree(Int_t runNumber);
TTree* Get_Seq_State_Tree(Int_t runNumber);

// ALPHA 2 Getters:
#ifdef BUILD_A2
TTreeReader* A2_SIS_Tree_Reader(Int_t runNumber);
TTreeReader* Get_A2_SVD_Tree(Int_t runNumber);
TTreeReader* Get_A2SpillTree(Int_t runNumber);
TTreeReader* Get_TA2AnalysisReport_Tree(Int_t runNumber);
#endif

TTreeReader* Get_feGEM_Tree(Int_t runNumber, const std::string& Category, const std::string& Varname);
TTreeReader* Get_feGEM_Tree(Int_t runNumber, const std::string& CombinedName);

TTreeReader* Get_feLV_Tree(Int_t runNumber, const std::string& BankName);

std::vector<TTreeReader*> Get_feGEM_File_Trees(Int_t runNumber, const std::string& CombinedName);

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
