//
// Module to generate friend trees with 'Official' cross calibrated 
// time between all modules (EVB and chronoboxes)
// I.E Convert 'RunTime' to 'Official Time'
// JTK McKENNA
//




#include "manalyzer.h"
#include "midasio.h"
#include "AgFlow.h"
#include "RecoFlow.h"

#include "TTree.h"
#include "TChrono_Event.h"
#include <iostream>
#include "TChronoChannelName.h"

class OfficialTimeFlags
{
public:
   bool fPrint = false;
   bool fNoSync= false;
   bool fLoadJsonChannelNames = false;
   TString fLoadJsonFile="";
};


class OfficialTime: public TARunObject
{
private:
   std::vector<double> TPCts;
   //double TPCZeroTime; Unused
   
   int tpc_channel=-1;
   int tpc_board=-1;
   std::vector<double> Chrono_TPC;

   int ChronoSyncChannel[CHRONO_N_BOARDS]={-1,-1};
   std::vector<double> ChronoSyncTS[CHRONO_N_BOARDS];

   std::deque<double> ChronoEventRunTime[CHRONO_N_BOARDS][CHRONO_N_CHANNELS];
   // std::vector<ChronoEvent*>* ChronoEventsFlow=NULL; Unused

public:
   OfficialTimeFlags* fFlags;

   double TPC_TimeStamp;
   TTree* TPCOfficial;

   double Chrono_Timestamp[CHRONO_N_BOARDS][CHRONO_N_CHANNELS];
   TTree* ChronoOfficial[CHRONO_N_BOARDS][CHRONO_N_CHANNELS];

   bool fTrace = true;

   OfficialTime(TARunInfo* runinfo, OfficialTimeFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="Official Time";
#endif
      if (fTrace)
         printf("OfficialTime::ctor!\n");
   }

