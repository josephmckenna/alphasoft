//
// sis 
//
// JTK McKenna

#include "manalyzer.h"
#include "midasio.h"

#include "TTree.h"
#include "TMath.h"
#include "TChrono_Event.h"
#include <iostream>
#include "TChronoChannelName.h"

#include <TBufferJSON.h>
#include <fstream>

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

   uint32_t midas_start_time = -1 ;
   
   //Variables to catch the start of good data from the SISboxes
   int Overflows[NUM_SIS_MODULES]={0};
   uint LastTS[NUM_SIS_MODULES]={0};
   
   TTree* SISTree;

   bool fTrace = true;

   SIS(TARunInfo* runinfo, SISFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="sis_module";
#endif
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

      //Get the start time of the run (for TInfoSpill constructors)
      #ifdef INCLUDE_VirtualOdb_H
      midas_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      #endif
      #ifdef INCLUDE_MVODB_H
      runinfo->fOdb->RU32("Runinfo/Start time binary",(uint32_t*) &midas_start_time);
      #endif   

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
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
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
      if (event->event_id != 11)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      event->FindAllBanks();

      //void* ptr[NUM_SIS_MODULES];
      TMBank* sis_bank = {0};
      int size = {0};
    
      char bankname[] = "MCS0";
      int totalsize = 0;
      Int_t SISdiff =0;

      std::vector<TSISEvent*> SIS_Events;

      for (int i=0; i<NUM_SIS_MODULES; i++)
      {
         bankname[3] = '0' + i; 
         //size[i] = event.LocateBank(NULL,bankname,&ptr[i]);
         sis_bank = event->FindBank(bankname);
         if (!sis_bank) continue;
         size = sis_bank->data_size/4;
         totalsize+=size;
         SIS_Events.reserve(totalsize);
         (i==0)? SISdiff+=size:SISdiff-=size; 
         assert( size % NUM_SIS_CHANNELS == 0);// check SIS databank size is a multiple of 32
         
         // Copy the sis data into a TSISEvent
         for (int sis_event = 0; sis_event < size; sis_event+=NUM_SIS_CHANNELS )
         {
            TSISEvent* s=new TSISEvent();
            s->SetMidasUnixTime(event->time_stamp);
            s->SetMidasEventID(event->event_id);
            s->SetRunNumber(runinfo->fRunNo);
            s->SetSISModuleNo(i);
            unsigned long int *sis_data = (unsigned long int*) event->GetBankData(sis_bank);
            // We may want to set the gClock variable in the other function.. not this function is in the main thread and the other is a side thread
            unsigned long int clock = sis_data[0];
            gClock[i] += clock;
            for (int kk=0; kk<NUM_SIS_CHANNELS; kk++)
            {
               s->SetCountsInChannel(kk,sis_data[kk]);
            } 
           double runtime = clock2time(gClock[i],gExptStartClock[i]); 
           s->SetRunTime(runtime);
           s->SetClock(gClock[i]);

           if (i==0)
           {
              gVF48Clock += sis_data[vf48clkchan];
              s->SetVF48Clock(gVF48Clock);
           }
           SIS_Events.push_back(s);
         }
      }

      if (SIS_Events.size())
      {
         std::cout<<"Sending SIS_Events " << SIS_Events.size() <<std::endl;
         SISEventFlow* sf = new SISEventFlow(flow);
         for (TSISEvent* s: SIS_Events)
         {
            sf->Add(s);
         }
         flow = sf;
         gSisCounter++;
      }
      return flow;
   }

std::deque<TSISEvent*> gSISSyncBuffer[NUM_SIS_CHANNELS];

TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
{
      SISEventFlow* sf = new SISEventFlow(flow);
      if (!sf)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }

     //Sort SIS events here!
     int list_size_0 = sf->sis_events[0].size();
     int list_size_1 = sf->sis_events[1].size();

     if (list_size_0 != list_size_1)
     {
         for (int module = 0; module < NUM_SIS_MODULES; module++)
         {
            for (TSISEvent* s: sf->sis_events[module])
               gSISSyncBuffer[module].push_back(s);
            sf->sis_events[module].clear();  
         }
         //if (SISdiffPrev !=0 && SISdiff+ SISdiffPrev !=0 ){
         //if (SISdiff+ SISdiffPrev !=0 )
         {
            std::string warning = std::string("Unpaired SIS buffers ") + 
                                  std::to_string(sf->sis_events[0].size()/NUM_SIS_CHANNELS) + 
                                  std::string(" ") + 
                                  std::to_string(sf->sis_events[1].size()/NUM_SIS_CHANNELS) + 
                                  std::string(" diff ") + 
                                  std::to_string(gSISdiff/NUM_SIS_CHANNELS);
            std::cout << warning << "\n";
            gBadSisCounter++;
            // Joe: TODO We need a UNIX TIMESTAMP FOR THIS... grab it from a TSIS Event
            TInfoSpill* WarningSpill = new TInfoSpill(runinfo->fRunNo, midas_start_time, 0 /*event->time_stamp*/ , warning.c_str());

            TInfoSpillFlow* f = new TInfoSpillFlow(flow);
            f->spill_events.push_back(WarningSpill);
            flow = f;
         }
      }
      // If we have data in the buffer... it needs to come first
      if (gSISSyncBuffer[0].size() || gSISSyncBuffer[1].size())
      {
         for (int j=0; j<NUM_SIS_MODULES; j++)
         {
            for (TSISEvent* s: sf->sis_events[j])
            {
               gSISSyncBuffer[j].push_back(s);
            }
            //reset flow contents
            sf->sis_events[j].clear();
         }         
         while (gSISSyncBuffer[0].size() && gSISSyncBuffer[1].size())
         {
            //Check the time stamps of events here joe!
            std::cout<<"sis_module "<<__LINE__<<std::endl;

            TSISEvent* sis_event[2];
            sis_event[0] = gSISSyncBuffer[0].front();
            sis_event[1] = gSISSyncBuffer[1].front();

            if (sis_event[0]->GetClock() != sis_event[1]->GetClock())
            {
               assert(!"FML");
            }
            for (int j=0; j<NUM_SIS_MODULES; j++)
            {
               SaveToTree(runinfo,gSISSyncBuffer[j].front());
               sf->sis_events[j].push_back(gSISSyncBuffer[j].front());
               gSISSyncBuffer[j].pop_front();
            }
         }
      }
      else
      {
         for (int j=0; j<NUM_SIS_MODULES; j++)
         {
            for (size_t i=0; i<sf->sis_events[j].size(); i++)
            {
               SaveToTree(runinfo,sf->sis_events[j].at(i));
            }
         }
      }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("SIS::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n",
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
   
   void PreEndRun(TARunInfo* runinfo)
   {
      if (gBadSisCounter)
      {
         std::string WarningLine = std::string("Warning: ") +
                                   std::to_string(gBadSisCounter) + 
                                   std::string(" bad sis events out of ") + 
                                   std::to_string(gSisCounter);
         TInfoSpill* WarningSpill = new TInfoSpill(runinfo->fRunNo, 0, 0, WarningLine.c_str());

         TInfoSpillFlow* flow = new TInfoSpillFlow(NULL);
         flow->spill_events.push_back(WarningSpill);
         runinfo->AddToFlowQueue(flow);
      }
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
