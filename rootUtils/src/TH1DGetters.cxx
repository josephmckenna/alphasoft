#include "TH1DGetters.h"

extern Int_t gNbin;



#ifdef BUILD_AG
std::vector<TH1D*> Get_Summed_Chrono(Int_t runNumber, std::vector<TChronoChannel> chrono_chan, std::vector<double> tmin, std::vector<double> tmax, double range )
{
   assert(tmin.size()==tmax.size());
   double first_time = 1E99;
   double last_time = 0;
   
   const size_t n_times = tmin.size();
   
   //If range is not set, calcualte it
   if (range<0)
   {
      for (int i=0; i<n_times; i++)
      {
         double diff=tmax[i]-tmin[i];
         if (range<diff) range=diff;
      }
   }
   for (auto& t: tmin)
   {
      if (t < first_time) first_time = t;
   }
   for (auto& t: tmax)
   {
      //Replace negative tmax times with the end of run...
      if (t < 0) t = 1E99; //FIXME: Get total run time!
      //Find the latest tmax time
      if (last_time < t ) last_time = t;
   }

   int n_chans=chrono_chan.size();
   std::vector<TH1D*> hh;

   for (int i=0; i<n_chans; i++)
   {
      TString Title="R";
      Title+=runNumber;
      Title+=" Chrono Channel:";
      Title+=chrono_chan[i].GetChannel();
      Title+=" - ";
      Title+=Get_Chrono_Name(runNumber, chrono_chan[i]);

      //Replace this is a data base call to get the channel name
      TString name=Get_Chrono_Name(runNumber, chrono_chan[i]);
      //Title+=name.Data();

      TH1D* h= new TH1D( name.Data(),
                        Title.Data(),
                        gNbin,0.,range );
      hh.push_back(h);
   }

   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   for (int c = 0; c < CHRONO_N_CHANNELS; c++)
   {
      for (int b = 0; b < CHRONO_N_BOARDS; b++)
      {
         double official_time;
         TTree* t = Get_Chrono_Tree(runNumber, TChronoChannel(b,c), official_time);
         //We might be able to use TTreeReader with a friend... this is sub optimal 
         TChrono_Event* e=new TChrono_Event();
         t->SetBranchAddress("ChronoEvent", &e);
         for (Int_t i = 0; i < t->GetEntriesFast(); ++i)
         {
            t->GetEntry(i);
            if (official_time < first_time) continue;
            if (official_time > last_time) break;
            //Loop over all time windows
            for (int j = 0; j < n_times; j++)
            {
               if (official_time > tmin[j] && official_time < tmax[j])
               {
                  for (int i = 0; i < n_chans; i++)
                  {
                     const int counts=e->GetCounts();
                     if (counts)
                     {
                        //std::cout<<t<<"\t"<<tmin[j]<<"\t"<<t-tmin[j]<<std::endl;
                        hh[i]->Fill(official_time - tmin[j], counts);
                     }
                  }
                  //This event has been written to the array... so I dont need
                  //to check the other winodws... break! Move to next SISEvent
                 break;
               }
            }
         }
         delete e;
         delete t;
      }
   }
   return hh;
}
#endif
#ifdef BUILD_AG
std::vector<TH1D*> Get_Summed_Chrono(Int_t runNumber, std::vector<TChronoChannel> chrono_chans, std::vector<TAGSpill> spills)
{
   std::vector<double> tmin;
   std::vector<double> tmax;
   for (auto & spill: spills)
   {
      if (spill.ScalerData)
      {
         tmin.push_back(spill.ScalerData->StartTime);
         tmax.push_back(spill.ScalerData->StopTime);
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return Get_Summed_Chrono(runNumber,  chrono_chans, tmin, tmax );
}
#endif

#ifdef BUILD_AG
std::vector<TH1D*> Get_Summed_Chrono(Int_t runNumber, std::vector<TChronoChannel> chrono_chans, std::vector<std::string> description, std::vector<int> dumpIndex)
{
   std::vector<TAGSpill> spills=Get_AG_Spills(runNumber, description, dumpIndex);
   return Get_Summed_Chrono( runNumber, chrono_chans, spills);
}
#endif


#ifdef BUILD_AG
std::vector<std::vector<TH1D*>> Get_Chrono(Int_t runNumber, std::vector<TChronoChannel> chrono_chan, std::vector<double> tmin, std::vector<double> tmax, double range)
{
   assert(tmin.size()==tmax.size());
   double first_time = 1E99;
   double last_time = 0;
   
   const int n_chans=chrono_chan.size();
   const size_t n_times = tmin.size();
   
   //If range is not set, calculate it
   if (range<0)
   {
      for (int i=0; i<n_times; i++)
      {
         double diff=tmax[i]-tmin[i];
         if (range<diff) range=diff;
      }
   }
   for (auto& t: tmin)
   {
      if (t < first_time) first_time = t;
   }
   for (auto& t: tmax)
   {
      //Replace negative tmax times with the end of run...
      if (t < 0) t = 1E99; //FIXME: Get total run time!
      //Find the latest tmax time
      if (last_time < t ) last_time = t;
   }


   std::vector<std::vector<TH1D*>> hh;
   for (int i=0; i<n_chans; i++)
   {
      std::vector<TH1D*> times;
      for (int j = 0; j < n_times; j++)
      {
         if (!chrono_chan[i].IsValidChannel())
            break;
         TString Title="R";
         Title+=runNumber;
         Title+=" Chrono Channel:";
         Title+=chrono_chan[i].GetChannel();
         Title+=" - ";
         Title+=Get_Chrono_Name(runNumber, chrono_chan[i]);

         //Replace this is a data base call to get the channel name
         TString name=Get_Chrono_Name(runNumber, chrono_chan[i]);
         //Title+=name.Data();
   
         TH1D* h= new TH1D( name.Data(),
                           Title.Data(),
                           gNbin,0.,tmax[j] - tmin[j] );
         times.push_back(h);
      }
      hh.push_back(times);
   }

   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   for (int i = 0; i < chrono_chan.size(); i++)
   {
      if (!chrono_chan[i].IsValidChannel())
         continue;
      double official_time;
      TTree* t = Get_Chrono_Tree(runNumber, chrono_chan[i], official_time);
      //We might be able to use TTreeReader with a friend... this is sub optimal 
      TChrono_Event* e=new TChrono_Event();
      t->SetBranchAddress("ChronoEvent", &e);
      for (Int_t event_n = 0; event_n < t->GetEntries(); ++event_n)
      {
         t->GetEntry(event_n);
         //if (official_time < first_time) continue;
         //if (official_time > last_time) break;
         //Loop over all time windows
         for (int j = 0; j < n_times; j++)
         {
            //Increase efficiency by breaking this look when we are outside of range
            if (official_time > tmin[j] && official_time < tmax[j])
            {
               const int counts=e->GetCounts();
               if (counts)
               {
                  //std::cout<<official_time<<"\t"<<tmin[j]<<"\t"<<official_time - tmin[j] <<"\t" << e->GetRunTime()<<std::endl;
                  hh.at(i).at(j)->Fill(official_time - tmin[j], counts);
               }
            }
         }
      }
      delete e;
      delete t;
   }
   return hh;
}
#endif
#ifdef BUILD_AG
std::vector<std::vector<TH1D*>> Get_Chrono(Int_t runNumber,std::vector<TChronoChannel> channels,std::vector<TAGSpill> spills)
{
   std::vector<double> tmin;
   std::vector<double> tmax;
   for (const TAGSpill& s: spills)
   {
      tmin.push_back( s.GetStartTime() );
      tmax.push_back( s.GetStopTime() );
   }
   return Get_Chrono(runNumber,channels,tmin,tmax);
}
#endif

#ifdef BUILD_AG
std::vector<std::vector<TH1D*>> Get_Chrono(Int_t runNumber,std::vector<TChronoChannel> channels, std::vector<std::string> description, std::vector<int> dumpIndex)
{
   std::vector<TAGSpill> spills = Get_AG_Spills(runNumber, description, dumpIndex);
   std::vector<double> tmin;
   std::vector<double> tmax;
   for (const TAGSpill& s: spills)
   {
      tmin.push_back( s.GetStartTime() );
      tmax.push_back( s.GetStopTime() );
   }
   return Get_Chrono(runNumber, channels, spills);
}

#endif


#ifdef BUILD_AG
TH1D* Get_Delta_Chrono(Int_t runNumber, TChronoChannel chan, Double_t tmin, Double_t tmax, Double_t PlotMin, Double_t PlotMax)
{
   if (tmax<0.) tmax=GetAGTotalRunTime(runNumber);
   double official_time;
   TTree* t=Get_Chrono_Tree(runNumber,chan,official_time);
   TChrono_Event* e=new TChrono_Event();
   TString name=Get_Chrono_Name(runNumber,chan);
   TString Title="Chrono Time between Events - Board:";
   Title+=chan.GetBoard();
   Title+=" Channel:";
   Title+=chan.GetChannel();
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
#endif
#ifdef BUILD_AG
TH1D* Get_Delta_Chrono(Int_t runNumber, TChronoChannel chan, const char* description, Int_t repetition)
{
   std::vector<TAGSpill> spill = Get_AG_Spills(runNumber,{description},{repetition});
   Double_t tmin = spill.front().GetStartTime();
   Double_t tmax = spill.front().GetStopTime();
   return Get_Delta_Chrono( runNumber, chan, tmin, tmax);
}
#endif
#ifdef BUILD_AG
TH1D* Get_Delta_Chrono(Int_t runNumber, const char* ChannelName, Double_t tmin, Double_t tmax)
{
   TChronoChannel chan = Get_Chrono_Channel(runNumber, ChannelName);
   if (chan.IsValidChannel())
      return Get_Delta_Chrono(runNumber,chan, tmin, tmax);
   return NULL;
}
#endif
#ifdef BUILD_AG
TH1D* Get_Delta_Chrono(Int_t runNumber,const char* ChannelName, const char* description, Int_t repetition)
{
   std::vector<TAGSpill> spill = Get_AG_Spills(runNumber,{description},{repetition});
   Double_t tmin = spill.front().GetStartTime();
   Double_t tmax = spill.front().GetStopTime();
   return Get_Delta_Chrono(runNumber, ChannelName, tmin, tmax);
}
#endif



#ifdef BUILD_A2
std::vector<TH1D*> Get_Summed_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax, double range )
{
   assert(tmin.size()==tmax.size());
   double first_time = 1E99;
   double last_time = 0;
   
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
   for (auto& t: tmin)
   {
      if (t < first_time) first_time = t;
   }
   for (auto& t: tmax)
   {
      //Replace negative tmax times with the end of run...
      if (t < 0) t = 1E99; //FIXME: Get total run time!
      //Find the latest tmax time
      if (last_time < t ) last_time = t;
   }

   int n_chans=SIS_Channel.size();
   std::vector<TH1D*> hh;

   TSISChannels chans(runNumber);

   for (int i=0; i<n_chans; i++)
   {
      TString Title="R";
      Title+=runNumber;
      Title+=" SIS Channel:";
      Title+=SIS_Channel[i];
      Title+=" - ";
      Title+=chans.GetDescription(SIS_Channel[i], runNumber);

      //Replace this is a data base call to get the channel name
      TString name=chans.GetDescription(SIS_Channel[i], runNumber);
      //Title+=name.Data();

      TH1D* h= new TH1D( name.Data(),
                        Title.Data(),
                        gNbin,0.,range );
      hh.push_back(h);
   }

   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   for (int sis_module_no = 0; sis_module_no < NUM_SIS_MODULES; sis_module_no++)
   {
      TTreeReader* reader=A2_SIS_Tree_Reader(runNumber, sis_module_no);
      TTreeReaderValue<TSISEvent> SISEvent(*reader, "TSISEvent");
      // I assume that file IO is the slowest part of this function... 
      // so get multiple channels and multiple time windows in one pass
      while (reader->Next())
      {
         double t = SISEvent->GetRunTime();
         if (t < first_time) continue;
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
   }
   return hh;
}
#endif
#ifdef BUILD_A2
std::vector<TH1D*> Get_Summed_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<TA2Spill> spills)
{
   std::vector<double> tmin;
   std::vector<double> tmax;
   for (auto & spill: spills)
   {
      if (spill.ScalerData)
      {
         tmin.push_back(spill.ScalerData->StartTime);
         tmax.push_back(spill.ScalerData->StopTime);
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return Get_Summed_SIS(runNumber,  SIS_Channel, tmin, tmax );
}
#endif
#ifdef BUILD_A2
std::vector<TH1D*> Get_Summed_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<std::string> description, std::vector<int> dumpIndex)
{
   std::vector<TA2Spill> spills=Get_A2_Spills(runNumber, description, dumpIndex);
   return Get_Summed_SIS( runNumber, SIS_Channel, spills);
}
#endif

#ifdef BUILD_A2
std::vector<std::vector<TH1D*>> Get_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<double> tmin, std::vector<double> tmax )
{
   assert(tmin.size()==tmax.size());
   double first_time = 1E99;
   double last_time = 0;
   
   int n_times=tmin.size();

   for (auto& t: tmin)
   {
      if (t < first_time) first_time = t;
   }
   for (auto& t: tmax)
   {
      //Replace negative tmax times with the end of run...
      if (t < 0) t = 1E99; //FIXME: Get total run time!
      //Find the latest tmax time
      if (last_time < t ) last_time = t;
   }

   int n_chans=SIS_Channel.size();

   std::vector<std::vector<TH1D*>> hh;

   TSISChannels chans(runNumber);

   for (int i=0; i<n_chans; i++)
   {
      std::vector<TH1D*> times;
      for (int j=0; j<tmax.size(); j++)
      {
         TString Title="R";
         Title+=runNumber;
         Title+=" SIS Channel:";
         Title+=SIS_Channel[i];
         Title+=" - ";
         Title+=chans.GetDescription(SIS_Channel[i], runNumber);

         //Replace this is a data base call to get the channel name
         TString name=chans.GetDescription(SIS_Channel[i], runNumber);
         name += "_";
         name += j;
         //Title+=name.Data();

         TH1D* h= new TH1D( name.Data(),
                           Title.Data(),
                           gNbin,0. ,tmax.at(j) - tmin.at(j) );
         times.push_back(h);
      }
      hh.push_back(times);
   }
   //TTreeReaders are buffered... so this is faster than iterating over a TTree by hand
   //More performance is maybe available if we use DataFrames...
   for (int sis_module_no = 0; sis_module_no < NUM_SIS_MODULES; sis_module_no++)
   {
      TTreeReader* reader=A2_SIS_Tree_Reader(runNumber, sis_module_no);
      TTreeReaderValue<TSISEvent> SISEvent(*reader, "TSISEvent");
      // I assume that file IO is the slowest part of this function... 
      // so get multiple channels and multiple time windows in one pass
      while (reader->Next())
      {
         double t=SISEvent->GetRunTime();
         if (t < first_time) continue;
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
                     hh.at(i).at(j)->Fill(t-tmin[j],counts);
                  }
               }
               //This event has been written to the array... so I dont need
               //to check the other winodws... break! Move to next SISEvent
               break;
            }
         }
      }
   }
   return hh;
}
#endif
#ifdef BUILD_A2
std::vector<std::vector<TH1D*>> Get_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<TA2Spill> spills)
{
   std::vector<double> tmin;
   std::vector<double> tmax;
   for (auto & spill: spills)
   {
      if (spill.ScalerData)
      {
         tmin.push_back(spill.ScalerData->StartTime);
         tmax.push_back(spill.ScalerData->StopTime);
      }
      else
      {
         std::cout<<"Spill didn't have Scaler data!? Was there an aborted sequence?"<<std::endl;
      }
   }
   return Get_SIS(runNumber,  SIS_Channel, tmin, tmax );
}
#endif
#ifdef BUILD_A2
std::vector<std::vector<TH1D*>> Get_SIS(Int_t runNumber, std::vector<int> SIS_Channel, std::vector<std::string> description, std::vector<int> dumpIndex)
{
   std::vector<TA2Spill> spills=Get_A2_Spills(runNumber, description, dumpIndex);
   return Get_SIS( runNumber, SIS_Channel, spills);
}
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
