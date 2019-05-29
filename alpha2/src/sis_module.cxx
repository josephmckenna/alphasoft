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
   //TSIS_Event* fSISEvent[NUM_SIS_MODULES][NUM_SIS_CHANNELS];
   TTree* SISTree[NUM_SIS_MODULES][NUM_SIS_CHANNELS];

   //TSIS_Event* fSISTS[NUM_SIS_MODULES][NUM_SIS_CHANNELS];
   TTree* SISTimeStampTree[NUM_SIS_MODULES][NUM_SIS_CHANNELS];
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

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SIS::BeginRun, run %d\n", runinfo->fRunNo);
      //printf("SIS::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

   
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


struct SISChannelEvent {
  uint8_t Channel;
  uint32_t Counts;
};

#if 0
   bool UpdateSISScalerClock(SISChannelEvent* e, int b)
   {
      uint32_t EventTime=e->Counts-ZeroTime[b];
      //std::cout <<"TIME CHAN:"<<(int)e->Channel<<std::endl;
      if (ZeroTime[b]==0)
      {
         std::cout <<"Zeroing time of SISboard "<<b+1<<" at "<< EventTime<<std::endl;
         ZeroTime[b]=EventTime;
         //SISflow=NULL;
         //Also reject the first event...
         return true;
      }
      else
      {
         gClock[b]=EventTime;
         if (gClock[b]<LastTime[b])// && gClock[b]<100000)
         {
            NOverflows[b]++;
            //std::cout <<"OVERFLOWING"<<std::endl;
         }
         //      std::cout <<"TIME DIFF   "<<gClock[b]-LastTime[b] <<std::endl;
         LastTime[b]=gClock[b];
         gClock[b]+=NOverflows[b]*(TMath::Power(2,32)); //-1?
         //gClock[b]+=NOverflows[b]*((uint32_t)-1);
         //std::cout <<"TIME"<<b<<": "<<EventTime<<" + "<<NOverflows[b]<<" = "<<gClock[b]<<std::endl;
      }
      //Is not first event... (has been used)
      return false;
   }
   void SaveSISScaler(SISChannelEvent* e, int b)
   {
      Double_t RunTime=(Double_t)gClock[b]/SIS_CLOCK_FREQ;
      Int_t Chan=(Int_t)e->Channel;
      uint32_t counts=e->Counts;

      //Check for sync
      if (Chan==SyncChannel[b])
         if (FirstSyncTime[b]<0)
            FirstSyncTime[b]=RunTime;

      //Start official time at first Sync pulse


      if (FirstSyncTime[b]>0 && FirstSyncTime[0]>0)
      {
         RunTime=RunTime-FirstSyncTime[b]+FirstSyncTime[0];
      }
      if (Chan>SIS_N_CHANNELS) return;
      if (!counts) return;
      if (fFlags->fPrint)
         if (counts>100000  && Chan != SIS_CLOCK_CHANNEL)
            std::cout <<"CORR COUNTS!("<<Chan<<"):  "<<counts<<std::endl;
      //      std::cout<<"ScalerChannel:"<<Chan<<"("<<b+1<<")"<<": "<<counts<<" at "<<RunTime<<"s"<<std::endl;
      fSISEvent[b][Chan]->SetID(ID);
      fSISEvent[b][Chan]->SetTS(gClock[b]);
      fSISEvent[b][Chan]->SetBoardIndex(b+1);
      fSISEvent[b][Chan]->SetRunTime(RunTime);
      //fSISEvent[b][Chan]->SetOfficialTime(OT);
      fSISEvent[b][Chan]->SetChannel(Chan);
      fSISEvent[b][Chan]->SetCounts(counts);
      SISEvent* CE=new SISEvent{RunTime,Chan,counts,b};
      SISEventsFlow->push_back(CE);
      //fSISEvent[b][Chan]->Print();
      SISTree[b][Chan]->Fill();
      LastCounts[b][Chan]=counts;
      ID++;
      Events[b]++;
   }
   void SaveSISTimeStamp(SISChannelEvent* e, int b)
   {
      Int_t Chan=(Int_t)e->Channel-100;
      //This TS is really just 24 bit...
      gTS[b]=e->Counts;
      gFullTS[b]=gTS[b]+gTSOverflows[b]*(1<<24);
      Double_t RunTime=(Double_t)gFullTS[b]/SIS_CLOCK_FREQ;
      if (gTS[b]<gLastTS[b])
      {
         gTSOverflows[b]++;
         //std::cout <<"TS overflow"<<std::endl;
      }
      //std::cout<<"TSChannel:"<<Chan<<"("<<b+1<<")"<<": ts"<<gTS[b]<<" overfl:"<<gTSOverflows[b]<<" at "<<RunTime<<"s"<<std::endl;
      fSISTS[b][Chan]->Reset();
      fSISTS[b][Chan]->SetID(TSID);
      TSID++;
      fSISTS[b][Chan]->SetTS(gFullTS[b]);
      fSISTS[b][Chan]->SetBoardIndex(b+1);
      fSISTS[b][Chan]->SetRunTime(RunTime);
      fSISTS[b][Chan]->SetChannel(Chan);
      SISTimeStampTree[b][Chan]->Fill();
      gLastTS[b]=gTS[b];
      TSEvents[b]++;
   }

#endif
   //Variables to catch the start of good data from the SISboxes
   int Overflows[NUM_SIS_MODULES]={0};
   uint LastTS[NUM_SIS_MODULES]={0};
   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      //std::cout<<"SIS::Analyze   Event # "<<me->serial_number<<std::endl;

      if( me->event_id != 10 ) // sequencer event id
         return flow;
      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"SIS_module",timer_start);
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
