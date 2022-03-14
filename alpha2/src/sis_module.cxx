//
// sis 
//
// JTK McKenna

#include "manalyzer.h"
#include "midasio.h"

#include "TTree.h"
#include "TMath.h"

#include <iostream>

#include <TBufferJSON.h>
#include <fstream>

#include "A2Flow.h"
#include "AnalysisFlow.h"
#include "TSISChannels.h"
#include "TSISEvent.h"

VectorRecycler<TSISBufferEvent> SISModuleFlow::gTSISBufferEventRecycleBin(1000,"TSIS Buffer");
VectorRecycler<TSISEvent> SISEventFlow::gTSISEventRecycleBin(1000,"TSISEvent");

//This works but obviously this stuff is global which is very bad.
std::deque<TMEvent*> unpairedSISEvents;
void pushbackevent(TMEvent* event) //This is a global function. We in python now bois.
{
   unpairedSISEvents.push_back(new TMEvent(*event));
}

class SISFlags
{
public:
   bool fPrint = false;
   bool fSaveSIS = true;
};

class UnmatchedBuffer
{
   private:
      std::deque<std::vector<TSISBufferEvent>> fBuffer;
      int fPosition;
   public:
      UnmatchedBuffer()
      {
         fPosition = 0;
      }
      void AddData(std::vector<TSISBufferEvent>& data)
      {
         fBuffer.emplace_back(std::move(data));
      }
      const TSISBufferEvent* GetFront()
      {
         if (fPosition >= fBuffer.front().size())
         {
            SISModuleFlow::gTSISBufferEventRecycleBin.RecycleVector(fBuffer.front());
            fBuffer.pop_front();
            fPosition = 0;
            return GetFront();
         }
         return &fBuffer.front().at(fPosition);
      }
      void EventUsed()
      {
         fPosition++;
      }
      size_t size() const
      {
         size_t size = 0;
         for (int i = 0; i < fBuffer.size(); i++)
         {
            size += fBuffer[i].size();
         }
         return size - fPosition;
      }
      bool empty() const
      {
         for (int i = 0; i < fBuffer.size(); i++)
            if (!fBuffer[i].empty())
               return false;
         return true;
      }
};
   


class SIS: public TARunObject
{
private:
   uint64_t gClock[NUM_SIS_MODULES];
   uint64_t gExptStartClock[NUM_SIS_MODULES];
   //Used in 
   uint64_t gVF48Clock;
   Int_t ID;
   TTree* SISEventTree[NUM_SIS_MODULES]   = {nullptr};

public:
   SISFlags* fFlags;
   
   int gSISdiff =0;
   int SISdiffPrev =0;
   
   int gSisCounter = 0;
   int gBadSisCounter = 0;
   
   TSISChannel clkchan[NUM_SIS_MODULES];
   TSISChannel vf48clkchan;

   uint32_t midas_start_time = -1 ;
   
   //Variables to catch the start of good data from the SISboxes
   int Overflows[NUM_SIS_MODULES]={0};
   uint LastTS[NUM_SIS_MODULES]={0};

   bool fTrace = true;

   UnmatchedBuffer fUnmatchedBuffer[NUM_SIS_MODULES];

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
   
