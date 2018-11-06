#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <set>
#include <algorithm>
#include <future>
#include <string>

#include <TTree.h>
#include <TClonesArray.h>
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TMath.h"

#include "SignalsType.h"
//#include "tinyspline.h"

#include "AnalysisTimer.h"

#include "TStoreEvent.hh"
#include "TBarEvent.hh"

class BscFlags
{
public:
   bool fPrint = false;
};

using namespace std;

class BscModule: public TARunObject
{
private:
   
   float time_trig = 0;
   int fCounter = 0;
   TH1D *hBsc_Amplitude[8][16];
   int bscMap[64][4];
 
public:
   BscFlags* fFlags;
   TTree* BarEventTree = NULL;
   TBarEvent* BarEvent = NULL;
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
   TDirectory* NMA_BscModule = NULL;




   Alpha16Channel *channels_CFD[8][16];
   BscModule(TARunInfo* runinfo, BscFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
   }

   ~BscModule()
   {
   }
 

   void BeginRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      
      BarEvent = new TBarEvent();
      BarEventTree = new TTree("BarEventTree", "BarEventTree");
      BarEventTree->Branch("BarEventTree", &BarEvent, 32000, 0);

      gDirectory->mkdir("NMA_BscModule")->cd();
      hBsc=new TH1D("hBsc","BSc: All Bars Amplitude", 10000,0.0, 10000);
      hBsc_Ampl_Top=new TH1D("hBsc_Ampl_Top","BSc: Top Bars Amplitude", 10000,0.0, 10000);
      hBsc_Ampl_Bot=new TH1D("hBsc_Ampl_Bop","BSc: Bot Bars Amplitude", 10000,0.0, 10000);
      hBsc_Plot= new TH1D("hBsc_Plot","one channel", 700, 0, 700);
      hBsc_Signal=new TH1D("hBsc_Signal","Test Signal Plot", 700, 0, 700);
      hBsc_Reverse=new TH1D("hBsc_Reverse","Test Signal Reverse Plot", 700, 0, 700);
      hBsc_Delayed=new TH1D("hBsc_Delayed","Test Signal Delayed Plot", 700, 0, 700);
      hBsc_CFD=new TH1D("hBsc_CFD","Test Signal CFD Plot", 700, 0, 700);
      hBsc_Occupency= new TH1D("hBsc_Occupency","Bars Occupency", 68, 0, 68);
      hBsc_AmplRange_Bot=new TH1D("hBsc_AmplRange_Bot","BSc: Bot Bars Amplitude When Top Trigger", 100,0.0, 10000); 
      hBsc_AmplRange_Top=new TH1D("hBsc_AmplRange_Top","BSc: Top Bars Amplitude When Bot Trigger", 100,0.0, 10000);

      char name[256];

      for (int mod=0; mod<8; mod++)
         {
            for(int chan=0; chan<16; chan++)
               {
                  sprintf(name, "mod%dch%d", mod, chan);
                  hBsc_Amplitude[mod][chan]= new TH1D(name,name,10000, 0, 10000);
                  channels_CFD[mod][chan]=new Alpha16Channel;
                  //*channels_CFD[mod][chan]=0;
               }
         }


      //Chargement Bscint map
      ifstream fbscMap("bscint.map", ios::in);
      
      if(fbscMap)
         {
            //int* bscMap[64][4];
            string comment;
            getline(fbscMap, comment);
            for(int bar_ind=0; bar_ind<64; bar_ind++)
               {
                  fbscMap >> bscMap[bar_ind][0] >> bscMap[bar_ind][1] >> bscMap[bar_ind][2] >> bscMap[bar_ind][3];
                  
               }
            fbscMap.close();
         }

