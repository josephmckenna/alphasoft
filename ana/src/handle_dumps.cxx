//
// Slow down flow to real time (testing module)
//
// JTK McKENNA
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include "TSpill.h"
#include "AgFlow.h"
#include "RecoFlow.h"

#include "cb_flow.h"
#include "store_cb.h"

#include "TChronoChannelName.h"
#include "DumpHandling.h"
#include <iostream>


class DumpMakerModuleFlags
{
public:
   bool fPrint = false;
};

class DumpMakerModule: public TARunObject
{
private:
   double LastSISTS=0;
   static const int MAXDET=10;
public:
   DumpMakerModuleFlags* fFlags;
   bool fTrace = false;
   std::deque<TAGSpill*> IncompleteDumps;

   TChronoChannel DumpStartChannels[USED_SEQ];
   TChronoChannel DumpStopChannels[USED_SEQ];
   
   TChronoChannel fADChannel = {-1, -1};
   TChronoChannel fPreTriggerChannel = {-1, -1};
   int fADCounter;
   int fPreTriggerCounter;
   
   TChronoChannel detectorCh[MAXDET];
   TString detectorName[MAXDET];
   
   bool have_svd_events = false;
   
   DumpList<TAGSpill,TStoreEvent,TCbFIFOEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS> dumplist[USED_SEQ];
   std::mutex SequencerLock[USED_SEQ];
   
   DumpMakerModule(TARunInfo* runinfo, DumpMakerModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="Handle Dumps";
#endif
      if (fTrace)
         printf("DumpMakerModule::ctor!\n");
   }

