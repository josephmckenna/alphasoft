//
// Slow down flow to real time (testing module)
//
// JTK McKENNA
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include "A2Flow.h"

#include "AnalysisTimer.h"
#include <iostream>
class CatchEfficiencyModuleFlags
{
public:
   bool fPrint = false;

};
class CatchEfficiencyModule: public TARunObject
{
private:
   clock_t start_time;
   uint32_t FirstEvent=0;
   TA2Spill* HotDump=NULL;
   TA2Spill* ColdDump=NULL;

public:
   CatchEfficiencyModuleFlags* fFlags;
   bool fTrace = false;
   
   
   CatchEfficiencyModule(TARunInfo* runinfo, CatchEfficiencyModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("CatchEfficiencyModule::ctor!\n");
   }

   ~CatchEfficiencyModule()
   {
      if (fTrace)
         printf("CatchEfficiencyModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("CatchEfficiencyModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      start_time=clock();
    }

   void EndRun(TARunInfo* runinfo)
   {
      if (HotDump)
         delete HotDump;
      if (ColdDump)
         delete ColdDump;
      if (fTrace)
         printf("CatchEfficiencyModule::EndRun, run %d\n", runinfo->fRunNo);
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("CatchEfficiencyModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
  {
      #ifdef _TIME_ANALYSIS_
      START_TIMER
      #endif
      A2SpillFlow* SpillFlow= flow->Find<A2SpillFlow>();
      if (SpillFlow)
      {
         for (size_t i=0; i<SpillFlow->spill_events.size(); i++)
         {
            TA2Spill* s=SpillFlow->spill_events.at(i);
            //s->Print();
            if (!s->SeqData) continue;
            int thisSeq=s->SeqData->fSequenceNum;
            if (thisSeq==0) //Catching trap
            //if (strcmp(s->SeqName.c_str(),"cat")==0)
            if (strcmp(s->Name.c_str(),"\"Hot Dump\"")==0)
            {
               if (HotDump)
                  delete HotDump;
               //Hot dump happens before cold dump... so if something exists in memory... its out of date
               if (ColdDump)
               {
                  delete ColdDump;
                  ColdDump=NULL;
               }
               HotDump=new TA2Spill(s);
            }
            if (strcmp(s->Name.c_str(),"\"Cold Dump\"")==0)
            {
               if (ColdDump)
                  delete ColdDump;
               ColdDump=new TA2Spill(s);
               std::cout<<"catch_efficiency_module::CalculateCatchEfficiency"<<std::endl;
               TA2Spill* eff=*ColdDump/(*ColdDump+HotDump);
               //eff->Print();
               SpillFlow->spill_events.push_back(eff);
            }
            
         }
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"catch_efficiency_module",timer_start);
      #endif
      }

      return flow; 
  }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("CatchEfficiencyModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class CatchEfficiencyModuleFactory: public TAFactory
{
public:
   CatchEfficiencyModuleFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("CatchEfficiencyModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("CatchEfficiencyModuleFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("CatchEfficiencyModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new CatchEfficiencyModule(runinfo, &fFlags);
   }
};

static TARegister tar(new CatchEfficiencyModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
