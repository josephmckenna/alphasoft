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

TGraph* Get_TPC_EventTime_vs_OfficialTime_Drift(Int_t runNumber, Double_t tmin, Double_t tmax)
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
      y.push_back(e->GetTimeOfEvent()-official_time);
      points++;
   }
   TGraph* g=new TGraph(points,x.data(),y.data());
   g->SetTitle("TPC Time over Official time; Entry #; TPC Time - Official time");
   return g;
}
TGraph* Get_TPC_EventTime_vs_OfficialTime_Matching(Int_t runNumber, Double_t tmin, Double_t tmax)
{
   if (tmax<0) tmax=GetTotalRunTime(runNumber);
   std::vector<double> x,y;
   int points=0;
   double official_time;
   TTree* t=Get_StoreEvent_Tree(runNumber, official_time);
   TStoreEvent* e=new TStoreEvent();
   t->SetBranchAddress("StoredEvent",&e);
   
   double lastTPC=0;
   double lastOfficial=0;
   for (int i=0; i<t->GetEntries(); i++)
   {
      t->GetEntry(i);
      if (official_time<tmin) continue;
      //Skip first data point... we are plotting diffs of diffs so we need to
      if (!points)
      {
         lastTPC=e->GetTimeOfEvent();
         lastOfficial=official_time;
         points++;
         continue;
      }
      if (official_time>tmax) break;
      x.push_back((double)points);
      y.push_back((e->GetTimeOfEvent()-lastTPC)-(official_time-lastOfficial));
      lastTPC=e->GetTimeOfEvent();
      lastOfficial=official_time;
      points++;
   }
   TGraph* g=new TGraph(points-1,x.data(),y.data());
   g->SetTitle("Time between events (should scatter around size of LNE) TPC time vs Official time; Entry #; Delta (TPC Time) - Delta(Official time)");
   return g;
}

