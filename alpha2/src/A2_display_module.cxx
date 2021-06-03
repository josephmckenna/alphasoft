//
// A2_display_module.cxx
//
// display of TPC data
//

#include <stdio.h>
#include <iostream>

#include "manalyzer.h"
#include "midasio.h"

#include "A2Flow.h"

#include "TAlphaDisplay.h"

#include "TStoreEvent.hh"

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

class A2DisplayModuleFlags
{
public:
   //By default, run in batch mode (no event display)
   bool fBatch = true;
   //By default do not auto save plots
   int fAutoSave = 0;
   //Allow users to give a range of event numbers, default settings allow save of all events
   int fAutoSaveStart = -1;
   int fAutoSaveStop = 2147483647;
   
};

class A2DisplayRun: public TARunObject
{
private:
   TAlphaDisplay *a2ed;
   A2DisplayModuleFlags* fFlags;

public:

   A2DisplayRun(TARunInfo* runinfo, A2DisplayModuleFlags* flags)
      : TARunObject(runinfo), a2ed(NULL), fFlags(flags)
   {
#ifdef MANALYZER_PROFILER
      fModuleName="Display Module";
#endif
      printf("A2DisplayRun::ctor!\n");
   }

   ~A2DisplayRun()
   {
      DELETE(a2ed);
      printf("A2DisplayRun::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("A2DisplayRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      
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
      printf("A2DisplayRun::EndRun, run %d\n", runinfo->fRunNo);
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
      DELETE(a2ed);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("A2DisplayRun::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("A2DisplayRun::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   //   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if( fFlags->fBatch && fFlags->fAutoSave == 0 )
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      //printf("DisplayModule::Analyze, run %d\n",runinfo->fRunNo);
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TAlphaEvent* alphaevent=fe->alphaevent;
      TSiliconEvent* silevent=fe->silevent;

      //printf("A2DisplayRun::Analyze event no %d, FlowEvent no %d, BarEvent no %d\n", age->counter,analysis_flow->fEvent->GetEventNumber(),bar_flow->BarEvent-> GetID());

      if (!a2ed) 
      {
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock); 
         if (!TARootHelper::fgApp)
            TARootHelper::fgApp = new TApplication("A2EventDisplay", NULL, NULL, 0, 0);
         printf("New A2ed!\n");
         a2ed = new TAlphaDisplay( runinfo->fRunNo);
      }

      //analysis_flow->fEvent->Print();
      if (a2ed && alphaevent && silevent) {
         int EventNo = silevent->GetVF48NEvent();
         // Do not save if event number is outside of requested range
         if (fFlags->fAutoSaveStart < EventNo && EventNo <= fFlags->fAutoSaveStop )
         {
            std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock); 
            a2ed->SetEventPointer(alphaevent,silevent);
            a2ed->DrawAllViews(); //? Is this the best draw function to be calling?
            // a2ed->DrawViewX3D();
            // a2ed->DrawViewOGL();  //Prettier draw mode :)
            if(fFlags->fAutoSave)
            {
               a2ed->Save();
            }
            //Let the user interact if not in batch mode
            if (!fFlags->fBatch)
               TARootHelper::fgApp->Run(kTRUE);
            //printf("A2ed::ShowEvent is %d\n",flags);
         }
      }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("A2DisplayRun::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class A2DisplayModuleFactory: public TAFactory
{
public:
   A2DisplayModuleFlags fFlags;
public:
   void Usage()
   {
      printf("A2DisplayModuleFactory::Help!\n");
      printf("\t--a2ed                Turn A2 event display on (interactive mode)\n");
      printf("\t--a2edsave            Automatically save events\n");
      printf("\t--a2edsavestart XXX   Automatically save events after event number XXX\n");
      printf("\t--a2edsavestop XXX    Automatically save events after event number XXX\n");
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("A2DisplayModuleFactory::Init!\n");
      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      // READ cmd line parameters to pass to this module here
      for (unsigned i=0; i<args.size(); i++)
         {
            if( args[i] == "--a2ed" )
               fFlags.fBatch = false;

            if( args[i] == "--a2edsave" )
               fFlags.fAutoSave = true;

            if( args[i] == "--a2edsavestart")
            {
               fFlags.fAutoSave = true;
               fFlags.fAutoSaveStart = atoi(args[++i].c_str());
            }
            if( args[i] == "--a2edsavestop")
            {
               fFlags.fAutoSave = true;
               fFlags.fAutoSaveStop = atoi(args[++i].c_str());
            }
         }
      // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
   }
   void Finish()
   {
      printf("A2DisplayModuleFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      if( fFlags.fBatch )
         printf("A2DisplayModuleFactory::NewRunObject, run %d, file %s -- BATCH MODE\n",
                runinfo->fRunNo, runinfo->fFileName.c_str());
      else
         printf("A2DisplayModuleFactory::NewRunObject, run %d, file %s\n", 
                runinfo->fRunNo, runinfo->fFileName.c_str());

      return new A2DisplayRun(runinfo, &fFlags);
   }
};

static TARegister tar(new A2DisplayModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
