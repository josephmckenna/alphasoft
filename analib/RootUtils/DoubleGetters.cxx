#include "DoubleGetters.h"



Double_t GetTotalRunTime(Int_t runNumber)
{
  TTree* t=Get_Chrono_Tree(runNumber,0,CHRONO_CLOCK_CHANNEL);
  TChrono_Event* e=new TChrono_Event();
  t->SetBranchAddress("ChronoEvent", &e);
  t->GetEntry(t->GetEntries()-1);
  Double_t RunTime=e->GetRunTime();
  delete e;
  return RunTime;
}