   ~DumpMakerModule()
   {
      if (fTrace)
         printf("DumpMakerModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("DumpMakerModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      for (int j=0; j<USED_SEQ; j++) 
      {
         dumplist[j].SequencerID=j;
         dumplist[j].fRunNo=runinfo->fRunNo;
      }
      //Save chronobox channel names
      TChronoChannelName* name[CHRONO_N_BOARDS];
      TString ChannelName;

      for (int i=0; i<USED_SEQ; i++)
      {
         int iSeq=USED_SEQ_NUM[i];
         
         std::cout<<i<<" is " << iSeq <<std::endl;
         DumpStartChannels[iSeq].SetChannel(-1);
         DumpStartChannels[iSeq].SetBoard(-1);
         DumpStopChannels[iSeq].SetChannel(-1);
         DumpStopChannels[iSeq].SetBoard(-1);
         //StartSeqChannel[iSeq].Channel=-1;
         //StartSeqChannel[iSeq].Board=-1;
      }
      for (int board=0; board < CHRONO_N_BOARDS; board++)
      {
         name[board]=new TChronoChannelName(runinfo->fOdb,board);
      }
      for (int board=0; board < CHRONO_N_BOARDS; board++)
      {
         int channel=-1;
         for (int i=0; i<USED_SEQ; i++)
         {
            int iSeq=USED_SEQ_NUM[i];
            channel=name[board]->GetChannel(StartDumpName[i]);
            if (channel>0)
            {
               std::cout<<"Sequencer["<<iSeq<<"]:"<<StartDumpName[iSeq]<<" on channel:"<<channel<< " board:"<<board<<std::endl;
               DumpStartChannels[iSeq].SetChannel(channel);
               DumpStartChannels[iSeq].SetBoard(board);
               std::cout<<"Start Channel:"<<channel<<std::endl;
            }
            
            channel=name[board]->GetChannel(StopDumpName[i]);
            if (channel>0)
            {
               std::cout<<"Sequencer["<<iSeq<<"]:"<<StopDumpName[iSeq]<<" on channel:"<<channel<< " board:"<<board<<std::endl;
               DumpStopChannels[iSeq].SetChannel(channel);
               DumpStopChannels[iSeq].SetBoard(board );
               std::cout<<"Stop Channel:"<<channel<<std::endl;
            }
            /*
            channel=name[board]->GetChannel(StartSeqName[i]);
            if (channel>0)
            {
               std::cout<<"Sequencer["<<iSeq<<"]"<<StartSeqName[iSeq]<<" on channel:"<<channel<< " board:"<<board<<std::endl;
               StartSeqChannel[iSeq].Channel=channel;
               StartSeqChannel[iSeq].Board=board;
            }
            */
         }

         channel=name[board]->GetChannel("AD_TRIG");
         if (channel>0)
         {
            fADChannel.SetChannel(channel);
            fADChannel.SetBoard(board);
            std::cout<<"AD_TRIG:"<<channel<<" board:"<<board<<std::endl;
         }
         /*channel=name[board]->GetChannel("POS_TRIG");
         if (channel>0 )
         {
            fPOSChannel.SetChannel(channel);
            fPOSChannel.SetBoard(board);
         }*/
      }
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("DumpMakerModule::EndRun, run %d\n", runinfo->fRunNo);
      for (int a=0; a<USED_SEQ; a++)
      {
         dumplist[a].finish();
         while (dumplist[a].error_queue.size())
         {
            IncompleteDumps.push_back(dumplist[a].error_queue.front());
            dumplist[a].error_queue.pop_front();
         }
         dumplist[a].fRunNo=-2;
      }
         
      if (IncompleteDumps.size())
         printf("Error: Incomplete dumps!!!");
      for ( auto &spill :  IncompleteDumps )
      {
         std::cout<<"Deleting spill:"<<std::endl;
         spill->Print();
         delete spill;
      }
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("DumpMakerModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   //Catch sequencer flow in the main thread, so that we have expected dumps ASAP
   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      if( me->event_id != 8 ) // sequencer event id
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      DumpFlow* DumpsFlow=flow->Find<DumpFlow>();
      if (!DumpsFlow)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      const uint ndumps=DumpsFlow->DumpMarkers.size();
      if (!ndumps)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      int iSeq=DumpsFlow->SequencerNum;
      std::cout <<"iSeq"<<iSeq <<std::endl;
      {
      //Lock scope
      std::lock_guard<std::mutex> lock(SequencerLock[iSeq]);
      
      dumplist[iSeq].setup(me->time_stamp);
      
      for(auto dump: DumpsFlow->DumpMarkers)
      {
         dumplist[iSeq].AddDump( &dump);
      }
      //Copy states into dumps
      dumplist[iSeq].AddStates(DumpsFlow->states);
      //Inspect dumps and make sure the SIS will get triggered when expected... (study digital out)
      dumplist[iSeq].check(DumpsFlow->driver);
      
      }
      //dumplist[iSeq].Print();
      return flow;
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      TCbFlow* FIFOFlow = flow->Find<TCbFlow>();
      uint32_t midas_time = 0;
      if (FIFOFlow)
         midas_time = FIFOFlow->fMidasTime;

      TCbFIFOEventFlow* ChronoFlow = flow->Find<TCbFIFOEventFlow>();
      if (!FIFOFlow && !ChronoFlow)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      if (ChronoFlow)
      {
         for (const std::pair<std::string,std::vector<TCbFIFOEvent>> hits: ChronoFlow->fCbHits)
         {
            const std::string cbname = hits.first;
            char number = cbname[3];
            int ChronoBoard = number - '0' - 1;
            for (int a=0; a<USED_SEQ; a++)
            {
               std::lock_guard<std::mutex> lock(SequencerLock[a]);
               if (DumpStartChannels[a].GetBoard() == ChronoBoard)
               {
                  for (const TCbFIFOEvent& hit: hits.second)
                  {
                     if (DumpStartChannels[a].GetChannel() == hit.GetChannel())
                     {
                        if (hit.IsLeadingEdge())
                           dumplist[a].AddStartTime(midas_time, hit.GetRunTime());
                     }
                  }
               }
               if (DumpStopChannels[a].GetBoard() == ChronoBoard)
               {
                  for (const TCbFIFOEvent& hit: hits.second)
                  {
                     if (DumpStopChannels[a].GetChannel() == hit.GetChannel())
                     {
                        if (hit.IsLeadingEdge())
                           dumplist[a].AddStopTime(midas_time, hit.GetRunTime());
                     }
                  }
               }
            }
            //grep std::cout<<"SIZE:"<<ce->size()<<std::endl;


            //This is pretty dumb... but I need a container that can handle addition. Joe, FIX THIS!
            
            for (int a=0; a<USED_SEQ; a++)
            {
               std::lock_guard<std::mutex> lock(SequencerLock[a]);
               //if (SISFlow->sis_events[j].size()
               dumplist[a].AddScalerEvents(hits.second);
               //dumplist[a].Print();
            }
         }
      }
      AGSpillFlow* f = new AGSpillFlow(flow);
      //Flush errors
      for (int a=0; a<USED_SEQ; a++)
      {
         std::lock_guard<std::mutex> lock(SequencerLock[a]);
         while (dumplist[a].error_queue.size())
         {
            IncompleteDumps.push_back(dumplist[a].error_queue.front());
            dumplist[a].error_queue.pop_front();
         }
      }

      for (size_t i=0; i<IncompleteDumps.size(); i++)
      {
         //if IncompleteDumps.front()INFO TYPE...
         //IncompleteDumps.front()->Print();
          f->spill_events.push_back(IncompleteDumps.front());
          IncompleteDumps.pop_front();
      }

      //Flush completed dumps as TA2Spill objects and put into flow
      for (int a=0; a<USED_SEQ; a++)
      {
         std::lock_guard<std::mutex> lock(SequencerLock[a]);
         std::vector<TAGSpill*> finished = dumplist[a].flushComplete();
         for (TAGSpill* spill: finished)
         {
            //spill->Print();
            f->spill_events.push_back(spill);
            std::cout<<"SPILL FINISHED"<<std::endl;
         }
      }

      flow=f;
      return flow; 
   }

};

class DumpMakerModuleFactory: public TAFactory
{
public:
   DumpMakerModuleFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("DumpMakerModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("DumpMakerModuleFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("DumpMakerModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new DumpMakerModule(runinfo, &fFlags);
   }
};

static TARegister tar(new DumpMakerModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
