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

