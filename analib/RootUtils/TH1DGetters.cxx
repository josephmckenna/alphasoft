#include "TH1DGetters.h"

extern Int_t gNbin;

TH1D* Get_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetTotalRunTime(runNumber);
  TTree* t=Get_Chrono_Tree(runNumber,Chronoboard,ChronoChannel);
  TChrono_Event* e=new TChrono_Event();
  TString name=Get_Chrono_Name(runNumber,Chronoboard,ChronoChannel);
  TH1D* hh = new TH1D(	name.Data(),
                      name.Data(),
                      gNbin,tmin,tmax);

  t->SetBranchAddress("ChronoEvent", &e);
  for (Int_t i = 0; i < t->GetEntries(); ++i)
  {
     t->GetEntry(i);
     if (e->GetRunTime()<tmin) continue;
     if (e->GetRunTime()>tmax) continue;
     hh->Fill(e->GetRunTime(),e->GetCounts());
   }
   return hh;
}
