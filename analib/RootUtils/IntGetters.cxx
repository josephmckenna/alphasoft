#include "IntGetters.h"




Int_t Get_Chrono_Channel(Int_t runNumber, Int_t ChronoBoard, TString ChannelName, Bool_t ExactMatch)
{
   TTree* t=Get_Chrono_Name_Tree(runNumber);
   TChronoChannelName* n=new TChronoChannelName();
   t->SetBranchAddress("ChronoChannel", &n);
   t->GetEntry(ChronoBoard);
   Int_t Channel=n->GetChannel(ChannelName, ExactMatch);
   delete n;
   return Channel;
}
