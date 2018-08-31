// 
// chronobox 
// 
// A. Capra
// JTK McKenna

#include "manalyzer.h"
#include "midasio.h"
#include "AgFlow.h"

#include "TTree.h"
#include "TChrono_Event.h"
#include <iostream>
#include "chrono_module.h"


class ChronoFlags
{
public:
   bool fPrint = false;
};

class Chrono: public TARunObject
{
private:
  Int_t ID;
  uint64_t gClock=0;
  uint64_t ZeroTime[CHRONO_N_BOARDS];
  uint64_t NOverflows=0;
  uint32_t LastTime; //Used to catch overflow in clock
  uint32_t LastCounts[CHRONO_N_BOARDS][CHRONO_N_CHANNELS];
public:
  ChronoFlags* fFlags;
  TChrono_Event* fChronoEvent[CHRONO_N_BOARDS][CHRONO_N_CHANNELS];
  TTree* ChronoTree[CHRONO_N_BOARDS][CHRONO_N_CHANNELS];
  bool fTrace = true;
   
   Chrono(TARunInfo* runinfo, ChronoFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("Chrono::ctor!\n");
   }

   ~Chrono()
   {
      if (fTrace)
         printf("Chrono::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("Chrono::BeginRun, run %d\n", runinfo->fRunNo);
      //printf("Chrono::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      for (int i=0; i<CHRONO_N_BOXES; i++)
         ZeroTime[i]=0;
      LastTime=0;
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      //Later split this by channel:  
      for (int box=0; box<CHRONO_N_BOXES; box++)
      {
         for (int chan=0; chan<CHRONO_N_CHANNELS; chan++)
         {
            fChronoEvent[box][chan] = new TChrono_Event();
            TString Name="ChronoEventTree_";
            Name+=box;
            Name+="_";
            Name+=chan;
            ChronoTree[box][chan] = new TTree(Name, "ChronoEventTree");
            ChronoTree[box][chan]->Branch("ChronoEvent", &fChronoEvent[box][chan], 32000, 0);
            ID=0;
            LastCounts[box][chan]=0;
         }
      }
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("Chrono::EndRun, run %d\n", runinfo->fRunNo);
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("Chrono::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }


struct ChronoChannelEvent {
  uint8_t Channel;
  uint32_t Counts;
};


   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      //std::cout<<"Chrono::Analyze   Event # "<<me->serial_number<<std::endl;

      if( me->event_id != 10 ) // sequencer event id
         return flow;
         
      AgChronoFlow* Chronoflow =new AgChronoFlow(flow);
      //me->FindAllBanks();
      //std::cout<<"===================================="<<std::endl;
      //std::cout<<me->HeaderToString()<<std::endl;
      //std::cout<<me->BankListToString()<<std::endl;
      //Chronoboard index counts from 1
      for (Int_t BoardIndex=1; BoardIndex<CHRONO_N_BOARDS+1; BoardIndex++)
      {
         char BankName[4];
         BankName[0]='C';
         BankName[1]='B';
         BankName[2]='S';
         BankName[3]='0'+BoardIndex;
         const TMBank* b = me->FindBank(BankName);
         if( !b ) return flow;
         //else std::cout<<"Chrono::Analyze   BANK NAME: "<<b->name<<std::endl;
         //std::cout<<me->HeaderToString()<<std::endl;
         ChronoChannelEvent *cce;
         cce= (ChronoChannelEvent*)me->GetBankData(b);
         int bklen = b->data_size;
         //std::cout<<"bank size: "<<bklen<<std::endl;
         if( bklen > 0 )
         {
            uint32_t EventTime=cce[bklen/8-1].Counts-ZeroTime[BoardIndex-1];
            if (ZeroTime[BoardIndex-1]==0)
            {
              std::cout <<"Zeroing time of chronoboard"<<BoardIndex<<" at "<< EventTime<<std::endl;
              ZeroTime[BoardIndex-1]=EventTime;
              //Also reject the first event... 
              break;
            }
            else
            {
              gClock=EventTime;
            }
            if (EventTime<LastTime) NOverflows++;
            LastTime=EventTime;
            gClock+=NOverflows*((uint32_t)-1);
            
            for (int ChanEvent=0; ChanEvent<(bklen/8); ChanEvent++)
            {
               Int_t Chan=(Int_t)cce[ChanEvent].Channel;
               uint32_t counts=cce[ChanEvent].Counts;
               if (!counts) continue;
               Double_t RunTime=(Double_t)gClock/CHRONO_CLOCK_FREQ;
               //std::cout<<"Channel:"<<Chan<<": "<<counts<<" at "<<RunTime<<"s"<<std::endl;
               fChronoEvent[BoardIndex-1][Chan]->Reset();
               fChronoEvent[BoardIndex-1][Chan]->SetID(ID);
               fChronoEvent[BoardIndex-1][Chan]->SetTS(gClock);
               fChronoEvent[BoardIndex-1][Chan]->SetBoardIndex(BoardIndex);
               fChronoEvent[BoardIndex-1][Chan]->SetRunTime(RunTime);
               fChronoEvent[BoardIndex-1][Chan]->SetChannel(Chan);
               fChronoEvent[BoardIndex-1][Chan]->SetCounts(counts);
               Chronoflow->SetRunTime(Chan,RunTime);
               Chronoflow->SetCounts(Chan,counts);
               //fChronoEvent[BoardIndex-1][Chan]->Print();
               ChronoTree[BoardIndex-1][Chan]->Fill();
               ID++;
               //LastCounts[BoardIndex-1][Chan]=pdata32[Chan];
             }
         }
         //std::cout<<"________________________________________________"<<std::endl;
      }
      return Chronoflow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("Chrono::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class ChronoFactory: public TAFactory
{
public:
   ChronoFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("ChronoFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      printf("ChronoFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("ChronoFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new Chrono(runinfo, &fFlags);
   }
};

static TARegister tar(new ChronoFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

