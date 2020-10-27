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
#include "A2Flow.h"
#include "TSISChannels.h"
#include "AnalysisTimer.h"
#include "DumpHandling.h"
#include <iostream>
class DumpMakerModuleFlags
{
public:
   bool fPrint = false;
};
TString SeqNames[NUMSEQ]={"cat","rct","atm","pos","rct_botg","atm_botg","atm_topg","rct_topg","bml"};

TString StartNames[NUMSEQ]={"SIS_PBAR_DUMP_START","SIS_RECATCH_DUMP_START","SIS_ATOM_DUMP_START","SIS_POS_DUMP_START","NA","NA","NA","NA","NA"};
TString StopNames[NUMSEQ] ={"SIS_PBAR_DUMP_STOP", "SIS_RECATCH_DUMP_STOP", "SIS_ATOM_DUMP_STOP", "SIS_POS_DUMP_STOP","NA","NA","NA","NA","NA"};


class DumpMakerModule: public TARunObject
{
private:
   double LastSISTS=0;
   static const int MAXDET=10;
public:
   DumpMakerModuleFlags* fFlags;
   bool fTrace = false;
   std::deque<TA2Spill*> IncompleteDumps;

   int DumpStartChannels[USED_SEQ] ={-1};
   int DumpStopChannels[USED_SEQ]  ={-1};
   int detectorCh[MAXDET];
   TString detectorName[MAXDET];
   
   bool have_svd_events = false;
   
   DumpList<TA2Spill,TSVD_QOD,TSISEvent,NUM_SIS_MODULES> dumplist[USED_SEQ];
   std::mutex SequencerLock[USED_SEQ];
   
   DumpMakerModule(TARunInfo* runinfo, DumpMakerModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      ModuleName="Handle Dumps";
      if (fTrace)
         printf("DumpMakerModule::ctor!\n");
      TSISChannels* SISChannels=new TSISChannels( runinfo->fRunNo );
      for (int j=0; j<USED_SEQ; j++) 
      {
         dumplist[j].SequencerID=j;
         DumpStartChannels[j] =SISChannels->GetChannel(StartNames[j],runinfo->fRunNo);
         DumpStopChannels[j]  =SISChannels->GetChannel(StopNames[j], runinfo->fRunNo);
      }
      delete SISChannels;
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
         dumplist[j].fRunNo=runinfo->fRunNo;
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
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      AgDumpFlow* DumpFlow=flow->Find<AgDumpFlow>();
      if (!DumpFlow)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      uint ndumps=DumpFlow->DumpMarkers.size();
      if (!ndumps)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
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
      return flow;
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SISEventFlow* SISFlow = flow->Find<SISEventFlow>();
      if (SISFlow)
      {
         //Add timestamps to dumps
         for (int j=0; j<NUM_SIS_MODULES; j++)
         {
            std::vector<TSISEvent*>* ce=&SISFlow->sis_events[j];
            for (uint i=0; i<ce->size(); i++)
            {
              TSISEvent* e=ce->at(i);
              for (int a=0; a<USED_SEQ; a++)
              {
                 std::lock_guard<std::mutex> lock(SequencerLock[a]);
                 if (DumpStartChannels[a]>0)
                    //if (e->GetCountsInChannel(DumpStartChannels[a]))
                    for (int nstarts=0; nstarts<e->GetCountsInChannel(DumpStartChannels[a]); nstarts++)
                    {
                       dumplist[a].AddStartTime(e->GetMidasUnixTime(), e->GetRunTime());
                    }
                 if (DumpStopChannels[a]>0)
                    for (int nstops=0; nstops<e->GetCountsInChannel(DumpStopChannels[a]); nstops++)
                    {
                       dumplist[a].AddStopTime(e->GetMidasUnixTime(),e->GetRunTime());
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
                  dumplist[a].AddScalerEvents(&SISFlow->sis_events[j]);
            }
         }
      }
      SVDQODFlow* SVDFlow = flow->Find<SVDQODFlow>();
      if (SVDFlow)
      {
         for (int a=0; a<USED_SEQ; a++)
         {
            std::lock_guard<std::mutex> lock(SequencerLock[a]);
            dumplist[a].AddSVDEvents(&SVDFlow->SVDQODEvents);
         }
      }
      
      A2SpillFlow* f=new A2SpillFlow(flow);
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
         std::vector<TA2Spill*> finished=dumplist[a].flushComplete();
         for (size_t i=0; i<finished.size(); i++)
         {
            f->spill_events.push_back(finished.at(i));
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
