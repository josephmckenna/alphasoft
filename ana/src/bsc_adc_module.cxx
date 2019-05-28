#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>

#include "TMath.h"
#include "TH1D.h"
#include "TH2D.h"
#include "AnalysisTimer.h"

#include "TBarEvent.hh"

class BscFlags
{
public:
   bool fPrint = false;
};

class BscModule: public TARunObject
{
private:
   float time_trig = 0;
   int fCounter = 0;
   int pedestal_length = 100;
   int threshold = 1400;
   int bscMap[64][4];
   TBarEvent* BarEvent;

   std::map<int,std::map<std::string,double>> fBarHits;
   std::map<int,std::map<std::string,int>> fBarTime;

public:
   BscFlags* fFlags;

private:
   TH1D *hBsc=NULL;
   TH1D *hBsc_Ampl_Top=NULL;
   TH1D *hBsc_Ampl_Bot=NULL;
   TH1D *hBsc_Plot = NULL;
   TH1D *hBsc_Signal = NULL;
   TH1D *hBsc_Reverse = NULL;
   TH1D *hBsc_Delayed = NULL;
   TH1D *hBsc_CFD=NULL;
   TH1D *hBsc_AmplRange_Bot=NULL;
   TH1D *hBsc_AmplRange_Top=NULL;
   TH1D *hBsc_Occupency=NULL;
   TH1D *hBsc_Amplitude[8][16];
   TDirectory* NMA_BscModule = NULL;
   TH1D *hBsc_Zed=NULL;
   TH1D *hBsc_TimeVsTop = NULL;
   TH1D *hBsc_TimeVsBot =NULL;
   TH1D *hBsc_TimeTopAndBot =NULL;
   TH2D *hBsc_TimeVsBar = NULL;
   TH1D *hBsc_TimeDiff = NULL;
public:

   BscModule(TARunInfo* runinfo, BscFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      ResetBarHits();
   }

   ~BscModule()
   {   }


   void BeginRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("bsc")->cd();

      hBsc=new TH1D("hBsc","BSc: All Bars Amplitude", 10000,0.0, 10000);
      hBsc_Ampl_Top=new TH1D("hBsc_Ampl_Top","BSc: Top Bars Amplitude", 1000,0.0, 10000);
      hBsc_Ampl_Bot=new TH1D("hBsc_Ampl_Bop","BSc: Bot Bars Amplitude", 1000,0.0, 10000);
      hBsc_Plot= new TH1D("hBsc_Plot","one channel", 700, 0, 700);
      hBsc_Signal=new TH1D("hBsc_Signal","Test Signal Plot", 700, 0, 700);
      hBsc_Reverse=new TH1D("hBsc_Reverse","Test Signal Reverse Plot", 700, 0, 700);
      hBsc_Delayed=new TH1D("hBsc_Delayed","Test Signal Delayed Plot", 700, 0, 700);
      hBsc_CFD=new TH1D("hBsc_CFD","Test Signal CFD Plot", 700, 0, 700);
      hBsc_Occupency= new TH1D("hBsc_Occupency","Bars Occupency", 64, 0, 64);
      hBsc_AmplRange_Bot=new TH1D("hBsc_AmplRange_Bot","BSc: Bot Bars Amplitude When Top Trigger", 100,0.0, 10000);
      hBsc_AmplRange_Top=new TH1D("hBsc_AmplRange_Top","BSc: Top Bars Amplitude When Bot Trigger", 100,0.0, 10000);
      hBsc_Zed=new TH1D("hBsc_Zed", "Altitude from ADC time difference", 46000,-2300.,2300.);
      hBsc_TimeVsTop=new TH1D("hBsc_TimeVsTop", "ADC time on TOP", 700,0,700);
      hBsc_TimeVsBot=new TH1D("hBsc_TimeVsBot", "ADC Time on BOT", 700,0,700);
      hBsc_TimeTopAndBot=new TH1D("hBsc_TimeAndBot", "ADC Time on TOP and BOT", 700,0,700);
      hBsc_TimeVsBar=new TH2D("hBsc_TimeVsBar", "ADC Time on TOP and BOT", 700,0,700, 63, 0, 63);
      hBsc_TimeDiff=new TH1D("hBsc_TimeDiff", "ADC Time Diff", 20,-10,10);

      char name[256];
      for (int mod=0; mod<8; mod++)
         {
            for(int chan=0; chan<16; chan++)
               {
                  sprintf(name,"Bsc_amplitude%d_%d",mod,chan);
                  hBsc_Amplitude[mod][chan]= new TH1D(name,name,10000, 0, 10000);
               }
         }

