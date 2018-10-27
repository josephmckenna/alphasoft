#include "TH1DGetters.h"

extern Int_t gNbin;

TH1D* Get_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetTotalRunTime(runNumber);
  double official_time;
  TTree* t=Get_Chrono_Tree(runNumber,Chronoboard,ChronoChannel,official_time);
  TChrono_Event* e=new TChrono_Event();
  TString name=Get_Chrono_Name(runNumber,Chronoboard,ChronoChannel);
  TString Title="Chrono - Board:";
  Title+=Chronoboard;
  Title+=" Channel:";
  Title+=ChronoChannel;
  Title+=" - ";
  Title+=name.Data();
  TH1D* hh = new TH1D(	name.Data(),
                      Title.Data(),
                      gNbin,tmin,tmax);

  t->SetBranchAddress("ChronoEvent", &e);
  for (Int_t i = 0; i < t->GetEntries(); ++i)
  {
     t->GetEntry(i);
     if (official_time<tmin) continue;
     if (official_time>tmax) continue;
     hh->Fill(official_time,e->GetCounts());
   }
   return hh;
}
TH1D* Get_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, const char* description, Int_t repetition, Int_t offset)
{
  Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
  Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
  return Get_Chrono( runNumber, Chronoboard, ChronoChannel, tmin, tmax);
}

TH1D* Get_Chrono(Int_t runNumber, const char* ChannelName, Double_t tmin, Double_t tmax)
{
   Int_t chan=-1;
   Int_t board;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
       chan=Get_Chrono_Channel(runNumber, board, ChannelName);
       if (chan>-1) break;
   }
   return Get_Chrono(runNumber,board, chan, tmin, tmax);
}
TH1D* Get_Chrono(Int_t runNumber,const char* ChannelName, const char* description, Int_t repetition, Int_t offset)
{
  Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
  Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
  return Get_Chrono(runNumber, ChannelName, tmin, tmax);
}