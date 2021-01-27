#include "TreeGetters.h"

TTree* Get_Tree_By_Name(Int_t runNumber,const char* name)
{
   TFile* f=Get_File(runNumber);
   TTree *tree = NULL;
   tree = (TTree *)f->Get(name);
   if (tree == NULL)
   {
      Error(name, "\033[31mTree for run number %d not found\033[00m", runNumber);
      tree->GetName(); // This is to crash the CINT interface  instead of exiting (deliberately)
   }
   return tree;
}
#ifdef BUILD_AG
TTree* Get_Chrono_Tree_OfficialTime(Int_t runNumber, std::pair<Int_t,Int_t> ChronoBoardChannel)
{
   TString Name="chrono/ChronoEventTree_";
           Name+=ChronoBoardChannel.first;
           Name+="_";
           Name+=ChronoBoardChannel.second;
           Name+="OfficialTime";
   return Get_Tree_By_Name(runNumber,Name.Data());
}
#endif
#ifdef BUILD_AG
TTree* Get_Chrono_Tree(Int_t runNumber, std::pair<Int_t,Int_t> ChronoBoardChannel, double &official_time)
{
   TString Name="chrono/ChronoEventTree_";
           Name+=ChronoBoardChannel.first;
           Name+="_";
           Name+=ChronoBoardChannel.second;
   TTree* t=Get_Tree_By_Name(runNumber,Name.Data());
   Name+="OfficialTime";
   TTree* tf=Get_Tree_By_Name(runNumber,Name.Data());
   tf->SetBranchAddress("OfficialTime",&official_time);
   t->AddFriend(tf);
   return t;
}
#endif
#ifdef BUILD_AG
/*TTree* Get_Chrono_Tree(Int_t runNumber, const char* ChannelName, double &official_time)
{
   Int_t chan=-1;
   Int_t board=-1;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
       chan=Get_Chrono_Channel(runNumber, board, ChannelName);
       if (chan>-1) break;
   }
   return Get_Chrono_Tree(runNumber,board,chan,official_time);
}*/
#endif
#ifdef BUILD_AG
TTree* Get_Chrono_Name_Tree(Int_t runNumber)
{
   return Get_Tree_By_Name(runNumber,"ChronoBoxChannels");
}
#endif

TTree* Get_Seq_Event_Tree(Int_t runNumber)
{
   return Get_Tree_By_Name(runNumber,"SequencerEventTree");
}
#ifdef BUILD_AG
TTree* Get_StoreEvent_Tree(Int_t runNumber)
{
   return Get_Tree_By_Name(runNumber,"StoreEventTree");
}
#endif
#ifdef BUILD_AG
TTree* Get_StoreEvent_Tree(Int_t runNumber, Double_t &time)
{
   TTree* t=Get_StoreEvent_Tree(runNumber);
   TTree* tf=Get_Tree_By_Name(runNumber,"StoreEventOfficialTime");
   tf->SetBranchAddress("OfficalTime",&time);
   t->AddFriend(tf);
   return t;
}
#endif
#ifdef BUILD_A2
// ALPHA 2 Getters:
TTreeReader* A2_SIS_Tree_Reader(Int_t runNumber)
{
   TFile* f=Get_File(runNumber);
   TTreeReader* t=new TTreeReader("SISEventTree", f);
   return t;
}

TTreeReader* Get_A2_SVD_Tree(Int_t runNumber)
{
   TFile* f=Get_File(runNumber);
   TTreeReader* t=new TTreeReader("SVDOfficialA2Time",f);
   return t;
}
TTreeReader* Get_A2SpillTree(Int_t runNumber)
{
   TFile* f=Get_File(runNumber);
   TTreeReader* t=new TTreeReader("A2SpillTree", f);
   return t;
}
#endif

TTreeReader* Get_feGEM_Tree(Int_t runNumber, const std::string& Category, const std::string& Varname)
{
   std::string CombinedName = Category + "\\" + Varname;
   return Get_feGEM_Tree(runNumber,CombinedName);
}

TTreeReader* Get_feGEM_Tree(Int_t runNumber, const std::string& CombinedName)
{
   TFile* f = Get_File(runNumber);
   TDirectory* d=f->GetDirectory("feGEM");
   TTreeReader* t = new TTreeReader(CombinedName.c_str(), f->GetDirectory("/feGEM"));
   return t;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
