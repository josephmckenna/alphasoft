#include "TH1DGetters.h"

extern Int_t gNbin;

TH1D* Get_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin, Double_t tmax)
{
  if (tmax<0.) tmax=GetTotalRunTime(runNumber);
  double official_time;
  TTree* t=Get_Chrono_Tree(runNumber,Chronoboard,ChronoChannel,official_time);
  TChrono_Event* e=new TChrono_Event();
  TString name=Get_Chrono_Name(runNumber,Chronoboard,ChronoChannel);
  TString Title="R";
  Title+=runNumber;
  Title+=" Chrono - Board:";
  Title+=Chronoboard;
  Title+=" Channel:";
  Title+=ChronoChannel;
  Title+=" - ";
  Title+=name.Data();
  TH1D* hh = new TH1D(	name.Data(),
                      Title.Data(),
                      gNbin,0.,tmax-tmin);

  t->SetBranchAddress("ChronoEvent", &e);
  for (Int_t i = 0; i < t->GetEntries(); ++i)
  {
     t->GetEntry(i);
     if (official_time<tmin) continue;
     if (official_time>tmax) continue;
     hh->Fill(official_time-tmin,e->GetCounts());
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

TH1D* Get_Delta_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, Double_t tmin, Double_t tmax, Double_t PlotMin, Double_t PlotMax)
{
   if (tmax<0.) tmax=GetTotalRunTime(runNumber);
   double official_time;
   TTree* t=Get_Chrono_Tree(runNumber,Chronoboard,ChronoChannel,official_time);
   TChrono_Event* e=new TChrono_Event();
   TString name=Get_Chrono_Name(runNumber,Chronoboard,ChronoChannel);
   TString Title="Chrono Time between Events - Board:";
   Title+=Chronoboard;
   Title+=" Channel:";
   Title+=ChronoChannel;
   Title+=" - ";
   Title+=name.Data();
   TH1D* hh = new TH1D(name.Data(),
                      Title.Data(),
                      gNbin,PlotMin,PlotMax);

   t->SetBranchAddress("ChronoEvent", &e);
   t->GetEntry(0);
   double last_time=official_time;
   for (Int_t i = 1; i < t->GetEntries(); ++i)
   {
      t->GetEntry(i);
      if (official_time<tmin) continue;
      if (official_time>tmax) continue;
      hh->Fill(official_time-last_time,e->GetCounts());
      last_time=official_time;
   }
   return hh;
}
TH1D* Get_Delta_Chrono(Int_t runNumber, Int_t Chronoboard, Int_t ChronoChannel, const char* description, Int_t repetition, Int_t offset)
{
   Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
   Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
   return Get_Delta_Chrono( runNumber, Chronoboard, ChronoChannel, tmin, tmax);
}

TH1D* Get_Delta_Chrono(Int_t runNumber, const char* ChannelName, Double_t tmin, Double_t tmax)
{
   Int_t chan=-1;
   Int_t board;
   for (board=0; board<CHRONO_N_BOARDS; board++)
   {
      chan=Get_Chrono_Channel(runNumber, board, ChannelName);
      if (chan>-1) break;
   }
   return Get_Delta_Chrono(runNumber,board, chan, tmin, tmax);
}
TH1D* Get_Delta_Chrono(Int_t runNumber,const char* ChannelName, const char* description, Int_t repetition, Int_t offset)
{
   Double_t tmin=MatchEventToTime(runNumber, description,true,repetition, offset);
   Double_t tmax=MatchEventToTime(runNumber, description,false,repetition, offset);
   return Get_Delta_Chrono(runNumber, ChannelName, tmin, tmax);
}



std::vector<TH1D*> Get_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax, double range )
{
   assert(tmin.size()==tmax.size());
   double last_time=0;
   
   int n_times=tmin.size();
   
   //If range is not set, calcualte it
   if (range<0)
   {
      for (int i=0; i<n_times; i++)
      {
         double diff=tmax[i]-tmin[i];
         if (range<diff) range=diff;
      }
   }

   for (auto& t: tmax)
   {
      //Replace negative tmax times with the end of run...
      if (t<0) t=1E99; //FIXME: Get total run time!
      //Find the latest tmax time
      if (last_time<t ) last_time=t;
   }

   int n_chans=SIS_Channel.size();
   std::vector<TH1D*> hh;

   for (int i=0; i<n_chans; i++)
   {
      TString Title="R";
      Title+=runNumber;
      Title+=" SIS Channel:";
      Title+=SIS_Channel[i];
      Title+=" - ";

      //Replace this is a data base call to get the channel name
      TString name=SIS_Channel[i];
      //Title+=name.Data();

      TH1D* h= new TH1D( name.Data(),
                        Title.Data(),
                        gNbin,0.,range );
      hh.push_back(h);
   }

   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   TTreeReader* reader=A2_SIS_Tree_Reader(runNumber);
   TTreeReaderValue<TSISEvent> SISEvent(*reader, "TSISEvent");
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (reader->Next())
   {
      double t=SISEvent->GetRunTime();
      if (t>last_time) break;
      
      //Loop over all time windows
      for (int j=0; j<n_times; j++)
      {
         if (t>tmin[j] && t< tmax[j])
         {
            for (int i=0; i<n_chans; i++)
            {
               int counts=SISEvent->GetCountsInChannel(SIS_Channel[i]);
               if (counts)
               {
                  //std::cout<<t<<"\t"<<tmin[j]<<"\t"<<t-tmin[j]<<std::endl;
                  hh[i]->Fill(t-tmin[j],counts);
               }
            }
            //This event has been written to the array... so I dont need
            //to check the other winodws... break! Move to next SISEvent
            break;
         }
      }
   }
   return hh;
}

std::vector<TH1D*> Get_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<TA2Spill*> spills)
{
   std::vector<double> tmin;
   std::vector<double> tmax;
   for (auto & spill: spills)
   {
      if (spill->ScalerData)
      {
         tmin.push_back(spill->ScalerData->StartTime);
         tmax.push_back(spill->ScalerData->StopTime);
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return Get_SIS(runNumber,  SIS_Channel, tmin, tmax );
}

std::vector<TH1D*> Get_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<std::string> description, std::vector<int> repetition)
{
   std::vector<TA2Spill*> spills=Get_A2_Spills(runNumber, description, repetition);
   return Get_SIS( runNumber, SIS_Channel, spills);
}


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