      // affichage bscMap
      if (fFlags->fPrint)
         for(int bar_ind=0; bar_ind<64; bar_ind++)
            printf(" ligne %d : %d %d %d %d \n", bar_ind,  bscMap[bar_ind][0], bscMap[bar_ind][1], bscMap[bar_ind][2], bscMap[bar_ind][3]);

      
   }


   void EndRun(TARunInfo* runinfo)
   {
      BarEventTree->Write();

      delete BarEvent;
      delete BarEventTree;
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
                  delete channels_CFD[mod][chan];
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
      
      const AgEvent* e = ef->fEvent;
      const Alpha16Event* data = e->a16; 

      if( !data ) 
         {
            std::cout<<"Bscint_adc_Module::AnalyzeFlowEvent(...) No Alpha16Event in AgEvent # "
                     <<e->counter<<std::endl;
            return flow;
         }
      else
         if (fFlags->fPrint)
            printf("NMA-> Event number is : %d \n", data->counter);

      BarEvent->Reset();
      BarEvent->SetID(e->counter);
      BarEvent->SetRunTime(e->time);

      //int channelCounter=0;

      Alpha16Channel *channels[8][16]; //declaration tableau 2D de waveforms.
      GetBscChannels(e->a16, channels);

      int *baseline[8][16]={NULL};
      int baseline_length=100;
      GetBaseline(channels, baseline, baseline_length);

      double* channels_max[8][16];
      GetMaxChannel(channels, baseline, channels_max);






      runinfo->fRoot->fOutputFile->cd();
      

      // plotting bars amplitude
      for(int ii=0; ii<8; ii++)
         {
            for(int jj=0; jj<16; jj++)
               {
                  hBsc->Fill(*channels_max[ii][jj]);
                  hBsc_Amplitude[ii][jj]->Fill(*channels_max[ii][jj]);
               }
         }



      
      // GetEvent
      int *barEvent[64];
      int threshold = 1500;

      GetEvent(threshold, barEvent, channels_max);

      

      //Display occupency per bars
      for(int ii=0; ii<64; ii++)
         {
            //printf("Bars %d : %d \n", ii,*Event[ii]);
            if(*barEvent[ii]>0)
               hBsc_Occupency->Fill(ii);
         }

      
      // Get CFD time from a signal
      /* double fraction=0.5;
      int delay= 7;
      GetCFD(channels_CFD, channels, baseline, channels_max, threshold, delay, fraction);
      */ 
     


      //Look for amplitude range on one side when trig on the other side
      int real_mod=0;
      for(int mod=0; mod<8; mod++)
         {
            for(int chan=0; chan<16; chan++)
               {
                  if(*channels_max[mod][chan]>threshold)
                     {
                        for(int bar_ind=0; bar_ind<64; bar_ind++)
                           {
                              //printf("moduletest !!! \n");
                              real_mod=mod+9;
                              if(mod==6)
                                 real_mod=18;

                              if(bscMap[bar_ind][1]==real_mod)
                                 {
                                    if (fFlags->fPrint)
                                       printf("module is : %d \n", real_mod);
                                    if(bscMap[bar_ind][2]==chan) //if trigger come from Top
                                       {
                                          if (fFlags->fPrint)
                                             printf("Channel %d is from top : looking at channel %d from bottom \n", chan, bscMap[bar_ind][3]);
                                          if(*channels_max[mod][bscMap[bar_ind][3]]<*channels_max[mod][chan])
                                             hBsc_AmplRange_Bot->Fill(*channels_max[mod][bscMap[bar_ind][3]]);
                                          
                                       }
                                    if(bscMap[bar_ind][3]==chan) //if trigger come from Bot
                                       {
                                          if (fFlags->fPrint)
                                             printf("Channel %d is from bot : looking at channel %d from Top \n", chan, bscMap[bar_ind][3]);
                                          if(*channels_max[mod][bscMap[bar_ind][2]]<*channels_max[mod][chan])
                                             hBsc_AmplRange_Top->Fill(*channels_max[mod][bscMap[bar_ind][2]]);
                                          
                                       }
                                 }
                           }
                     }
               }
         }



      //Display one particular channel
      
      int mod_plot = 3;
      int chan_plot = 5;
      for(uint ii=0; ii<channels[mod_plot][chan_plot]->adc_samples.size(); ii++)
         hBsc_Plot->Fill(ii, channels[mod_plot][chan_plot]->adc_samples[ii]);
     
      
      /******** ******/

      flow = new AgBarEventFlow(flow, BarEvent);
      BarEventTree->Fill();
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"bscint_adc_module");
      #endif
      return flow;
   }





   void GetBaseline(Alpha16Channel* channels[8][16], int* baseline[8][16], int baseline_length)
   {
      
      for(int ii=0; ii<8; ii++)
         {
            for(int jj=0; jj<16; jj++)
               {
                  baseline[ii][jj]= new int;
                  *baseline[ii][jj]=0;
                  for(int kk=0; kk<baseline_length; kk++)
                     *baseline[ii][jj]=*baseline[ii][jj]+channels[ii][jj]->adc_samples[kk]/baseline_length;
                      
               }
         }
   
   }


   void GetBscChannels(Alpha16Event* data, Alpha16Channel* channels[8][16])
   {
      
      for(uint ind_hit=0; ind_hit<data->hits.size(); ind_hit++)
         {
            Alpha16Channel *c= data->hits[ind_hit];
            if(c->adc_chan < 16)
               {
                  int ind_module=0;
                  int ind_chan=0;

                  if(c->adc_module <17)
                     ind_module=c->adc_module-9;
                  else
                     ind_module=6;

                  ind_chan=c->adc_chan;
                  channels[ind_module][ind_chan]=new Alpha16Channel;
                  *channels[ind_module][ind_chan]=*c;
               }
         }

   }

   void GetMaxChannel(Alpha16Channel* channels[8][16],int* baseline[8][16], double* channels_max[8][16])
   {

      for(int ii=0; ii<8; ii++)
         {
            for(int jj=0; jj<16; jj++)
               {
                  channels_max[ii][jj] = new double;
                  *channels_max[ii][jj]=0;
                  int max_local=0;
                  int a=0;
                  for(uint kk=0; kk<channels[ii][jj]->adc_samples.size(); kk++)
                     {
                        a = channels[ii][jj]->adc_samples[kk];
                        if(a > max_local)
                           max_local = a;

                     }
                  *channels_max[ii][jj]= max_local - *baseline[ii][jj];
                  //printf("module %d, channel %d, max = %d baseline = %d \n", ii, jj, max, *baseline[ii][jj]);
               }
         } 
   }

   void GetEvent(int threshold, int *barEvent[64], double* channels_max[8][16])
   {
      for(int bar_ind=0; bar_ind<64; bar_ind ++)
         {
            barEvent[bar_ind]= new int;
            *barEvent[bar_ind]=0;
            
            int module=bscMap[bar_ind][1]-9;
            if(bscMap[bar_ind][1]>17)
               module=6;
            
  
            
            int top_chan=bscMap[bar_ind][2];
            int bot_chan=bscMap[bar_ind][3];
            hBsc_Ampl_Top->Fill(*channels_max[module][top_chan]); 
            hBsc_Ampl_Bot->Fill(*channels_max[module][bot_chan]);
            
            if(*channels_max[module][top_chan]>threshold && *channels_max[module][bot_chan]>threshold)
            {
               *barEvent[bar_ind]=1;
               BarEvent->AddADCHit(bar_ind, *channels_max[module][top_chan], *channels_max[module][bot_chan]);
            }

         }
   }

   
   /* void GetCFD(Alpha16Channel* channels_CFD[8][16], Alpha16Channel* channels[8][16],int* baseline[8][16], double* channels_max[8][16], int threshold, int delay, int fraction)
   {
      int reverseSignal[700];
      int delayedSignal[700];
      int CFDSignal[700];

      for(int mod=0; mod<8; mod++)
         {
            for(int chan=0; chan<16; chan++)
               {
                  if(*channels_max[mod][chan] > threshold)
                     {
                        for(int ii=0; ii<700-delay; ii++)
                           {
                              reverseSignal[ii]=- fraction * (-*baseline[mod][chan]+channels[mod][chan]->adc_samples[ii]);
                              delayedSignal[ii]=-*baseline[mod][chan]+channels[mod][chan]->adc_samples[ii+delay];
                              *channels_CFD[mod][chan]->adc_samples[ii]=delayedSignal[ii]+reverseSignal[ii];

                              CFDSignal[ii]=delayedSignal[ii]+reverseSignal[ii];
                              hBsc_Reverse->Fill(ii, reverseSignal[ii]);
                              hBsc_Delayed->Fill(ii, delayedSignal[ii]);
                              hBsc_Signal->Fill(ii, channels[mod][chan]->adc_samples[ii]);
                              hBsc_CFD->Fill(ii,channels_CFD[mod][chan]->adc_samples[ii]);
                           }
                     }
               }
         }
   }
*/ 
                     
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
