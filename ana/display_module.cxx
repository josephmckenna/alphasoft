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

#include "TStoreEvent.hh"
#include "../aged/Aged.h"
#include "X11/Intrinsic.h"
#include "AnalysisTimer.h"
extern int gVerb;

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

using std::cout;
using std::cerr;
using std::endl;

class DisplayRun: public TARunObject
{
private:
   Aged *aged;
   bool fBatch;

public:

   DisplayRun(TARunInfo* runinfo, bool mode)
      : TARunObject(runinfo), aged(NULL), fBatch(mode)
   {
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
      time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      
      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      // ********* CREATE your display here
      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("DisplayRun::EndRun, run %d\n", runinfo->fRunNo);
      time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
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
      if( fBatch ) return flow;

      printf("DisplayModule::Analyze, run %d\n",runinfo->fRunNo);

      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      AgEvent* age = ef->fEvent;
      
      if( !age->feam || !age->a16 )
         return flow;
      // if( !age->feam->complete || !age->a16->complete || age->feam->error || age->a16->error )
      //    return flow;

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow )
         return flow;
      
      AgAnalysisFlow* analysis_flow = flow->Find<AgAnalysisFlow>();
      if( !analysis_flow || !analysis_flow->fEvent )
        return flow;

      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      // DISPLAY low-level stuff here
      // fetching them from SigFlow
      // this object is defined in AgFlow.h
      // and more members will and can be added
      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      // DISPLAY high-level stuff here
      // fetching them from TStoreEvent* anEvent
      // this object is defined in $ANALYSIS_TPC/include/TStoreEvent.h
      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                                                                  
      // I guess there could be two tabs one for low-level and one for high-level
      // a "next" button to go the next event, i.e., terminating this fuction
      // and waiting for the reco_module to analyze the following event.

      // I think that I would like also a "previous" button, but it's not clear
      // to me how to implement a buffer.
      
      printf("DisplayRun::Analyze event no %d\n", age->counter);

      if (!aged) {
      	 aged = new Aged();
      }
      // analysis_flow->fEvent->Print();
      if (aged) aged->ShowEvent(analysis_flow, SigFlow, runinfo);
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"display_module");
      #endif
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
   bool fBatch;
public:
   void Init(const std::vector<std::string> &args)
   {
      printf("DisplayModuleFactory::Init!\n");
      fBatch = true;

      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      // READ cmd line parameters to pass to this module here
      for (unsigned i=0; i<args.size(); i++)
         {
            if( args[i] == "--aged" )
               fBatch = false;
         }
      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
   }
   void Finish()
   {
      printf("DisplayModuleFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      if( fBatch )
         printf("DisplayModuleFactory::NewRunObject, run %d, file %s -- BATCH MODE\n",
                runinfo->fRunNo, runinfo->fFileName.c_str());
      else
         printf("DisplayModuleFactory::NewRunObject, run %d, file %s\n", 
                runinfo->fRunNo, runinfo->fFileName.c_str());

      return new DisplayRun(runinfo, fBatch);
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
