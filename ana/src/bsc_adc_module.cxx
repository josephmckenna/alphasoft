#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>

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
   int threshold = 1500;
   int bscMap[64][4];

   std::map<int,std::map<std::string,double>> fBarHits;

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

      char name[256];
      for (int mod=0; mod<8; mod++)
         {
            for(int chan=0; chan<16; chan++)
               {
                  sprintf(name, "mod%dch%d", mod, chan);
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
      // delete hBsc;
      // delete hBsc_Ampl_Top;
      // delete hBsc_Ampl_Bot;
      // delete hBsc_Plot;
      // delete hBsc_Signal;
      // delete hBsc_Reverse;
      // delete hBsc_Delayed;
      // delete hBsc_CFD;
      // delete hBsc_Occupency;
      // delete hBsc_AmplRange_Bot;
      // delete hBsc_AmplRange_Top;

      // for (int mod=0; mod<8; mod++)
      //    {
      //       for(int chan=0; chan<16; chan++)
      //          {
      //             delete hBsc_Amplitude[mod][chan];
      //          }
      //    }
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
      if (TimeModules) flow=new AgAnalysisReportFlow(flow,"bscint_adc_module");
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
                  std::cerr<<"BscModule::AnalyzeBars() Error Bar number"<<std::endl;
                  continue;
               }
            
            // CALCULATE BASELINE
            double baseline(0.);
            for(int b = 0; b < pedestal_length; b++) baseline += ch->adc_samples.at( b );
            baseline /= double(pedestal_length);
            //std::cout<<"BscModule::AnalyzeBars() pedestal is "<<baseline<<" for channel "<<ch->bsc_bar<<std::endl;

            // CALCULATE PEAK HEIGHT
            auto maxpos = std::max_element(ch->adc_samples.begin(), ch->adc_samples.end());
            double max = double(*maxpos) - baseline;

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
                  
                  // check if there is a difference between NM and KO mapping
                  if( bscMap[bar][0] != bar ) 
                     std::cerr<<"BscModule::AnalyzeBars() Something went VERY VERY wrong with the mapping"<<std::endl;
                  if( bscMap[bar][1] != ch->adc_module )
                     std::cout<<"BscModule::AnalyzeBars() ADC<-/->BSC map NM: "<< bscMap[bar][1] <<" KO: "<< ch->adc_module<<std::endl;
                  if( end == "top" && bscMap[bar][2] != ch->adc_chan )
                     std::cout<<"BscModule::AnalyzeBars() ADC CHAN<-/->BSC map TOP NM: "<< bscMap[bar][2] <<" KO: "<<ch->adc_chan<<std::endl;
                  if( end == "bot" && bscMap[bar][3] != ch->adc_chan )
                     std::cout<<"BscModule::AnalyzeBars() ADC CHAN <-/->BSC map BOT NM: "<< bscMap[bar][3] <<" KO: "<<ch->adc_chan<<std::endl;
               }
         }

      TBarEvent* BarEvent = new TBarEvent();
      for(int bar_index = 0; bar_index < 64; ++bar_index)
         {
            std::map<std::string,double> hit = fBarHits.at(bar_index);         
            double max_top = hit["top"];
            double max_bot = hit["bot"];
            if( max_top > 0. && max_bot > 0. ) 
               {
                  BarEvent->AddADCHit(bar_index,max_top,max_bot);
                  if( fFlags->fPrint )
                     std::cout<<"BscModule::AnalyzeBars() Bar: "<<bar_index<<" max top: "<<max_top<<" max bot: "<<max_bot<<std::endl;
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
