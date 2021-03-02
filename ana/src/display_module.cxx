//
// display_module.cxx
//
// display of TPC data
//

#include <stdio.h>
#include <iostream>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

#include "TStoreEvent.hh"
#include "Aged.h"
#include "X11/Intrinsic.h"

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

using std::cout;
using std::cerr;
using std::endl;

class DisplayFlags
{
public:
   bool fBatch;
   bool fForce;

   DisplayFlags():fBatch(true),fForce(false)
   {}
   ~DisplayFlags() {}
};

class DisplayRun: public TARunObject
{
private:
   Aged *aged;
   DisplayFlags* fFlags;

public:

   DisplayRun(TARunInfo* runinfo, DisplayFlags* f)
      : TARunObject(runinfo), aged(0), fFlags(f)
   {
#ifdef MANALYZER_PROFILER
      ModuleName="Display Module";
#endif
      printf("DisplayRun::ctor!\n");
   }

   ~DisplayRun()
   {
      DELETE(aged);
      printf("DisplayRun::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("DisplayRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      
      time_t run_start_time = 0;
      #ifdef INCLUDE_VirtualOdb_H
      run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      #endif
      #ifdef INCLUDE_MVODB_H
      runinfo->fOdb->RU32("/Runinfo/Start time binary",(uint32_t*) &run_start_time);
      #endif
      printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      
      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      // ********* CREATE your display here
      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("DisplayRun::EndRun, run %d\n", runinfo->fRunNo);
      time_t run_stop_time = 0;
      #ifdef INCLUDE_VirtualOdb_H
      run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      #endif
      #ifdef INCLUDE_MVODB_H
      runinfo->fOdb->RU32("/Runinfo/Stop time binary", (uint32_t*) &run_stop_time);
      #endif
      printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));

      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      // ********* DESTROY your display here
      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      DELETE(aged);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("DisplayRun::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("DisplayRun::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   //   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if( fFlags->fBatch )
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      //printf("DisplayModule::Analyze, run %d\n",runinfo->fRunNo);

      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      AgEvent* age = ef->fEvent;
      
      if( !age->a16 )
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }

      if( !age->feam && !fFlags->fForce )
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      // DISPLAY low-level stuff here
      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow )
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      // DISPLAY high-level stuff here
      AgAnalysisFlow* analysis_flow = flow->Find<AgAnalysisFlow>();
      if( !analysis_flow || !analysis_flow->fEvent )
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      // DISPLAY BSC stuff here
      AgBarEventFlow* bar_flow = flow->Find<AgBarEventFlow>();
      if( !bar_flow || !bar_flow->BarEvent)
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }

      printf("DisplayRun::Analyze event no %d, FlowEvent no %d, BarEvent no %d\n", age->counter,analysis_flow->fEvent->GetEventNumber(),bar_flow->BarEvent-> GetID());

      if (!aged) 
         {
            printf("New Aged!\n");
            aged = new Aged();
         }

      analysis_flow->fEvent->Print();
      if (aged) {
         flags=aged->ShowEvent(age,analysis_flow,SigFlow,bar_flow,flags,runinfo);
         printf("Aged::ShowEvent is %d\n",*flags);
      }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("DisplayRun::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class DisplayModuleFactory: public TAFactory
{
public:
   DisplayFlags fFlags;

public:
   void Usage()
   {
      printf("DisplayModuleFactory::Help!\n");
      printf("\t--aged      Turn AG event display on\n");
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("DisplayModuleFactory::Init!\n");
      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      // READ cmd line parameters to pass to this module here
      for (unsigned i=0; i<args.size(); i++)
         {
            if( args[i] == "--aged" )
               fFlags.fBatch = false;
            if( args[i] == "--forcereco" )
               fFlags.fForce=true;
         }
      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
   }
   void Finish()
   {
      printf("DisplayModuleFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      if( fFlags.fBatch )
         printf("DisplayModuleFactory::NewRunObject, run %d, file %s -- BATCH MODE\n",
                runinfo->fRunNo, runinfo->fFileName.c_str());
      else
         printf("DisplayModuleFactory::NewRunObject, run %d, file %s\n", 
                runinfo->fRunNo, runinfo->fFileName.c_str());

      return new DisplayRun(runinfo,&fFlags);
   }
};


static TARegister tar(new DisplayModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
