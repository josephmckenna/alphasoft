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
#include "TChronoChannelName.h"

#include "AnalysisTimer.h"

class ChronoFlags
{
public:
   bool fPrint = false;
};

class Chrono: public TARunObject
{
private:
  Int_t ID;
  uint64_t gClock[CHRONO_N_BOARDS];
  uint64_t ZeroTime[CHRONO_N_BOARDS];
  uint64_t NOverflows[CHRONO_N_BOARDS];
  uint32_t LastTime[CHRONO_N_BOARDS];; //Used to catch overflow in clock
  uint32_t LastCounts[CHRONO_N_BOARDS][CHRONO_N_CHANNELS];
  Int_t Events[CHRONO_N_BOARDS];
  
  Int_t TSID=0;
  uint32_t gTS[CHRONO_N_TS_CHANNELS];
  uint32_t gLastTS[CHRONO_N_TS_CHANNELS];
  uint64_t gFullTS[CHRONO_N_TS_CHANNELS];
  uint64_t gTSOverflows[CHRONO_N_TS_CHANNELS];
  Int_t TSEvents[CHRONO_N_BOARDS];
  
  std::vector<ChronoEvent*>* ChronoEventsFlow=NULL;
public:
  ChronoFlags* fFlags;
  TChrono_Event* fChronoEvent[CHRONO_N_BOARDS][CHRONO_N_CHANNELS];
  TTree* ChronoTree[CHRONO_N_BOARDS][CHRONO_N_CHANNELS];
  
  TChrono_Event* fChronoTS[CHRONO_N_BOARDS][CHRONO_N_TS_CHANNELS];
  TTree* ChronoTimeStampTree[CHRONO_N_BOARDS][CHRONO_N_TS_CHANNELS];
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

      //Save chronobox channel names
     TChronoChannelName* name = new TChronoChannelName();
     TString ChannelName;
     TTree* ChronoBoxChannels = new TTree("ChronoBoxChannels","ChronoBoxChannels");
     ChronoBoxChannels->Branch("ChronoChannel",&name, 32000, 0);
     for (int board=0; board<CHRONO_N_BOARDS; board++)
     {
        name->SetBoardIndex(board+1);
        for (int chan=0; chan<CHRONO_N_CHANNELS; chan++)
        {
            TString OdbPath="/Equipment/cbms0";
            OdbPath+=board+1;
            OdbPath+="/Settings/ChannelNames";
            //std::cout<<runinfo->fOdb->odbReadString(OdbPath.Data(),chan)<<std::endl;
            if (runinfo->fOdb->odbReadString(OdbPath.Data(),chan))
               name->SetChannelName(runinfo->fOdb->odbReadString(OdbPath.Data(),chan),chan);
         }
        if( fTrace )
           name->Print();
        ChronoBoxChannels->Fill();
      }
      delete name;
      for (int i=0; i<CHRONO_N_BOARDS; i++)
      {
        ZeroTime[i]=0;
        gClock[i]=0;
        NOverflows[i]=0;
        LastTime[i]=0;
      }
      
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("chrono")->cd();
      //
      for (int board=0; board<CHRONO_N_BOARDS; board++)
      {
         Events[board]=0;
         for (int chan=0; chan<CHRONO_N_CHANNELS; chan++)
         {
            fChronoEvent[board][chan] = new TChrono_Event();
            TString Name="ChronoEventTree_";
            Name+=board;
            Name+="_";
            Name+=chan;
            ChronoTree[board][chan] = new TTree(Name.Data(), "ChronoEventTree");
            ChronoTree[board][chan]->Branch("ChronoEvent", &fChronoEvent[board][chan], 32000, 0);
            ID=0;
            LastCounts[board][chan]=0;
         }
      }
      for (int board=0; board<CHRONO_N_BOARDS; board++)
      {
         TSEvents[board]=0;
         for (int chan=0; chan<CHRONO_N_TS_CHANNELS; chan++)
         {
            fChronoTS[board][chan] = new TChrono_Event();
            TString Name="TimeStampEventTree_";
            Name+=board;
            Name+="_";
            Name+=chan;
            ChronoTimeStampTree[board][chan] = new TTree(Name.Data(), "TimeStampEventTree");
            ChronoTimeStampTree[board][chan]->Branch("TimeStampEvent", &fChronoEvent[board][chan], 32000, 0);
            TSID=0;
            //uint24 inside uint32:
            gTS[board]=0;
            gLastTS[board]=0;
            //uint64:
            gFullTS[board]=0;
            gTSOverflows[board]=0;
         }
      }
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("Chrono::EndRun, run %d\n", runinfo->fRunNo);
      for (int i =0; i< CHRONO_N_BOARDS; i++)
         std::cout <<"Chronoboard["<<i<<"]"<<Events[i]<<std::endl;
      for (int i =0; i< CHRONO_N_BOARDS; i++)
         std::cout <<"ChronoboardTS["<<i<<"]"<<TSEvents[i]<<std::endl;
      for (int board=0; board<CHRONO_N_BOARDS; board++)
         {
            for (int chan=0; chan<CHRONO_N_CHANNELS; chan++)
               {
                  ChronoTree[board][chan]->Write();
                  delete ChronoTree[board][chan]; 
                  if (fChronoEvent[board][chan]) delete fChronoEvent[board][chan];
               }
         }                  
      for (int board=0; board<CHRONO_N_BOARDS; board++)
         {
            for (int chan=0; chan<CHRONO_N_TS_CHANNELS; chan++)
               {
                  ChronoTimeStampTree[board][chan]->Write();
                  delete ChronoTimeStampTree[board][chan];
                  if (fChronoTS[board][chan]) delete fChronoTS[board][chan];
               } 
         }
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

