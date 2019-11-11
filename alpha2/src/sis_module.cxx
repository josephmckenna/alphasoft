//
// sis 
//
// JTK McKenna

#include "manalyzer.h"
#include "midasio.h"
#include "AgFlow.h"

#include "TTree.h"
#include "TMath.h"
#include "TChrono_Event.h"
#include <iostream>
#include "chrono_module.h"
#include "TChronoChannelName.h"

#include <TBufferJSON.h>
#include <fstream>
#include "AnalysisTimer.h"

#include "A2Flow.h"
#include "TSISChannels.h"
#include "TSISEvent.h"

class SISFlags
{
public:
   bool fPrint = false;
   bool fSaveSIS = true;
};

class SIS: public TARunObject
{
private:
   uint64_t gClock[NUM_SIS_MODULES];
   uint64_t gExptStartClock[NUM_SIS_MODULES];
   //Used in 
   uint64_t gVF48Clock;
   Int_t ID;
   TTree* SISEventTree   = NULL;
   
public:
   SISFlags* fFlags;
   
   int gSISdiff =0;
   int SISdiffPrev =0;
   
   int gSisCounter = 0;
   int gBadSisCounter = 0;
   
   int clkchan[NUM_SIS_MODULES] = {-1};
   int vf48clkchan=-1;
   
   //Variables to catch the start of good data from the SISboxes
   int Overflows[NUM_SIS_MODULES]={0};
   uint LastTS[NUM_SIS_MODULES]={0};
   
   TTree* SISTree;

   bool fTrace = true;

   SIS(TARunInfo* runinfo, SISFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("SIS::ctor!\n");
   }

   ~SIS()
   {
      if (fTrace)
         printf("SIS::dtor!\n");
   }



double clock2time(unsigned long int clock){
  const double freq = 10000000.0; // 10 MHz AD clk
  
 return clock/freq;
}

double clock2time(unsigned long int clock, unsigned long int offset ){
  const double freq = 10000000.0; // 10 MHz AD clk
  
  return (clock - offset)/freq;
}


   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SIS::BeginRun, run %d\n", runinfo->fRunNo);
      //printf("SIS::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      TSISChannels* SISChannels=new TSISChannels( runinfo->fRunNo );
      for (int j=0; j<NUM_SIS_MODULES; j++) 
      {
        clkchan[j]  = SISChannels->Get10MHz_clk(j);
        vf48clkchan = SISChannels->Get_20MHz_VF48clk();
        gClock[j]=0;
        gExptStartClock[j]=0;
      }
      
      gVF48Clock=0;
      ID=0;
      delete SISChannels;
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SIS::EndRun, run %d\n", runinfo->fRunNo);
  
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SIS::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }
   
   void SaveToTree(TARunInfo* runinfo,TSISEvent* s)
   {
         if (!fFlags->fSaveSIS) return;
         #ifdef HAVE_CXX11_THREADS
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         #endif
         runinfo->fRoot->fOutputFile->cd();
         if (!SISEventTree)
            SISEventTree = new TTree("SISEventTree","SISEventTree");
         TBranch* b_variable = SISEventTree->GetBranch("TSISEvent");
         if (!b_variable)
            SISEventTree->Branch("TSISEvent","TSISEvent",&s,16000,1);
         else
            SISEventTree->SetBranchAddress("TSISEvent",&s);
         SISEventTree->Fill();
   }
   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {

      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif
      if (event->event_id != 11)
         return flow;

      event->FindAllBanks();

      //void* ptr[NUM_SIS_MODULES];
      TMBank* sis_bank[2]={0};
      int size[NUM_SIS_MODULES]={0};
    
      char bankname[] = "MCS0";
      int totalsize = 0;
      Int_t SISdiff =0;

      for (int i=0; i<NUM_SIS_MODULES; i++)
      {
         bankname[3] = '0' + i; 
         //size[i] = event.LocateBank(NULL,bankname,&ptr[i]);
         sis_bank[i] = event->FindBank(bankname);
         if (!sis_bank[i]) continue;
         size[i]=sis_bank[i]->data_size/4;
         totalsize+=size[i];
         (i==0)? SISdiff+=size[i]:SISdiff-=size[i]; 
         assert( size[i] % NUM_SIS_CHANNELS == 0);// check SIS databank size is a multiple of 32
      }
      //if (!size[0]) return flow;
      if (size[0] != size[1])
      {
         //if (SISdiffPrev !=0 && SISdiff+ SISdiffPrev !=0 ){
         if (SISdiff+ SISdiffPrev !=0 )
         {
            //printf("Unpaired SIS buffers %d  %d  diff %d \n", size[0]/NUM_SIS_CHANNELS, size[1]/NUM_SIS_CHANNELS, gSISdiff/NUM_SIS_CHANNELS);
            gBadSisCounter++;
            //return flow;
         }
         SISdiffPrev+=SISdiff; 
      }
      
      if (totalsize<=0) return flow;
      
      SISModuleFlow* mf=new SISModuleFlow(NULL);
      mf->MidasTime=event->time_stamp;
      for (int j=0; j<NUM_SIS_MODULES; j++)
         mf->AddData(j,event->GetBankData(sis_bank[j]),size[j]);
      flow=mf;
      //runinfo->AddToFlowQueue(mf);
      
        
 
      
      #ifdef _TIME_ANALYSIS_
        if (TimeModules) flow=new AgAnalysisReportFlow(flow,"SIS_module(unpack)",timer_start);
      #endif
      return flow;
   }

TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      #ifdef _TIME_ANALYSIS_
         clock_t timer_start=clock();
      #endif
      SISModuleFlow* mf=flow->Find<SISModuleFlow>();
      if (!mf)
         return flow;
      
      
      SISEventFlow* sf=new SISEventFlow(flow);
      int size[NUM_SIS_MODULES];
      uint32_t* sis[NUM_SIS_MODULES];
      for (int j=0; j<NUM_SIS_MODULES; j++)
      {
         sis[j] = (uint32_t*)mf->xdata[j]; // get pointers
         size[j] = mf->xdata_size[j];
         if(gExptStartClock[j]==0 && sis[j])
         {
            gExptStartClock[j]=sis[j][clkchan[j]];  //first clock reading
         }
      }
      for (int j=0; j<NUM_SIS_MODULES; j++) // loop over databanks
      {
        //uint32_t* b=(uint32_t*)sis_bank[j];
        if (!size[j]) continue;
        for (int i=0; i<size[j]; i+=NUM_SIS_CHANNELS, sis[j]+=NUM_SIS_CHANNELS)
        {
           unsigned long int clock = sis[j][clkchan[j]]; // num of 10MHz clks
           gClock[j] += clock;
           double runtime=clock2time(gClock[j],gExptStartClock[j]); 
           //SISModule* module=new SISModule(j,gClock[j],runtime);
           TSISEvent* s=new TSISEvent();
           s->SetMidasUnixTime(mf->MidasTime);
           s->SetRunNumber(runinfo->fRunNo);
           s->SetRunTime(runtime);
           s->SetClock(gClock[j]);
           s->SetSISModuleNo(j);
           if (j==0)
           {
              gVF48Clock+=sis[j][vf48clkchan];
              s->SetVF48Clock(gVF48Clock);
           }
           for (int kk=0; kk<NUM_SIS_CHANNELS; kk++)
           {
              s->SetCountsInChannel(kk,sis[j][kk]);
           }
           //SisEvent->Print();
           sf->sis_events[j].push_back(s);
           
           //runinfo->AddToFlowQueue(new SISEventFlow(NULL,SisEvent));
        }
      }
      flow=sf;
      //I am totally done with the Module Flow... lets free some ram now
      mf->Clear();

      for (int j=0; j<NUM_SIS_MODULES; j++)
      {
         for (size_t i=0; i<sf->sis_events[j].size(); i++)
         {
            SaveToTree(runinfo,sf->sis_events[j].at(i));
         }
      }
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"sis_module",timer_start);
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("SIS::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n",
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class SISFactory: public TAFactory
{
public:
   SISFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("SISFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--printcorruption")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("SISFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("SISFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new SIS(runinfo, &fFlags);
   }
};

static TARegister tar(new SISFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
