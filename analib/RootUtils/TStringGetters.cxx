#include "TStringGetters.h"





TString Get_Chrono_Name(Int_t runNumber, Int_t ChronoBoard, Int_t Channel)
{
   TTree* t=Get_Chrono_Name_Tree(runNumber);
   TChronoChannelName* n=new TChronoChannelName();
   t->SetBranchAddress("ChronoChannel", &n);
   t->GetEntry(ChronoBoard);
   TString name=n->GetChannelName(Channel);
   delete n;
   return name;
}
   
   
