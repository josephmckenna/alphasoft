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


TTree* Get_Chrono_Tree(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel)
{
   TString Name="chrono/ChronoEventTree_";
           Name+=Chronoboard;
           Name+="_";
           Name+=ChronoChannel;
   return Get_Tree_By_Name(runNumber,Name.Data());
}

TTree* Get_Chrono_Tree(Int_t runNumber, const char* ChannelName)
{
   Int_t chan=-1;
   Int_t board=-1;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
       chan=Get_Chrono_Channel(runNumber, board, ChannelName);
       if (chan>-1) break;
   }
   return Get_Chrono_Tree(runNumber,board,chan);
}


TTree* Get_Chrono_Name_Tree(Int_t runNumber)
{
   return Get_Tree_By_Name(runNumber,"ChronoBoxChannels");
}

TTree* Get_Seq_Event_Tree(Int_t runNumber)
{
   return Get_Tree_By_Name(runNumber,"SequencerEventTree");
}

TTree* Get_StoreEvent_Tree(Int_t runNumber)
{
   return Get_Tree_By_Name(runNumber,"StoreEventTree");
}

TTree* Get_StoreEvent_Tree(Int_t runNumber, Double_t &time)
{
   TTree* t=Get_StoreEvent_Tree(runNumber);
   TTree* tf=Get_Tree_By_Name(runNumber,"StoreEventOfficialTime");
   tf->SetBranchAddress("OfficalTime",&time);
   t->AddFriend(tf);
   return t;
}