      //Chargement Bscint map
      TString mapfile=getenv("AGRELEASE");
      mapfile+="/ana/bscint/";
      mapfile+="bscint.map";
      std::ifstream fbscMap(mapfile.Data());
      if(fbscMap)
         {
            std::string comment;
            getline(fbscMap, comment);
            for(int bar_ind=0; bar_ind<64; bar_ind++)
               {
                  fbscMap >> bscMap[bar_ind][0] >> bscMap[bar_ind][1] >> bscMap[bar_ind][2] >> bscMap[bar_ind][3];
               }
            fbscMap.close();
         }
   }

   void EndRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->Write();
      delete hBsc;
      delete hBsc_Ampl_Top;
      delete hBsc_Ampl_Bot;
      delete hBsc_Plot;
      delete hBsc_Signal;
      delete hBsc_Reverse;
      delete hBsc_Delayed;
      delete hBsc_CFD;
      delete hBsc_Occupency;
      delete hBsc_AmplRange_Bot;
      delete hBsc_AmplRange_Top;

      for (int mod=0; mod<8; mod++)
         {
            for(int chan=0; chan<16; chan++)
               {
                  delete hBsc_Amplitude[mod][chan];
               }
         }
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }


   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Bscint Module: Analyze, run %d, counter %d\n", runinfo->fRunNo, fCounter);
      const AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;
      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif      
      const AgEvent* e = ef->fEvent;
      const Alpha16Event* data = e->a16;

      if( !data )
         {
            std::cout<<"BscModule::AnalyzeFlowEvent(...) No Alpha16Event in AgEvent # "
                     <<e->counter<<std::endl;
            return flow;
         }

      if( fFlags->fPrint )
         printf("BscModule::AnalyzeFlowEvent(...) Event number is : %d \n", data->counter);


      TBarEvent* BarEvent = AnalyzeBars(data);
      BarEvent->SetID(e->counter);
      BarEvent->SetRunTime(e->time);

      std::cout<<"BscModule::AnalyzeFlowEvent(...) has "<<BarEvent->GetNBars()<<" hits"<<std::endl;

      flow = new AgBarEventFlow(flow, BarEvent);

#ifdef _TIME_ANALYSIS_
      if (TimeModules) flow=new AgAnalysisReportFlow(flow,"bscint_adc_module",timer_start);
