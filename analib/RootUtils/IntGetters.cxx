#include "IntGetters.h"




Int_t Get_Chrono_Channel(Int_t runNumber, Int_t ChronoBoard, const char* ChannelName, Bool_t ExactMatch)
{
   TTree* t=Get_Chrono_Name_Tree(runNumber);
   TChronoChannelName* n=new TChronoChannelName();
   t->SetBranchAddress("ChronoChannel", &n);
   t->GetEntry(ChronoBoard);
   Int_t Channel=n->GetChannel(ChannelName, ExactMatch);
   delete n;
   return Channel;
}

Int_t GetCountsInChannel(Int_t runNumber,  Int_t ChronoBoard, Int_t ChronoChannel, Double_t tmin, Double_t tmax)
{
   Int_t Counts=0;
   if (tmax<0.) tmax=GetTotalRunTime(runNumber);
   TTree* t=Get_Chrono_Tree(runNumber,ChronoBoard,ChronoChannel);
   TChrono_Event* e=new TChrono_Event();
   t->SetBranchAddress("ChronoEvent", &e);
   for (Int_t i = 0; i < t->GetEntries(); ++i)
   {
      t->GetEntry(i);
      if (e->GetRunTime()<tmin) continue;
      if (e->GetRunTime()>tmax) continue;
      Counts+=e->GetCounts();
   }
   return Counts;
}


Int_t GetCountsInChannel(Int_t runNumber,  const char* ChannelName, Double_t tmin, Double_t tmax)
{
   Int_t chan=-1;
   Int_t board=-1;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
       chan=Get_Chrono_Channel(runNumber, board, ChannelName);
       if (chan>-1) break;
   }
   return GetCountsInChannel( runNumber,  board, chan, tmin, tmax);
}


Int_t ApplyCuts(TStoreEvent* e)
{
   //Dummy example of ApplyCuts for a TStoreEvent... 
   //Change this when we have pbars!
   Double_t R=e->GetVertex().Perp();
   Int_t NTracks=e->GetNumberOfTracks();
   if (NTracks==2)
      if (R<5) return 1;
   if (NTracks>2)
      if (R<4.5) return 1;
   return 0;
}
