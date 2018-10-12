#include "PlotGetters.h"


//Plots

void Plot_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetTotalRunTime(runNumber);
  TH1D* h=Get_Chrono( runNumber, Chronoboard, ChronoChannel, tmin, tmax);
  h->Draw();
  return;  
} 

void Plot_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, const char* description, Int_t repetition, Int_t offset)
{
   Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
   Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
   return Plot_Chrono(runNumber, Chronoboard, ChronoChannel, tmin, tmax);
}

void Plot_Chrono(Int_t runNumber, const char* ChannelName, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetTotalRunTime(runNumber);
  TH1D* h=Get_Chrono( runNumber, ChannelName, tmin, tmax);
  h->Draw();
  return;  
} 

void Plot_Chrono(Int_t runNumber, const char* ChannelName, const char* description, Int_t repetition, Int_t offset)
{
   Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
   Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
   return Plot_Chrono(runNumber, ChannelName, tmin, tmax);
}

void Plot_TPC(Int_t runNumber,  Double_t tmin, Double_t tmax)
{
   if (tmax<0.) tmax=GetTotalRunTime(runNumber);
   TAGPlot* p=new TAGPlot(0); //Cuts off
   p->SetTimeRange(0.,tmax-tmin);
   p->AddEvents(runNumber,tmin,tmax);
   p->Canvas();
}
   
void Plot_TPC(Int_t runNumber,  const char* description, Int_t repetition, Int_t offset)
{
   Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
   Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
   return Plot_TPC(runNumber,tmin,tmax);
}

void Plot_ClockDrift_TPC(Int_t runNumber, Int_t tmin, Int_t tmax)
{
   if (tmax<0.) tmax=GetTotalRunTime(runNumber);
   TCanvas* c=new TCanvas("ClockDrift","ClockDrift",1200,800);
   c->Divide(1,3);
   c->cd(1);
   Get_TPC_EventTime_vs_OfficialTime(runNumber, tmin, tmax)->Draw();
   c->cd(2);
   Get_TPC_EventTime_vs_OfficialTime_Drift(runNumber,tmin,tmax)->Draw();
   c->cd(3);
   Get_TPC_EventTime_vs_OfficialTime_Matching(runNumber,tmin,tmax)->Draw();
   c->Draw();

}
