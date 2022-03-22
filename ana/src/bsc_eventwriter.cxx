//
// reco_spacepoint_builder.cxx
//
// reconstruction of TPC data
//

#include <stdio.h>
#include <iostream>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

#include <TTree.h>

#include "TBarEvent.hh"

#include "AnaSettings.hh"
#include "json.hpp"

class TBarEventWriterFlags
{
public:
public:
   TBarEventWriterFlags() // ctor
   {
   }

   ~TBarEventWriterFlags() // dtor
   { }
};

class TBarEventWriter: public TARunObject
{
public:
   const bool fTrace = false;
   // bool fTrace = true;
   bool fVerb=false;

   TBarEventWriterFlags* fFlags;

private:
   bool diagnostics;

public:
   TBarEvent *bar_event;
   TTree *EventTree;

   TBarEventWriter(TARunInfo* runinfo, TBarEventWriterFlags* f): TARunObject(runinfo),
                                                 fFlags(f)

   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TStoreEvent_Writer";
#endif
   }

   ~TBarEventWriter()
   {
      printf("TBarEventWriter::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("TBarEventWriter::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      bar_event = new TBarEvent();
      EventTree = new TTree("TBarEventTree", "TBarEventTree");
      EventTree->Branch("TBarEvent", &bar_event, 32000, 0);
      delete bar_event;
      bar_event=NULL;
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("TBarEventWriter::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("TBarEventWriter::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("TBarEventWriter::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if( fTrace )
         printf("TBarEventWriter::AnalyzeFlowEvent, run %d\n", runinfo->fRunNo);

      AgBarEventFlow *af = flow->Find<AgBarEventFlow>();
      if (!af )
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }

      // prepare event to store in TTree
      if (af->BarEvent)
      {
         bar_event = af->BarEvent;
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         EventTree->Fill();
      }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("TBarEventWriter::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class TBarEventWriterFactory: public TAFactory
{
public:
   TBarEventWriterFlags fFlags;
public:
   void Help()
   {
      printf("TBarEventWriterFactory::Help\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      TString json="default";
      printf("TBarEventWriterFactory::Init!\n");
      for (unsigned i=0; i<args.size(); i++) {
         if( args[i]=="-h" || args[i]=="--help" )
            Help();
      }
   }

   void Finish()
   {
      printf("TBarEventWriterFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("TBarEventWriterFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new TBarEventWriter(runinfo,&fFlags);
   }
};


static TARegister tar(new TBarEventWriterFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