   void SaveToTree(TARunInfo* runinfo, TSISEvent* s)
   {
         if (!fFlags->fSaveSIS) return;
         int i = s->GetSISModule();
         assert(i >= 0 && i < NUM_SIS_MODULES);
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         runinfo->fRoot->fOutputFile->cd();
         if (!SISEventTree[i])
         {
            std::string TreeName = "SIS" + std::to_string(i) + std::string("Tree");
            SISEventTree[i] = new TTree(TreeName.c_str(),TreeName.c_str());
         }
         TBranch* b_variable = SISEventTree[i]->GetBranch("TSISEvent");
         if (!b_variable)
            SISEventTree[i]->Branch("TSISEvent",&s);
         else
            b_variable->SetAddress(&s);
         SISEventTree[i]->Fill();
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
            std::string warning = std::string("Unpaired SIS buffers ") + 
                                  std::to_string(size[0]/NUM_SIS_CHANNELS) + 
                                  std::string(" ") + 
                                  std::to_string(size[1]/NUM_SIS_CHANNELS) + 
                                  std::string(" diff ") + 
                                  std::to_string(gSISdiff/NUM_SIS_CHANNELS);
            std::cout << warning << "\n";
            gBadSisCounter++;
            // Dont report this in the spill log
            //TInfoSpill* WarningSpill = new TInfoSpill(runinfo->fRunNo, midas_start_time, event->time_stamp, warning.c_str());

            //TInfoSpillFlow* f = new TInfoSpillFlow(flow);
            //f->spill_events.push_back(WarningSpill);
            ////Return flow here to disable unpaired SIS events recovery (added Sept 2021)
            ////return f;
            //flow = f;
         }
         SISdiffPrev+=SISdiff; 
      }
      
      if (totalsize<=0) return flow;
      gSisCounter++;
      SISModuleFlow* mf=new SISModuleFlow(flow);
      mf->MidasEventID=event->event_id;
      mf->MidasTime=event->time_stamp;
      for (int j=0; j<NUM_SIS_MODULES; j++)
         mf->AddData(j,event->GetBankData(sis_bank[j]),size[j]);
      flow=mf;

      return flow;
   }

TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SISModuleFlow* mf=flow->Find<SISModuleFlow>();
      if (!mf)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }

      SISEventFlow* sf=new SISEventFlow(flow);
      //Put TSISBufferEvent flow into a matching buffer
      for (int j=0; j<NUM_SIS_MODULES; j++)
      {
         fUnmatchedBuffer[j].AddData(mf->fSISBufferEvents[j]);

         if(gExptStartClock[j]==0 && !fUnmatchedBuffer[j].empty())
         {
            gExptStartClock[j] = fUnmatchedBuffer[j].GetFront()->fCounts[clkchan[j].fChannel];  //first clock reading
         }
      }
      //Lets get the number of events in the shortest queue (these should be pairs of SIS events)
      int range = 0;
      if (fUnmatchedBuffer[0].size() < fUnmatchedBuffer[1].size())
         range = fUnmatchedBuffer[0].size();
      else
         range = fUnmatchedBuffer[1].size();
      //std::cout<<"Buffer:" << fUnmatchedBuffer[0].size() <<"\t"<< fUnmatchedBuffer[1].size() << std::endl;
         
      for (int j=0; j<NUM_SIS_MODULES; j++) // loop over databanks
      {
        //uint32_t* b=(uint32_t*)sis_bank[j];
        if (!range) continue;
        for (int i = 0; i < range; i++)
        {
           const TSISBufferEvent* event = fUnmatchedBuffer[j].GetFront();
          
           // event->Print();
           unsigned long int clock = event->fCounts[clkchan[j].fChannel]; // num of 10MHz clks
           gClock[j] += clock;
           double runtime=clock2time(gClock[j],gExptStartClock[j]); 
           //SISModule* module=new SISModule(j,gClock[j],runtime);
           sf->sis_events[j].emplace_back(*event);
           TSISEvent* s = &sf->sis_events[j].back();
           s->SetMidasUnixTime(mf->MidasTime);
           s->SetMidasEventID(mf->MidasEventID);
           s->SetRunNumber(runinfo->fRunNo);
           s->SetRunTime(runtime);
           s->SetClock(gClock[j]);
           if (j==0)
           {
              gVF48Clock+=s->GetCountsInChannel(vf48clkchan);
              s->SetVF48Clock(gVF48Clock);
           }
           fUnmatchedBuffer[j].EventUsed();
           // s->Print();
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
            SaveToTree(runinfo,&sf->sis_events[j].at(i));
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
