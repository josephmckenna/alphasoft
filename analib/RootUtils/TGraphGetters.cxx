#include "TGraphGetters.h"


TGraph* Get_TPC_EventTime_vs_OfficialTime(Int_t runNumber, Double_t tmin, Double_t tmax)
{
   if (tmax<0) tmax=GetTotalRunTime(runNumber);
   std::vector<double> x,y;
   int points=0;
   double official_time;
   TTree* t=Get_StoreEvent_Tree(runNumber, official_time);
   TStoreEvent* e=new TStoreEvent();
   t->SetBranchAddress("StoredEvent",&e);
   for (int i=0; i<t->GetEntries(); i++)
   {
      t->GetEntry(i);
      if (official_time<tmin) continue;
      if (official_time>tmax) break;
      x.push_back(official_time);
      y.push_back(e->GetTimeOfEvent());
      points++;
   }
   TGraph* g=new TGraph(points,x.data(),y.data());
   g->SetTitle("TPC Time vs Official time; Official Time (s); TPC Time (s)");
   return g;
}

TGraph* Get_TPC_EventTime_over_OfficialTime(Int_t runNumber, Double_t tmin, Double_t tmax)
{
   if (tmax<0) tmax=GetTotalRunTime(runNumber);
   std::vector<double> x,y;
   int points=0;
   double official_time;
   TTree* t=Get_StoreEvent_Tree(runNumber, official_time);
   TStoreEvent* e=new TStoreEvent();
   t->SetBranchAddress("StoredEvent",&e);
   for (int i=0; i<t->GetEntries(); i++)
   {
      t->GetEntry(i);
      if (official_time<tmin) continue;
      if (official_time>tmax) break;
      x.push_back((double)points);
      y.push_back(e->GetTimeOfEvent()/official_time);
      points++;
   }
   TGraph* g=new TGraph(points,x.data(),y.data());
   g->SetTitle("TPC Time over Official time; Entry #; TPC Time / Official time");
   return g;
}

