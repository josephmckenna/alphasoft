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
   double official_time;
   if (tmax<0.) tmax=GetTotalRunTime(runNumber);
   TTree* t=Get_Chrono_Tree(runNumber,ChronoBoard,ChronoChannel,official_time);
   TChrono_Event* e=new TChrono_Event();
   t->SetBranchAddress("ChronoEvent", &e);
   for (Int_t i = 0; i < t->GetEntries(); ++i)
   {
      t->GetEntry(i);
      if (official_time<tmin) continue;
      if (official_time>tmax) continue;
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


Int_t GetTPCEventNoBeforeOfficialTime(Double_t runNumber, Double_t tmin)
{
   double ot; //official time;
   TTree* t=Get_StoreEvent_Tree(runNumber, ot);
   TStoreEvent* e=new TStoreEvent();
   t->SetBranchAddress("StoredEvent", &e);
   int FirstEvent=-1;
   for (int i=0; i<t->GetEntries(); i++)
   {
      t->GetEntry(i);
      if (ot>tmin)
      {
         FirstEvent=i-1;
         break;
      }
   }
   delete e;
   return FirstEvent;
}


//*************************************************************
// Energy Analysis
//*************************************************************

Int_t LoadRampFile(const char* filename, Double_t* x, Double_t* y)
{
  std::ifstream fin(filename);
  Int_t n=0;
  while(fin.good())
    {
      fin>>x[n]>>y[n];
      //cout<<n<<"\t"<<x[n]<<"\t"<<y[n]<<endl;
      ++n;
    }
  fin.close();

  // actual number of points
  --n;
  //    cout<<"--> "<<n<<endl;
  Double_t endRampTime=-1.;

  for(Int_t i=n; i>0; --i)
    if(x[i]>0.000001)
      {
	endRampTime=x[i];
	break;
      }
  std::cout<<"Ramp Duration "<<"\t"<<endRampTime<<" s"<<std::endl;

  // time "normalization"
  //for(Int_t i=0; i<n; ++i) x[i] = x[i]/endRampTime;

  return n;
}
