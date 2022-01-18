//
// reco_module.cxx
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

#include "Reco.hh"

class RecoWriteFlags
{
    //This is empty, but in case it's needed later, easier to leave in.
    public:
    RecoWriteFlags() // ctor
    { }

    ~RecoWriteFlags() // dtor
    { }
};

class RecoWrite: public TARunObject
{
public:
   bool fTrace = false;
   //bool fTrace = true;
   RecoWriteFlags* fFlags;

private:
   TStoreEvent *analyzed_event;
   TTree *EventTree;
   TBranch *EventBranch;

public:
   RecoWrite(TARunInfo* runinfo, RecoWriteFlags* f): TARunObject(runinfo),fFlags(f)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="RecoModule";
#endif
   }

   ~RecoWrite()
   {
      printf("RecoWrite::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("RecoWrite::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());



      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      analyzed_event = new TStoreEvent;
      EventTree = new TTree("StoreEventTree", "StoreEventTree");
      EventBranch = EventTree->Branch("StoredEvent", &analyzed_event, 2048000, 0);
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("RecoWrite::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("RecoWrite::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("RecoWrite::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if( fTrace )
         printf("RecoWrite::AnalyzeFlowEvent, run %d\n", runinfo->fRunNo);

      AgAnalysisFlow *anaflow = flow->Find<AgAnalysisFlow>();

      if (!anaflow || !anaflow->fEvent)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
#ifdef HAVE_MANALYZER_PROFILER
      TAClock start_time = TAClockNow();
#endif

      // prepare event to store in TTree
    TStoreEvent* analyzed_event = anaflow->fEvent;
    std::cout << "The TStoreEvent I am WRITING to the TREE is the following:" << std::endl;
    analyzed_event->Print();

    if(analyzed_event->GetBarEvent())
    {
        std::cout<< std::endl<< std::endl<< std::endl << "Do we have a populated TBarEvent?" << analyzed_event->GetBarEvent() << std::endl<< std::endl<< std::endl;
    }
    else
    {
        std::cout<< std::endl<< std::endl<< std::endl << "No TBarEvent" << analyzed_event->GetBarEvent() << std::endl<< std::endl<< std::endl;
    }

    std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
    {EventTree->ResetBranchAddresses();
    analyzed_event->ClearUsedHelices();
    EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
    EventTree->Fill();}

      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("RecoWrite::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class RecoWriteModuleFactory: public TAFactory
{
public:
   RecoWriteFlags fFlags;
public:
   void Help()
   {
      printf("RecoWriteModuleFactory::Help\n");
      printf("\t--usetimerange 123.4 567.8\t\tLimit reconstruction to a time range\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
        printf("RecoWriteModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) 
      {
         if( args[i]=="-h" || args[i]=="--help" )
            Help();
      }
   }

   void Finish()
   {
      printf("RecoWriteModuleFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("RecoWriteModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new RecoWrite(runinfo,&fFlags);
   }
};


static TARegister tar(new RecoWriteModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
