//
// Slow down flow to real time (testing module)
//
// JTK McKENNA
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include "TSISEvent.h"
#include "TSpill.h"
#include "AgFlow.h"
#include "chrono_module.h"
#include "TChrono_Event.h"
#include "TChronoChannelName.h"
#include "AnalysisTimer.h"
#include "DumpHandling.h"
#include <iostream>
class DumpMakerModuleFlags
{
public:
   bool fPrint = false;
};
TString SeqNames[NUMSEQ]={"cat","rct","atm","pos","rct_botg","atm_botg","atm_topg","rct_topg","bml"};


class DumpMakerModule: public TARunObject
{
private:
   double LastSISTS=0;
   static const int MAXDET=10;
public:
   DumpMakerModuleFlags* fFlags;
   bool fTrace = false;
   std::deque<TAGSpill*> IncompleteDumps;

   ChronoChannel DumpStartChannels[USED_SEQ];
   ChronoChannel DumpStopChannels[USED_SEQ];
   ChronoChannel detectorCh[MAXDET];
   TString detectorName[MAXDET];
   
   bool have_svd_events = false;
   
   DumpList<TAGSpill,TStoreEvent,ChronoEvent,CHRONO_N_BOARDS*CHRONO_N_CHANNELS> dumplist[USED_SEQ];
   std::mutex SequencerLock[USED_SEQ];
   
