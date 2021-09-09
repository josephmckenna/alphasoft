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

//Made these things global... Very illegal.
TMBank* fTempSISBank[2] = {0};
int fTempSize[2] = {0};
//Lets make a global deque - we'll just save the MIDAS event for now. 
//I don't know how we can possibly even really pair but we'll see. 
//It might also be too late after the dump has finished being analysed...
std::deque<TMEvent*> unpairedSISEvents;

void pushbackevent(TMEvent* event)
{
   unpairedSISEvents.push_back(new TMEvent(*event));
}

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
         
         
         /*int jmax = event->banks.size();
         std::cout << "Size of the event banks for this event = " << jmax << std::endl;
         std::cout << "They are called: ";
         for(int j=0; j<jmax; j++)
         {
            std::cout << "" << event->banks[j].name << ", ";
         }
         std::cout << std::endl;*/


         if (!sis_bank[i]) continue;
         size[i]=sis_bank[i]->data_size/4;
         totalsize+=size[i];
         (i==0)? SISdiff+=size[i]:SISdiff-=size[i]; 
         assert( size[i] % NUM_SIS_CHANNELS == 0);// check SIS databank size is a multiple of 32
      }

      ///Lukas additions - DOesn't work, might work if fTemp and fSis are saved more globally. Lets just try that for now. 
      if (size[0] != size[1] )
      {
         std::cout << "Event mismatch was found. Lets investigate." << std::endl;
         if(size[0] > 0 || size[1] > 0)
         {
            //Lets save this event to the deque no matter what.
            pushbackevent(event);


            std::cout << "At least one size was non zero. Is there a previous event? If so match, if not save." << std::endl;
            //std::cout << "Previous event? " << fTempSISBank[0] << " and " << fTempSISBank[1] << std::endl;
            if(fTempSISBank[0] || fTempSISBank[1])
            {
               std::cout << "Previous event exists, lets match up." << std::endl;
               if(size[0] < size[1])
               {
                  sis_bank[0] = fTempSISBank[0];
                  size[0] = fTempSize[0];
               }
               if(size[1] < size[0])
               {
                  sis_bank[1] = fTempSISBank[1];
                  size[1] = fTempSize[1];
               }
               std::cout << "Now that we've matched the event lets wipe temp event." << std::endl;
               fTempSISBank[0] = NULL;
               fTempSISBank[1] = NULL;
               fTempSize[0] = 0;
               fTempSize[1] = 1;
               std::cout << "Deleted." << std::endl;
               
               
               //Instead of all this we need to find a way to check the deque if there is a match pair it up and if not go down and add to the deque. 
            }
            else
            {
               std::cout << "Previous event does not exist, lets save" << std::endl;
               //Instead  of this lets add to a deque.
               fTempSISBank[0] = sis_bank[0];
               fTempSISBank[1] = sis_bank[1];
               fTempSize[0] = size[0];
               fTempSize[1] = size[1];
               std::cout << "Saved." << std::endl;
            }
         }
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
            TInfoSpill* WarningSpill = new TInfoSpill(runinfo->fRunNo, midas_start_time, event->time_stamp, warning.c_str());

            TInfoSpillFlow* f = new TInfoSpillFlow(flow);
            f->spill_events.push_back(WarningSpill);
            return f;
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
           s->SetMidasEventID(mf->MidasEventID);
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
