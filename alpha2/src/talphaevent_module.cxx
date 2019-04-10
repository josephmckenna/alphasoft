// HandleVF48.cxx

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

#include "TFile.h"
#include "TH2.h"
#include "TF1.h"
#include "TLatex.h"
#include "TText.h"
#include "TBox.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TTree.h"


#include "manalyzer.h"
#include "midasio.h"

#include "TSettings.h"

#include "SiMod.h"
#include "UnpackVF48.h"
#include "A2Flow.h"

#include "TAlphaEvent.h"

#include "TVF48SiMap.h"
#include "TSystem.h"

#define MAX_CHANNELS VF48_MAX_CHANNELS // defined in UnpackVF48.h
#define NUM_SI_MODULES nSil // defined in SiMod.h
#define NUM_VF48_MODULES nVF48 // defined in SiMod.h
#define NUM_SI_ALPHA1 nSiAlpha1 // defined in SiMod.h
#define NUM_SI_ALPHA2 nSiAlpha2 // defined in SiMod.h

#define VF48_COINCTIME 0.000010

class AlphaEventFlags
{
public:
   bool fPrint = false;
};

class AlphaEventModule: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   bool fTrace = false;

   TAlphaEvent* gAlphaEvent = NULL;

   AlphaEventModule(TARunInfo* runinfo, AlphaEventFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("AlphaEventModule::ctor!\n");
      gAlphaEvent = new TAlphaEvent(); 
   }

   ~AlphaEventModule()
   {
      if (fTrace)
         printf("AlphaEventModule::dtor!\n");
      delete gAlphaEvent;
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("HitModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void PreEndRun(TARunInfo* runinfo, std::deque<TAFlowEvent*>* flow_queue)
   {
      if (fTrace)
         printf("HitModule::PreEndRun, run %d\n", runinfo->fRunNo);
      //time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      //printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("HitModule::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("HitModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }
   

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);

      if (event->event_id != 11)
         return flow;

      SilEventsFlow* fe=flow->Find<SilEventsFlow>();
      if (!fe)
         return flow;
      int n_events=fe->silevents.size();
      if (n_events>0)
      {
         flow=new SilEventsFlow(flow);
      }
      //std::cout<<"N Events: " <<n_events<<std::endl;
      for (int i=0; i<n_events; i++)
      {
         TSiliconEvent* s=((SilEventsFlow*)flow)->silevents.at(i);
      }
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"hybrid_hits_module");
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("HitModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class AlphaEventModuleFactory: public TAFactory
{
public:
   AlphaEventFlags fFlags;

public:
   void Help()
   {
      printf("AlphaEventModuleFactory::Help!\n");
      printf("\t--nounpack   Turn unpacking of TPC data (turn off reconstruction completely)\n");
   }
   void Usage()
   {
     Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++)
      {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      printf("AlphaEventModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule(runinfo, &fFlags);
   }
};

static TARegister tar(new AlphaEventModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
