//
// Slow down flow to real time (testing module)
//
// JTK McKENNA
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include "TSpill.h"
#include "A2Flow.h"
#include "TSISChannels.h"
#include "AnalysisTimer.h"
#include <iostream>
class DumpMakerModuleFlags
{
public:
   bool fPrint = false;
};
TString SeqNames[NUMSEQ]={"cat","rct","atm","pos","rct_botg","atm_botg","atm_topg","rct_topg","bml"};

TString StartNames[NUMSEQ]={"SIS_PBAR_DUMP_START","SIS_RECATCH_DUMP_START","SIS_ATOM_DUMP_START","SIS_POS_DUMP_START","NA","NA","NA","NA","NA"};
TString StopNames[NUMSEQ] ={"SIS_PBAR_DUMP_STOP", "SIS_RECATCH_DUMP_STOP", "SIS_ATOM_DUMP_STOP", "SIS_POS_DUMP_STOP","NA","NA","NA","NA","NA"};


bool compareRunTime(SIS_Counts* a, SIS_Counts* b) { return (a->t < b->t); }
class DumpMakerModule: public TARunObject
{
private:
   double LastSISTS=0;
   static const int MAXDET=10;
public:
   DumpMakerModuleFlags* fFlags;
   bool fTrace = false;
   std::deque<TA2Spill*> IncompleteDumps;

   std::deque<SIS_Counts*> SIS_Events[64];
   std::deque<SVD_Counts*> SVD_Events;

   int DumpStartChannels[USED_SEQ] ={-1};
   int DumpStopChannels[USED_SEQ]  ={-1};
   int detectorCh[MAXDET];
   TString detectorName[MAXDET];
   
   bool have_svd_events = false;
   
   DumpList<TA2Spill> dumplist[USED_SEQ];
   
   DumpMakerModule(TARunInfo* runinfo, DumpMakerModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("DumpMakerModule::ctor!\n");
      TSISChannels* SISChannels=new TSISChannels( runinfo->fRunNo );
      for (int j=0; j<USED_SEQ; j++) 
      {
         DumpStartChannels[j] =SISChannels->GetChannel(StartNames[j],runinfo->fRunNo);
         DumpStopChannels[j]  =SISChannels->GetChannel(StopNames[j], runinfo->fRunNo);
      }
      delete SISChannels;
   }

   ~DumpMakerModule()
   {
      if (fTrace)
         printf("DumpMakerModule::dtor!\n");
      for (int j=0; j<64; j++)
      {
        int n=SIS_Events[j].size();
        for (int i=0; i<n; i++)
           delete SIS_Events[j].at(i);
        SIS_Events[j].clear();
      }
      int n=SVD_Events.size();
      for (int i=0; i<n; i++)
         delete SVD_Events[i];
      SVD_Events.clear();
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("DumpMakerModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
    }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("DumpMakerModule::EndRun, run %d\n", runinfo->fRunNo);
         
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
 

   void PrintHeldEvents()
   {
      std::cout<<"Held SIS Events:"<<std::endl;
      for (int i=0; i<64; i++)
         std::cout<<i<<"\t"<<SIS_Events[i].size()<<std::endl;
      std::cout<<"Held SVD Events:"<< SVD_Events.size()<<std::endl;
   }
   void PrintActiveSpills()
   {

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
      //Prepare for next sequence data.. check the last sequence finished
      dumplist[iSeq].setup(&IncompleteDumps);
      TA2Spill* error=NULL;
      for(auto dump: DumpFlow->DumpMarkers)
      {
         error=dumplist[iSeq].AddDump( &dump);
         if (error)
            IncompleteDumps.push_back(error);
      }
      //Copy states into dumps
      dumplist[iSeq].AddStates(&DumpFlow->states);
      //Inspect dumps and make sure the SIS will get triggered when expected... (study digital out)
      dumplist[iSeq].check(DumpFlow->driver,&IncompleteDumps);
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
              TA2Spill* errors=NULL;
              for (int a=0; a<USED_SEQ; a++)
              {
                 if (DumpStartChannels[a]>0)
                    //if (e->GetCountsInChannel(DumpStartChannels[a]))
                    for (int nstarts=0; nstarts<e->GetCountsInChannel(DumpStartChannels[a]); nstarts++)
                    {
                       TA2Spill* error=dumplist[a].AddStartTime(e->GetRunTime());
                       if (error)
                       {
                          e->Print();
                          IncompleteDumps.push_back(error);
                       }
                    }
                 if (DumpStopChannels[a]>0)
                    for (int nstops=0; nstops<e->GetCountsInChannel(DumpStopChannels[a]); nstops++)
                    {
                       TA2Spill* error=dumplist[a].AddStopTime(e->GetRunTime());
                       if (error)
                       {
                          e->Print();
                          IncompleteDumps.push_back(error);
                       }
                    }
               }
            }
         }
         //Add SIS counts to dumps
         for (int a=0; a<USED_SEQ; a++)
            for (int j=0; j<NUM_SIS_MODULES; j++)
               dumplist[a].AddSISEvents(SISFlow->sis_events);
      }
      A2SpillFlow* f=new A2SpillFlow(flow);
      for (int a=0; a<USED_SEQ; a++)
      {
         std::vector<TA2Spill*> finished=dumplist[a].flushComplete();
         for (int i=0; i<finished.size(); i++)
         {
			 f->spill_events.push_back(finished.at(i));
		 }
      }
      flow=f;
      
      SVDQODFlow* QODFlow=flow->Find<SVDQODFlow>();
      if (QODFlow)
      {
         // Dont finish spill events until we fill it with SVD data... 
         // ...we have SVD data yay!
         have_svd_events=true;
         for (uint i=0; i<QODFlow->SVDQODEvents.size(); i++)
         {
            TSVD_QOD* q=QODFlow->SVDQODEvents.at(i);
            SVD_Counts* SV=new SVD_Counts();
            SV->VF48EventNo=q->VF48NEvent;
            SV->t=q->t;
            SV->has_vertex=q->NVertices;
            SV->passed_cuts=q->NPassedCuts;
            SV->online_mva=q->MVA;
            SVD_Events.push_back(SV);
         }
      }


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
