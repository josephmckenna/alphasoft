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


ChronoChannel Get_Chrono_Channel(Int_t runNumber, const char* ChannelName, Bool_t ExactMatch)
{
   ChronoChannel c;
   c.Channel=-1;
   c.Board=-1;
   for (int board=0; board<CHRONO_N_BOARDS; board++)
   {
      int chan=Get_Chrono_Channel(runNumber, board, ChannelName, ExactMatch);
      if (chan>0)
      {
         c.Channel=chan;
         c.Board=board;
         return c;
      }
   }
   return {-1, -1};
}


Int_t GetCountsInChannel(Int_t runNumber,  Int_t ChronoBoard, Int_t Channel, Double_t tmin, Double_t tmax)
{
   Int_t Counts=0;
   double official_time;
   if (tmax<0.) tmax=GetTotalRunTime(runNumber);
   TTree* t=Get_Chrono_Tree(runNumber,ChronoBoard,Channel,official_time);
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
      e->Reset();
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
Int_t GetTPCEventNoBeforeDump(Double_t runNumber, const char* description, Int_t repetition, Int_t offset)
{
   Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
   return  GetTPCEventNoBeforeOfficialTime(runNumber, tmin);
}
Int_t GetTPCEventNoAfterDump(Double_t runNumber, const char* description, Int_t repetition, Int_t offset)
{
   Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
   return  GetTPCEventNoBeforeOfficialTime(runNumber, tmax)+1;
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
      //std::cout<<n<<"\t"<<x[n]<<"\t"<<y[n]<<std::endl;
      ++n;
    }
  fin.close();

  // actual number of points
  --n;
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

#include <TRegexp.h>
int GetRunNumber( TString fname )
{
   //  TRegexp re("[0-9][0-9][0-9][0-9][0-9]");
  TRegexp re("[0-9][0-9][0-9][0-9]");
  int pos = fname.Index(re);
  //  int run = TString(fname(pos,5)).Atoi();
  int run = TString(fname(pos,6)).Atoi();
  return run;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