   bool UpdateChronoScalerClock(ChronoChannelEvent* e, int b)
   {
      uint32_t EventTime=e->Counts-ZeroTime[b];
      //std::cout <<"TIME CHAN:"<<(int)e->Channel<<std::endl;
      if (ZeroTime[b]==0)
      {
         std::cout <<"Zeroing time of chronoboard "<<b+1<<" at "<< EventTime<<std::endl;
         ZeroTime[b]=EventTime;
         //Chronoflow=NULL;
         //Also reject the first event... 
         return true;
      }
      else
      {
      gClock[b]=EventTime;
      
      if (gClock[b]<LastTime[b])
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
   void SaveChronoScaler(ChronoChannelEvent* e, int b)
   {
      Double_t RunTime=(Double_t)gClock[b]/CHRONO_CLOCK_FREQ;
      Int_t Chan=(Int_t)e->Channel;
      uint32_t counts=e->Counts;
      if (Chan>CHRONO_N_CHANNELS) return;
      if (!counts) return;
      //      std::cout<<"ScalerChannel:"<<Chan<<"("<<b+1<<")"<<": "<<counts<<" at "<<RunTime<<"s"<<std::endl;
      
      fChronoEvent[b][Chan]->SetID(ID);
      fChronoEvent[b][Chan]->SetTS(gClock[b]);
      fChronoEvent[b][Chan]->SetBoardIndex(b+1);
      fChronoEvent[b][Chan]->SetRunTime(RunTime);
      fChronoEvent[b][Chan]->SetChannel(Chan);
      fChronoEvent[b][Chan]->SetCounts(counts);
      ChronoEvent* CE=new ChronoEvent{RunTime,Chan,counts,b};
      ChronoEventsFlow->push_back(CE);
      //fChronoEvent[b][Chan]->Print();
      ChronoTree[b][Chan]->Fill();
      ID++;
      Events[b]++;
   }
   void SaveChronoTimeStamp(ChronoChannelEvent* e, int b)
   {
      Int_t Chan=(Int_t)e->Channel-100;
      //This TS is really just 24 bit...
      gTS[b]=e->Counts;
      gFullTS[b]=gTS[b]+gTSOverflows[b]*(1<<24);
      Double_t RunTime=(Double_t)gFullTS[b]/CHRONO_CLOCK_FREQ;
      if (gTS[b]<gLastTS[b])
      {
         gTSOverflows[b]++;
         std::cout <<"TS overflow"<<std::endl;
      }
      //std::cout<<"TSChannel:"<<Chan<<"("<<b+1<<")"<<": ts"<<gTS[b]<<" overfl:"<<gTSOverflows[b]<<" at "<<RunTime<<"s"<<std::endl;
      fChronoTS[b][Chan]->Reset();
      fChronoTS[b][Chan]->SetID(TSID);
      TSID++;
      fChronoTS[b][Chan]->SetTS(gFullTS[b]);
      fChronoTS[b][Chan]->SetBoardIndex(b+1);
      fChronoTS[b][Chan]->SetRunTime(RunTime);
      fChronoTS[b][Chan]->SetChannel(Chan);
      ChronoTimeStampTree[b][Chan]->Fill();
      gLastTS[b]=gTS[b];
      TSEvents[b]++;
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      std::cout<<"Chrono::Analyze   Event # "<<me->serial_number<<std::endl;

      if( me->event_id != 10 ) // sequencer event id
         return flow;

      gDirectory->cd("/chrono");
      ChronoEventsFlow=new std::vector<ChronoEvent*>;
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
         if( !b ) continue;
         //else 
         //std::cout<<"Chrono::Analyze   BANK NAME: "<<b->name<<std::endl;
         //std::cout<<me->HeaderToString()<<std::endl;
         int bklen = b->data_size;
         int bkread=0;
         ChronoChannelEvent* cce;
         cce=(ChronoChannelEvent*) me->GetBankData(b);
         //for (int bkit=0; bkit<(bklen/8); bkit++)
         //{
         //   std::cout <<"("<<bkit<<"/"<<bklen/8<<")"<<(uint32_t)cce[bkit].Channel<<"\t"<<cce[bkit].Counts<<std::endl;
         //}
         //return flow;
         std::cout<<"bank size: "<<bklen<<std::endl;
         if( bklen > 0 )
         {
            for (int block=0; block<(bklen/8); block++)
            {

               int Chan=(int)cce[block].Channel;
               int counts=cce[block].Counts;
               if (!counts) continue;
               if (Chan>=CHRONO_N_CHANNELS && Chan<100)
               {
                  std::cout<<"Bad Channel:"<<Chan<<": "<<counts<<" at "<<(Double_t)gClock[BoardIndex-1]/CHRONO_CLOCK_FREQ<<"s"<<std::endl;
                  continue;
               }
               if (Chan>=100+CHRONO_N_TS_CHANNELS)
               {
                  std::cout<<"Bad Channel:"<<Chan<<": "<<counts<<" at "<<(Double_t)gClock[BoardIndex-1]/CHRONO_CLOCK_FREQ<<"s"<<std::endl;
                  continue;
               }
               //Look for the scaler clock count
               if (Chan==CHRONO_CLOCK_CHANNEL)
               {
                  //Set up the gClock and check if first entry
                  UpdateChronoScalerClock(&cce[block],BoardIndex-1);
                     //if its the first event... do not put it in trees
                     //continue;
                  //Count the clock chan:
                  SaveChronoScaler(&cce[block],BoardIndex-1);
                  //Rewind and fill Scalers
                  for (int pos=block-1; CHRONO_CLOCK_CHANNEL>block-pos; pos--)
                  {
                     //Double check the right channel numbers?
                     //if (Chan<CHRONO_N_CHANNELS)
                     
                     if (pos<0) break;
                     
                     if (cce[pos].Channel==CHRONO_CLOCK_CHANNEL) break;
                     if (cce[pos].Counts>(uint32_t)-((uint16_t)-1)/2)
                     {
                        std::cout<<"Bad counts (probably underflow) in channel: "<<(int)cce[pos].Channel<<std::endl;
                        break;
                     }
                     SaveChronoScaler(&cce[pos],BoardIndex-1);
                     
                     
                  }
                  //block++;
                  continue;
               }
               if (Chan>99) SaveChronoTimeStamp(&cce[block],BoardIndex-1);
            }
         }
         //std::cout<<"________________________________________________"<<std::endl;
      }
      //Chronoflow->PrintChronoFlow();
      
      flow=new AgChronoFlow(flow,ChronoEventsFlow);
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"chrono_module");
      #endif
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