#endif
      return flow;
   }

   TBarEvent* AnalyzeBars(const Alpha16Event* data)
   {
      std::vector<Alpha16Channel*> channels = data->hits;
      ResetBarHits();

      for(unsigned int i = 0; i < channels.size(); ++i)
         {
            auto& ch = channels.at(i);   // Alpha16Channel*
            if( ch->adc_chan >= 16 ) continue; // it's AW
            if( ch->bsc_bar < 0 )
               {
                  //std::cerr<<"BscModule::AnalyzeBars() Error Bar number"<<std::endl;
                  continue;
               }

            // CALCULATE BASELINE
            double baseline(0.);
            for(int b = 0; b < pedestal_length; b++) baseline += ch->adc_samples.at( b );
            baseline /= double(pedestal_length);
            //std::cout<<"BscModule::AnalyzeBars() pedestal is "<<baseline<<" for channel "<<ch->bcs_bar<<std::endl;

            // CALCULATE PEAK HEIGHT
            auto maxpos = std::max_element(ch->adc_samples.begin(), ch->adc_samples.end());
            double max = double(*maxpos) - baseline;
            //std::cout<<"BscModule::AnalyzeBars() pedestal is "<<baseline<<" for channel "<<ch->bsc_bar<<std::endl;

            if( max > threshold ) // interesting signal
               {
                  int bar = -1;
                  std::string end = "";
                  if( ch->bsc_bar < 64 ) // KO mapping, can go into its own oject
                     {
                        bar = ch->bsc_bar;
                        end = "bot";
                     }
                  else
                     {
                        bar = ch->bsc_bar - 64;
                        end = "top";
                     }
                  if( bar < 0 )
                     {
                        std::cerr<<"BscModule::AnalyzeBars() Something went very wrong"<<std::endl;
                        continue;
                     }


                  std::map<std::string,double> h = fBarHits.at(bar);
                  h[end]=max;
                  fBarHits[bar] = h;
                  //std::cout<<"BscModule::AnalyzeBars() pedestal is "<<baseline<<" for channel "<<ch->bsc_bar<< " and max is " << h[end]<<std::endl;


                  // check if there is a difference between NM and KO mapping
                  if( bscMap[bar][0] != bar )
                     std::cerr<<"BscModule::AnalyzeBars() Something went VERY VERY wrong with the mapping"<<std::endl;
                  if( bscMap[bar][1] != ch->adc_module )
                     std::cout<<"BscModule::AnalyzeBars() ADC<-/->BSC map NM: "<< bscMap[bar][1] <<" KO: "<< ch->adc_module<<std::endl;
                  if( end == "top" && bscMap[bar][2] != ch->adc_chan )
                     std::cout<<"BscModule::AnalyzeBars() ADC CHAN<-/->BSC map TOP NM: "<< bscMap[bar][2] <<" KO: "<<ch->adc_chan<<std::endl;
                  if( end == "bot" && bscMap[bar][3] != ch->adc_chan )
                     std::cout<<"BscModule::AnalyzeBars() ADC CHAN <-/->BSC map BOT NM: "<< bscMap[bar][3] <<" KO: "<<ch->adc_chan<<std::endl;


                  // CFD Time
                  int delay=7;
                  int CFD_signal[ch->adc_samples.size()];
                  int max_CFD=0;
                  int ind_max=0;
                  for(uint ii=0; ii<ch->adc_samples.size()-delay; ii++)
                     {
                        CFD_signal[ii]= baseline-ch->adc_samples[ii]+ch->adc_samples[ii+delay];
                        //printf(" %d %d \n", ii,  CFD_signal[ii]);
                        if(CFD_signal[ii]>max_CFD)
                           {
                              max_CFD=CFD_signal[ii];
                              ind_max=ii;
                           }
                     }

                  int ii= ind_max;
                  while(CFD_signal[ii]>0)
                     ii=ii+1;
                  int ind_time=ii;

                  //TIME WITH SIGNAL ABOVE THRESHOLD TRIGGER
                  //if(1)
                     {
                        double tFactor=0.2;
                        int ii=0;
                        while((ch->adc_samples[ii]-baseline)<(tFactor*max))
                           ii++;

                        std::map<std::string,int> t = fBarTime.at(bar);
                        t[end]=ii;
                        fBarTime[bar] = t;

                        //printf("-------------------> Balise 1 \n");
                        if(end=="top")
                           {
                              hBsc_TimeVsTop->Fill(ii);
                           }
                        if(end=="bot")
                           {
                              hBsc_TimeVsBot->Fill(ii);
                            }
                        hBsc_TimeTopAndBot->Fill(ii);
                        hBsc_TimeVsBar->Fill(ii,bar);
                     }


                  //TIME WITH CDF LINEAR INTERPOLLATION
                  if(0)
                     {
                        double y1= CFD_signal[ind_time-1];
                        double y2= CFD_signal[ind_time+1];

                        double slope=y2-y1;
                        double intercept=y2-slope*(ind_time+1);
                        double CFD_time=-intercept/slope;


                        std::map<std::string,int> t = fBarTime.at(bar);
                        t[end]=CFD_time;
                        fBarTime[bar] = t;
                     }

               }
         }

      TBarEvent* BarEvent = new TBarEvent();
      for(int bar_index = 0; bar_index < 64; ++bar_index)
         {
            std::map<std::string,double> hit = fBarHits.at(bar_index);
            std::map<std::string,int> time = fBarTime.at(bar_index);
            double max_top = hit["top"];
            double max_bot = hit["bot"];
            int time_top=time["top"];
            int time_bot=time["bot"];

            //Get Zed
            double speed=TMath::C()*1.e-6;
            double cFactor=1.58;
            double ZedADC=((speed/cFactor) * double(time_bot-time_top)*10.)*0.5;

            //std::cout<<"BscModule::AnalyzeBars() Bar: "<<bar_index<<" max top: "<<max_top<<" max top: "<<hit["top"]<<" time top: "<<time_top<<" time bot: "<<time_bot<<" Zed: "<<ZedADC<<std::endl;

            if( max_top > 0. && max_bot > 0. )
               {
                  BarEvent->AddADCHit(bar_index,max_top,max_bot, time_top, time_bot, ZedADC);
                  hBsc_Zed->Fill(ZedADC);
                  hBsc_TimeDiff->Fill(time_bot-time_top);
                  if( fFlags->fPrint )
                     std::cout<<"BscModule::AnalyzeBars() Bar: "<<bar_index<<" max top: "<<max_top<<" max bot: "<<max_bot<<" time top: "<<time_top<<" time bot: "<<time_bot<<" Zed: "<<ZedADC<<std::endl;
                  hBsc_Ampl_Top->Fill(max_top);
                  hBsc_Ampl_Bot->Fill(max_bot);
                  hBsc_Occupency->Fill(bar_index);
               }
         }
      return BarEvent;
   }

   void ResetBarHits()
   {
      for(int bar_index = 0; bar_index < 64; ++bar_index)
         {
            std::map<std::string,double> h;
            h["top"]=-1.;
            h["bot"]=-1.;
            fBarHits[bar_index]=h;
            std::map<std::string,int> t;
            t["top"]=-1;
            t["bot"]=-1;
            fBarTime[bar_index]=t;
         }
   }

};


class BscModuleFactory: public TAFactory
{
public:
   BscFlags fFlags;
public:
   void Help()
   {   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("BscModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--bscprint")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      printf("BscModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("BscModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new BscModule(runinfo, &fFlags);
   }
};

static TARegister tar(new BscModuleFactory);


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