   DumpMakerModule(TARunInfo* runinfo, DumpMakerModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
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
      //Save chronobox channel names
      TChronoChannelName* name[CHRONO_N_BOARDS];
      TString ChannelName;

      for (int i=0; i<USED_SEQ; i++)
      {
         int iSeq=USED_SEQ_NUM[i];
         std::cout<<i<<" is " << iSeq <<std::endl;
         DumpStartChannels[iSeq].Channel=-1;
         DumpStartChannels[iSeq].Board=-1;
         DumpStopChannels[iSeq].Channel=-1;
         DumpStopChannels[iSeq].Board=-1;
         //StartSeqChannel[iSeq].Channel=-1;
         //StartSeqChannel[iSeq].Board=-1;
      }
      for (int board=0; board<CHRONO_N_BOARDS; board++)
      {
         name[board]=new TChronoChannelName();
         name[board]->SetBoardIndex(board+1);
         for (int det=0; det<MAXDET; det++)
         {
            detectorCh[det].Channel=-1;
            detectorCh[det].Board=-1;
         }
         for (int chan=0; chan<CHRONO_N_CHANNELS; chan++)
         {
            TString OdbPath="/Equipment/cbms0";
            OdbPath+=board+1;
            OdbPath+="/Settings/ChannelNames";
            //std::cout<<runinfo->fOdb->odbReadString(OdbPath.Data(),chan)<<std::endl;
            #ifdef INCLUDE_VirtualOdb_H
            if (runinfo->fOdb->odbReadString(OdbPath.Data(),chan))
               name->SetChannelName(runinfo->fOdb->odbReadString(OdbPath.Data(),chan),chan);
            #endif
            #ifdef INCLUDE_MVODB_H
            std::string tmp;
            runinfo->fOdb->RSAI(OdbPath.Data(),chan,&tmp);
            name[board]->SetChannelName(tmp.c_str(),chan);
            #endif
         }
     }
      for (int board=0; board<CHRONO_N_BOARDS; board++)
      {
         int channel=-1;
         for (int i=0; i<USED_SEQ; i++)
         {
            int iSeq=USED_SEQ_NUM[i];
            channel=name[board]->GetChannel(StartDumpName[i]);
            if (channel>0)
            {
               std::cout<<"Sequencer["<<iSeq<<"]:"<<StartDumpName[iSeq]<<" on channel:"<<channel<< " board:"<<board<<std::endl;
               DumpStartChannels[iSeq].Channel=channel;
               DumpStartChannels[iSeq].Board=board;
               std::cout<<"Start Channel:"<<channel<<std::endl;
            }
            
            channel=name[board]->GetChannel(StopDumpName[i]);
            if (channel>0)
            {
               std::cout<<"Sequencer["<<iSeq<<"]:"<<StopDumpName[iSeq]<<" on channel:"<<channel<< " board:"<<board<<std::endl;
               DumpStopChannels[iSeq].Channel=channel;
               DumpStopChannels[iSeq].Board=board;
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
/*
         channel=name[board]->GetChannel("AD_TRIG");
         if (channel>0)
         {
            gADSpillChannel.Channel=channel;
            gADSpillChannel.Board=board;
            std::cout<<"AD_TRIG:"<<channel<<" board:"<<board<<std::endl;
         }
         channel=name[board]->GetChannel("POS_TRIG");
         if (channel>0 )
         {
            gPOSSpillChannel.Channel=channel;
            gPOSSpillChannel.Board=board;
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
      }
         
      if (IncompleteDumps.size())
         printf("Error: Incomplete dumps!!!");
      for ( auto &spill :  IncompleteDumps )
         spill->Print();
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
      #ifdef _TIME_ANALYSIS_
      START_TIMER
      #endif
      if( me->event_id != 8 ) // sequencer event id
         return flow;
      AgDumpFlow* DumpFlow=flow->Find<AgDumpFlow>();
      if (!DumpFlow)
         return flow;
      uint ndumps=DumpFlow->DumpMarkers.size();
      if (!ndumps)
         return flow;
      int iSeq=DumpFlow->SequencerNum;
      {
      //Lock scope
      std::lock_guard<std::mutex> lock(SequencerLock[iSeq]);
      
      dumplist[iSeq].setup();
      
      for(auto dump: DumpFlow->DumpMarkers)
      {
         dumplist[iSeq].AddDump( &dump);
      }
      //Copy states into dumps
      dumplist[iSeq].AddStates(&DumpFlow->states);
      //Inspect dumps and make sure the SIS will get triggered when expected... (study digital out)
      dumplist[iSeq].check(DumpFlow->driver);
      
      }
      //dumplist[iSeq].Print();
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"handle_dumps(main thread)",timer_start);
      #endif
      return flow;
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      #ifdef _TIME_ANALYSIS_
      START_TIMER
      #endif
      AgChronoFlow* ChronoFlow = flow->Find<AgChronoFlow>();
      if (ChronoFlow)
      {
         //Add timestamps to dumps
         std::vector<ChronoEvent*>* ce=ChronoFlow->events;
         for (uint i=0; i<ce->size(); i++)
         {
            ChronoEvent* e=ce->at(i);
            for (int a=0; a<USED_SEQ; a++)
            {
               std::lock_guard<std::mutex> lock(SequencerLock[a]);
               if (DumpStartChannels[a].Channel==e->Channel)
               if (DumpStartChannels[a].Board ==e->ChronoBoard)
               {
                  //if (e->GetCountsInChannel(DumpStartChannels[a]))
                  for (uint32_t nstarts=0; nstarts<e->Counts; nstarts++)
                  {
                     dumplist[a].AddStartTime(e->MidasTime, e->RunTime);
                  }
               }
               if (DumpStopChannels[a].Channel==e->Channel)
               if (DumpStopChannels[a].Board  ==e->ChronoBoard)
               {
                  for (uint32_t nstops=0; nstops<e->Counts; nstops++)
                  {
                     dumplist[a].AddStopTime(e->MidasTime, e->RunTime);
                  }
               }
            }
         }
         //Add SIS counts to dumps
         for (int a=0; a<USED_SEQ; a++)
         {
            std::lock_guard<std::mutex> lock(SequencerLock[a]);
            for (int j=0; j<NUM_SIS_MODULES; j++)
            {
               //if (SISFlow->sis_events[j].size())
                  dumplist[a].AddScalerEvents(&ChronoFlow->events[j]);
            }
         }
      }
      /*
      SVDQODFlow* SVDFlow = flow->Find<SVDQODFlow>();
      if (SVDFlow)
      {
         for (int a=0; a<USED_SEQ; a++)
         {
            std::lock_guard<std::mutex> lock(SequencerLock[a]);
            dumplist[a].AddSVDEvents(&SVDFlow->SVDQODEvents);
         }
      }
      */
      AGSpillFlow* f=new AGSpillFlow(flow);
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
          f->spill_events.push_back(IncompleteDumps.front());
          IncompleteDumps.pop_front();
      }

      //Flush completed dumps as TA2Spill objects and put into flow
      for (int a=0; a<USED_SEQ; a++)
      {
         std::lock_guard<std::mutex> lock(SequencerLock[a]);
         std::vector<TAGSpill*> finished=dumplist[a].flushComplete();
         for (size_t i=0; i<finished.size(); i++)
         {
            f->spill_events.push_back(finished.at(i));
         }
      }

      flow=f;

      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"handle_dumps",timer_start);
      #endif

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
