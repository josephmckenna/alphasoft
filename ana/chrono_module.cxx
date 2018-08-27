// 
// chronobox 
// 
// A. Capra
//

#include "manalyzer.h"
#include "midasio.h"
#include "AgFlow.h"

#include "TTree.h"
#include "TChrono_Event.h"
#include <iostream>

#define ClockChannel 58
#define NChronoBoxes 1
#define NChannels 58
#define ClockFrequency 50E6

class ChronoFlags
{
public:
   bool fPrint = false;
};

class Chrono: public TARunObject
{
private:
  Int_t ID;
  uint32_t gClock;
  uint32_t ZeroTime[NChronoBoxes];

public:
  ChronoFlags* fFlags;
  TChrono_Event* fChronoEvent[NChronoBoxes][NChannels];
  TTree* ChronoTree[NChronoBoxes][NChannels];
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
      for (int i=0; i<NChronoBoxes; i++)
         ZeroTime[i]=0.;
      //Later split this by channel:  
      for (int box=0; box<NChronoBoxes; box++)
      {
         for (int chan=0; chan<NChannels; chan++)
         {
            fChronoEvent[box][chan] = new TChrono_Event();
            TString Name="ChronoEventTree_";
            Name+=box;
            Name+="_";
            Name+=chan;
            ChronoTree[box][chan] = new TTree(Name, "ChronoEventTree");
            ChronoTree[box][chan]->Branch("ChronoEvent", &fChronoEvent[box][chan], 32000, 0);
            ID=0;
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

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      std::cout<<"Chrono::Analyze   Event # "<<me->serial_number<<std::endl;

      if( me->event_id != 10 ) // sequencer event id
         return flow;
      me->FindAllBanks();
      std::cout<<"===================================="<<std::endl;
      std::cout<<me->HeaderToString()<<std::endl;
      std::cout<<me->BankListToString()<<std::endl;
      //Chronoboard index counts from 1
      for (Int_t BoardIndex=1; BoardIndex<NChronoBoxes+1; BoardIndex++)
      {
        char BankName[4];
        BankName[0]='C';
        BankName[1]='B';
        BankName[2]='S';
        BankName[3]='0'+BoardIndex;
        const TMBank* b = me->FindBank(BankName);
        if( b ) std::cout<<"Chrono::Analyze   BANK NAME: "<<b->name<<std::endl;
        else return flow;
        uint32_t *pdata32;
        pdata32= (uint32_t*)me->GetBankData(b);
        //const char* bkptr = me->GetBankData(b);
        int bklen = b->data_size;
        std::cout<<"bank size: "<<bklen<<std::endl;
        if( bklen > 0 )
        {
          //printf("%s\n",bkptr);
          //for (Int_t dave=0; dave<64; dave++)
          //std::cout<<pdata32[dave]<<std::endl;
          gClock=pdata32[ClockChannel]; // 
          std::cout<<pdata32[ClockChannel]<<std::endl;
          for (Int_t Chan=0; Chan<NChannels; Chan++)
          {
            if (!pdata32[Chan]) continue;
            fChronoEvent[BoardIndex-1][Chan]->Reset();
            fChronoEvent[BoardIndex-1][Chan]->SetID(ID);
            fChronoEvent[BoardIndex-1][Chan]->SetTS(gClock);
            fChronoEvent[BoardIndex-1][Chan]->SetBoardIndex(BoardIndex);
            fChronoEvent[BoardIndex-1][Chan]->SetRunTime((Double_t)gClock/ClockFrequency);
            fChronoEvent[BoardIndex-1][Chan]->SetChannel(Chan);
            fChronoEvent[BoardIndex-1][Chan]->SetCounts(pdata32[Chan]);
            ChronoTree[BoardIndex-1][Chan]->Fill();
            ID++;
          }
          std::cout<<"________________________________________________"<<std::endl;
        }
      }
      return flow;
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