   ~OfficialTime()
   {
      if (fTrace)
         printf("OfficialTime::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OfficialTime::BeginRun, run %d\n", runinfo->fRunNo);
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      
      TPCOfficial=new TTree("StoreEventOfficialTime","StoreEventOfficialTime");
      TPCOfficial->Branch("OfficialTime",&TPC_TimeStamp, 32000, 0);
      gDirectory->cd("/chrono");
      for (int board=0; board<CHRONO_N_BOARDS; board++)
      {
         for (int chan=0; chan<CHRONO_N_CHANNELS; chan++)
         {
            TString Name="ChronoEventTree_";
            Name+=board;
            Name+="_";
            Name+=chan;
            Name+="OfficialTime";
            ChronoOfficial[board][chan] = new TTree(Name.Data(), "ChronoEventTree");
            ChronoOfficial[board][chan]->Branch("OfficialTime",&Chrono_Timestamp[board][chan],32000,0);
         }
      }
      
      
      TChronoChannelName* n=NULL;
      
      
      
      tpc_channel=-1;
      tpc_board=-1;
      for (int board=0; board<CHRONO_N_BOARDS; board++)
      {
         //
         
         
         TString SyncName="CHRONO_SYNC";
         //Exact name:
         //SyncName+="_";
         //SyncName+=board+1;
         if (fFlags->fLoadJsonChannelNames)
         {
            n=new TChronoChannelName(fFlags->fLoadJsonFile,board);
         }
         else
         {
            //Read chrono channel names from ODB (default behaviour)
            n=new TChronoChannelName(runinfo->fOdb,board);
         }
         
         int channel=n->GetChannel(SyncName, false);
         if (channel>=0) ChronoSyncChannel[board]=channel;
      
         channel=n->GetChannel("TPC_TRIG", true);
         if (channel>0){ tpc_board=board; tpc_channel=channel; }
         delete n;
         n=NULL;
      }
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OfficialTime::EndRun, run %d\n", runinfo->fRunNo);
      //Flush out all un written timestamps
      FlushTPCTime();
      for (int b=0; b<CHRONO_N_BOARDS; b++)
      {
         if (!ChronoSyncTS[b].empty())
            //Add an extra sync time artificially on final flush (so we flush all events)
            ChronoSyncTS[b].push_back(ChronoSyncTS[b].back()+5.);
      }
      FlushChronoTime();
      TPCOfficial->Write();
      for (int b=0; b<CHRONO_N_BOARDS; b++)
      {
         for (int c=0; c<CHRONO_N_CHANNELS; c++)
         {
            ChronoOfficial[b][c]->Write();
         }
      }
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OfficialTime::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   void TPCMatchTime()
   {
      uint ChronoEvents=Chrono_TPC.size();
      uint TPCEvents=TPCts.size();
      if (ChronoEvents==0 || TPCEvents==0)
         return;
      if (TPCEvents>1000 && ChronoEvents>1000)
      {
         FlushTPCTime(500);
         //Clean up other vector
         TPCts.erase(TPCts.begin(),TPCts.begin()+500);
         Chrono_TPC.erase (Chrono_TPC.begin(),Chrono_TPC.begin()+500);
      }
      //Find smaller arrays
      int Events;
      if (ChronoEvents>TPCEvents)
         Events=TPCEvents;
      else
         Events=ChronoEvents;
      if (fFlags->fPrint)
         {
            std::cout <<"CALIB SIZE"<<ChronoEvents<<"\t"<<TPCEvents<<"   "<<Events<<std::endl;
            for (int  i=0; i<Events-1; i++)
               {
                  //std::cout<<"CALIB   "<<i<<":"<<Chrono_TPC.at(i)<<" - "<<TPCts.at(i)<<" = "<< Chrono_TPC[i]-TPCts[i]<<std::endl;
                  std::cout<<"CALIB   "<<i<<":"<<Chrono_TPC[i]<<" - "<<TPCts[i]<<" = "<< Chrono_TPC[i]-TPCts[i]<<std::endl;
               }
         }
   }

   void FlushTPCTime(int nToFlush=-1)
   {
      gDirectory->cd("/");
      if (nToFlush<0)
         nToFlush=TPCts.size();
      if (fFlags->fPrint)
         {
            std::cout <<"Flushing TPC time ("<<nToFlush<<" events)"<<std::endl;
         }
      if (Chrono_TPC.size()==0)
         {
            std::cout<<"NO TPC timestamps in chronobox..."<<std::endl;
            return;
         }
      for (int i=0; i<nToFlush; i++)
         {
            //Use spline of time here, not the vector:
            //TPC_TimeStamp=Chrono_TPC.at(i);
            TPC_TimeStamp=Chrono_TPC[i];
            TPCOfficial->Fill();
         }
   }

   void ChronoMatchTime()
   {
      uint ChronoSyncs[CHRONO_N_BOARDS];
      for (int i=0; i<CHRONO_N_BOARDS; i++)
      {
         ChronoSyncs[i]=ChronoSyncTS[i].size();
         if (ChronoSyncs[i]==0) return;
         if (ChronoSyncs[i]<1000) return;
      }
      FlushChronoTime(500);

   }

   void FlushChronoTime(int nToFlush=-1)
   {
      gDirectory->cd("/chrono");
      for (int b=0; b<CHRONO_N_BOARDS; b++)
      {
         uint ChronoSyncs=ChronoSyncTS[b].size();
         if (ChronoSyncTS[0].size()<ChronoSyncs) ChronoSyncs=ChronoSyncTS[0].size();
         if (nToFlush<0)
         {
            nToFlush=0;
            for (int flush=0; flush<CHRONO_N_CHANNELS; flush++)
              nToFlush+=ChronoEventRunTime[b][flush].size();;
         }
         //std::cout <<"Flushing Chrono time ("<<nToFlush<<" events)"<<std::endl;
         for (int c=0; c<CHRONO_N_CHANNELS;c++)
         {
            uint RTsize=ChronoEventRunTime[b][c].size();
            if (ChronoEventRunTime[b][c].size()==0) continue;
            uint lastpos=0;
            uint loop=nToFlush;
            if ((uint)nToFlush>RTsize) loop=RTsize;
            for (uint i=0; i<loop; i++)
            {
               if (i>RTsize) continue;
               //Find n sync
               for (uint s=lastpos; s<ChronoSyncs; s++)
               {
                  if (ChronoEventRunTime[b][c].front()>ChronoSyncTS[b].at(s)) continue;
                  lastpos=s;
                  //std::cout<<"Flush at "<<i<<"-"<<s<<"\t"<<ChronoEventRunTime[b][c].size()<<":"<<ChronoEventRunTime[b][c].front()<<"-"<<ChronoSyncTS[b].at(s)<<"+"<<ChronoSyncTS[0].at(s)<<std::endl;
                  Chrono_Timestamp[b][c]=ChronoEventRunTime[b][c].front()-ChronoSyncTS[b].at(s)+ChronoSyncTS[0].at(s);
                  ChronoOfficial[b][c]->Fill();
                  ChronoEventRunTime[b][c].pop_front();
                  break;
               }
            }
         }
      }
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      if (fFlags->fNoSync)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      if( 0 )
         std::cout<<"OfficialTime::Analyze   Event # "<<me->serial_number<<std::endl;

      //if( me->event_id != 10 ) // sequencer event id
      //   return flow;
      //AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      //if( !SigFlow ) return flow;
      
      AgChronoFlow* ChronoFlow = flow->Find<AgChronoFlow>();
      if (ChronoFlow)
         {
            std::vector<ChronoEvent*>* ce=ChronoFlow->events;
            for (uint i=0; i<ce->size(); i++)
               {
                  ChronoEvent* e=ce->at(i);
                  ChronoEventRunTime[e->ChronoBoard][e->Channel].push_back(e->RunTime);
                  //if (e->Channel==CHRONO_SYNC_CHANNEL)
                  
                  if (e->Channel==ChronoSyncChannel[e->ChronoBoard])
                  {
                     //std::cout<<"SYNC!"<<e->Channel<<"-"<<e->ChronoBoard<<" : "<<ChronoSyncChannel[e->ChronoBoard]<<std::endl;
                     ChronoSyncTS[e->ChronoBoard].push_back(e->RunTime);
                  }
                  //if TPC trigger... add it too
                  if (e->Channel==tpc_channel && e->ChronoBoard==tpc_board)
                     {
                        Chrono_TPC.push_back(e->RunTime);
                        //if (Chrono_TPC.size()==1) TPCZeroTime=e->RunTime();
                     }
               }
            ChronoMatchTime();
         }
      AgEventFlow *ef = flow->Find<AgEventFlow>();
      if (ef && ef->fEvent)
      {
         AgEvent* age = ef->fEvent;
         TPCts.push_back(age->time);
         TPCMatchTime();
      }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("OfficialTime::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class OfficialTimeFactory: public TAFactory
{
public:
   OfficialTimeFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("OfficialTimeFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--printcalib")
            fFlags.fPrint = true;
         if (args[i] == "--nosync")
            fFlags.fNoSync = true;
         if (args[i] == "--loadchronojson")
         {
            fFlags.fLoadJsonChannelNames = true;
            i++;
            fFlags.fLoadJsonFile=args[i];
         }
      }
   }

   void Finish()
   {
      printf("OfficialTimeFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("OfficialTimeFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new OfficialTime(runinfo, &fFlags);
   }
};

static TARegister tar(new OfficialTimeFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

