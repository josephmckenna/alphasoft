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
#include "TSisEvent.h"


class SISFlags
{
public:
   bool fPrint = false;

};

class SIS: public TARunObject
{
private:
   Int_t ID;
   uint64_t gClock[NUM_SIS_MODULES];
   uint64_t gExptStartClock[NUM_SIS_MODULES];
   
   uint64_t ZeroTime[NUM_SIS_MODULES];
   uint64_t NOverflows[NUM_SIS_MODULES];
   uint32_t LastTime[NUM_SIS_MODULES]; //Used to catch overflow in clock
   uint32_t LastCounts[NUM_SIS_MODULES][NUM_SIS_CHANNELS];
   Int_t Events[NUM_SIS_MODULES];

   Int_t    SyncChannel[NUM_SIS_MODULES]; //4 is temporary... fetch from ODB in begin runs
   Double_t FirstSyncTime[NUM_SIS_MODULES];

   Int_t TSID=0;
   uint32_t gTS[NUM_SIS_MODULES];
   uint32_t gLastTS[NUM_SIS_MODULES];
   uint64_t gFullTS[NUM_SIS_MODULES];
   uint64_t gTSOverflows[NUM_SIS_MODULES];
   Int_t TSEvents[NUM_SIS_MODULES];

   //std::vector<SISEvent*>* SISEventsFlow=NULL;
public:
   SISFlags* fFlags;
   
   int gSISdiff =0;
   int SISdiffPrev =0;
   
   int gSisCounter = 0;
   int gBadSisCounter = 0;
   
   int clkchan[NUM_SIS_MODULES] = {-1};
   
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
        clkchan[j] = SISChannels->Get10MHz_clk(j);
        gClock[j]=0;
        gExptStartClock[j]=0;
      }
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

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      //std::cout<<"SIS::Analyze   Event # "<<event->serial_number<<std::endl;


      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif
      if (event->event_id != 11)
         return flow;

      event->FindAllBanks();

      //void* ptr[NUM_SIS_MODULES];
      TMBank* sis_bank[2]={0};
      int size[NUM_SIS_MODULES];
    
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
      if (!size[0]) return flow;
      if (size[0] != size[1])
      {
         //if (SISdiffPrev !=0 && SISdiff+ SISdiffPrev !=0 ){
         if (SISdiff+ SISdiffPrev !=0 )
         {
            //printf("Unpaired SIS buffers %d  %d  diff %d \n", size[0]/NUM_SIS_CHANNELS, size[1]/NUM_SIS_CHANNELS, gSISdiff/NUM_SIS_CHANNELS);
            gBadSisCounter++;
            return flow;
         }
         SISdiffPrev+=SISdiff; 
      }

      if (totalsize<=0) return flow;
      uint32_t* sis[NUM_SIS_MODULES];
      for (int j=0; j<NUM_SIS_MODULES; j++)
      {
         sis[j] = (uint32_t*)event->GetBankData(sis_bank[j]); // get pointers
         //sis[j] = (uint32_t*)&event+sis_bank[j]->data_offset; // get pointers
         
         if(gExptStartClock[j]==0 && sis[j])
         {
            gExptStartClock[j]=sis[j][clkchan[j]];  //first clock reading
         }
      
      } 
      for (int j=0; j<NUM_SIS_MODULES; j++) // loop over databanks
      {
        //uint32_t* b=(uint32_t*)sis_bank[j];
        for (int i=0; i<size[j]; i+=NUM_SIS_CHANNELS, sis[j]+=NUM_SIS_CHANNELS)
        {
           unsigned long int clock = sis[j][clkchan[j]]; // num of 10MHz clks
           gClock[j] += clock;
           double runtime=clock2time(gClock[j],gExptStartClock[j]); 
           TSisEvent* SisEvent=new TSisEvent(j,gClock[j],runtime);

           for (int kk=0; kk<NUM_SIS_CHANNELS; kk++)
           {
              SisEvent->SetCountsInChannel(kk,(int)sis[j][kk]);
           }
           //SisEvent->Print();
           runinfo->AddToFlowQueue(new SISEventFlow(NULL,SisEvent));
           
        }
      }
      
        
 
      
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
