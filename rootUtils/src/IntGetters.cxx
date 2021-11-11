#include "IntGetters.h"

#ifdef BUILD_AG
Int_t Get_Chrono_Channel_In_Board(Int_t runNumber, const std::string& ChronoBoard, const char* ChannelName, Bool_t ExactMatch)
{
   TTree* t=Get_Chrono_Name_Tree(runNumber);
   TChronoChannelName* n=new TChronoChannelName();
   t->SetBranchAddress("ChronoChannel", &n);
   t->GetEntry(TChronoChannel::CBMAP.at(ChronoBoard));
   Int_t Channel=n->GetChannel(ChannelName, ExactMatch);
   delete n;
   return Channel;
}
#endif

#ifdef BUILD_AG
Int_t GetCountsInChannel(Int_t runNumber,  TChronoChannel channel, Double_t tmin, Double_t tmax)
{
   Int_t Counts=0;
   if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);
   TTree* t=Get_Chrono_Tree(runNumber,channel.GetBranchName());
   TCbFIFOEvent* e = new TCbFIFOEvent();
   t->SetBranchAddress("FIFOData", &e);
   for (Int_t i = 0; i < t->GetEntries(); ++i)
   {
      t->GetEntry(i);
      if (e->GetRunTime() < tmin) continue;
      if (e->GetRunTime() > tmax) continue;
      //Is leading edge pulse
      if (! (e->GetFlag() & CB_HIT_FLAG_TE))
         Counts++;
   }
   return Counts;
}
#endif
#ifdef BUILD_AG
Int_t GetCountsInChannel(Int_t runNumber,  const char* ChannelName, Double_t tmin, Double_t tmax)
{
   TChronoChannel chan = Get_Chrono_Channel(runNumber, ChannelName);
   if (chan.IsValidChannel())
      return GetCountsInChannel( runNumber, chan, tmin, tmax);
   else
      return -1;
}
#endif
#ifdef BUILD_AG
Int_t ApplyCuts(TStoreEvent* e)
{
   //Dummy example of ApplyCuts for a TStoreEvent... 
   //Change this when we have pbars!
   Double_t R=e->GetVertex().Perp();
   Int_t NTracks=e->GetNumberOfTracks();
   if (NTracks==2)
      if (R<50) return 1;
   if (NTracks>2)
      if (R<45) return 1;
   return 0;
}
#endif
#ifdef BUILD_AG

#endif
#ifdef BUILD_AG
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
Int_t GetTPCEventNoBeforeDump(Double_t runNumber, const char* description, Int_t dumpIndex)
{
   std::vector<TAGSpill> s = Get_AG_Spills(runNumber,{description},{dumpIndex});
   Double_t tmin = s.front().GetStartTime();
   return  GetTPCEventNoBeforeOfficialTime(runNumber, tmin);
}
Int_t GetTPCEventNoAfterDump(Double_t runNumber, const char* description, Int_t dumpIndex)
{
   std::vector<TAGSpill> s = Get_AG_Spills(runNumber,{description},{dumpIndex});
   Double_t tmax = s.front().GetStopTime();
   return  GetTPCEventNoBeforeOfficialTime(runNumber, tmax)+1;
}
#endif

#ifdef BUILD_A2

Int_t GetSISChannel(int runNumber, const char* ChannelName)
{
   int chan=-1;
   TSISChannels sisch(runNumber);
   chan=sisch.GetChannel(ChannelName);
   return chan;
}
std::vector<Int_t> GetSISChannels(int runNumber, const std::vector<std::string>& ChannelNames)
{
    std::vector<Int_t> channels;
    TSISChannels sisch(runNumber);
    for (auto& name: ChannelNames)
    {
        channels.push_back(sisch.GetChannel(name.c_str()));
    }
    return channels;
}


int Count_SIS_Triggers(int runNumber, int ch, std::vector<double> tmin, std::vector<double> tmax)
{
   std::vector<std::pair<double,int>> counts = GetSISTimeAndCounts(runNumber, ch, tmin, tmax);
   int total = 0;
   for (const std::pair<double,int>& c: counts)
      total += c.second;
   return total;
}
#endif

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
      std::cout<<n<<"\t"<<x[n]<<"\t"<<y[n]<<std::endl;
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

  // time "normalization" to 1 second? Why would we ever need this?
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
