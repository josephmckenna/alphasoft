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

#include "TStoreEvent.hh"

#include "AnaSettings.hh"
#include "json.hpp"

class TStoreEventWriterFlags
{
public:
public:
   TStoreEventWriterFlags() // ctor
   { }

   ~TStoreEventWriterFlags() // dtor
   { }
};

class TStoreEventWriter: public TARunObject
{
public:
   bool do_plot = false;
   const bool fTrace = false;
   // bool fTrace = true;
   bool fVerb=false;

   TStoreEventWriterFlags* fFlags;

private:
   bool diagnostics;

public:
   TStoreEvent *analyzed_event;
   TTree *EventTree;

   TStoreEventWriter(TARunInfo* runinfo, TStoreEventWriterFlags* f): TARunObject(runinfo),
                                                 fFlags(f)

   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TStoreEvent_Writer";
#endif
   }

   ~TStoreEventWriter()
   {
      printf("TStoreEventWriter::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("TStoreEventWriter::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      analyzed_event = new TStoreEvent;
      EventTree = new TTree("StoreEventTree", "StoreEventTree");
      EventTree->Branch("StoredEvent", &analyzed_event, 32000, 0);
      delete analyzed_event;
      analyzed_event=NULL;
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("TStoreEventWriter::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("TStoreEventWriter::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("TStoreEventWriter::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if( fTrace )
         printf("TStoreEventWriter::AnalyzeFlowEvent, run %d\n", runinfo->fRunNo);

      AgAnalysisFlow *af = flow->Find<AgAnalysisFlow>();
      if (!af )
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }

      // prepare event to store in TTree
      analyzed_event = af->fEvent;
      {
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
         EventTree->Fill();
      }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("TStoreEventWriter::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class TStoreEventWriterFactory: public TAFactory
{
public:
   TStoreEventWriterFlags fFlags;
public:
   void Help()
   {
      printf("TStoreEventWriterFactory::Help\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      TString json="default";
      printf("TStoreEventWriterFactory::Init!\n");
      for (unsigned i=0; i<args.size(); i++) {
         if( args[i]=="-h" || args[i]=="--help" )
            Help();
      }
   }

   void Finish()
   {
      printf("TStoreEventWriterFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("TStoreEventWriterFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new TStoreEventWriter(runinfo,&fFlags);
   }
};


static TARegister tar(new TStoreEventWriterFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
