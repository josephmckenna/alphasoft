// A module to write a minaturised version of TAGDetectorEvent

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"

#include "TVector3.h"
#include <iostream>
#include "RecoFlow.h"
#include "TTree.h"
#include "TAGDetectorEvent.hh"

class TAGDetectorEventWriterFlags
{
public:
   //Buffer events before writing into root file (reduces number of locks needed)
   int fWriteInterval;
   TAGDetectorEventWriterFlags() // ctor
   {
      fWriteInterval = 100;
   }

   ~TAGDetectorEventWriterFlags() // dtor
   { }
};

class TAGDetectorEventWriter: public TARunObject
{
public:
   const bool fTrace = false;
   TAGDetectorEventWriterFlags* fFlags;
   const int WriteInterval = 10000;
   std::deque<TAGDetectorEvent> fEvents;

   int fTPCEvents;
   int fBarEvents;
   int fFills;
   

private:

   void Flush_Buffer()
   {
      if (fTrace)
         std::cout <<"Flush\n";
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      while (fEvents.size())
      {
         const TAGDetectorEvent& e = fEvents.front();
         if (!e.fHasAnalysisFlow && !e.fHasBarFlow)
         {
            if (fTrace)
               std::cout <<__FILE__<<" buffer has missing flow!"<<std::endl;
            break;
         }
         write_event = (TAGDetectorEvent*) &e;
         EventTree->Fill();
         fFills++;
         fEvents.pop_front();
      }
      fEvents.clear();
   }

   void AddToBuffer(const TStoreEvent* event)
   {
      if (fTrace)
         std::cout << event->GetEventNumber() << "\n";
      bool flow_added = false;
      for (TAGDetectorEvent& e: fEvents)
      {
         flow_added = e.AddAnalysisFlow(event);
         if (flow_added)
         {
            if (fTrace)
               std::cout <<"TStoreEvent added to existing buffer\n";
            return;
         }
      }
      fEvents.emplace_back(event);
      if (fTrace)
         std::cout <<"TADetectorEvent constructed with TStoreEvent\n";
   }

   void AddToBuffer(const TBarEvent* b)
   {
      bool flow_added = false;
      if (fTrace)
         std::cout << b->GetID() <<"\n";
      for (TAGDetectorEvent& e: fEvents)
      {
         flow_added = e.AddBarFlow(b);
         if (flow_added)
         {
            if (fTrace)
               std::cout <<"TBarEvent added to existing buffer\n";
            return;
         }
      }
      fEvents.emplace_back(b);
      if (fTrace)
         std::cout <<"TADetectorEvent constructed with TBarEvent\n";
   }

public:
   TAGDetectorEvent *write_event;
   TTree *EventTree;
   TBranch* EventBranch;

   TAGDetectorEventWriter(TARunInfo* runinfo, TAGDetectorEventWriterFlags* f): TARunObject(runinfo),
                                                 fFlags(f)

   {
      //fEvents.reserve(WriteInterval);
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAGDetectorEvent_Writer";
#endif
   }

   ~TAGDetectorEventWriter()
   {
      if (fTrace)
         printf("TAGDetectorEventWriter::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("TAGDetectorEventWriter::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      write_event = new TAGDetectorEvent();
      EventTree = new TTree("TADetectorEventTree", "TADetectorEventTree");
      EventBranch = EventTree->Branch("TADetectorEvent", &write_event, 32000, 0);
      delete write_event;
      write_event=NULL;

      fTPCEvents = 0;
      fBarEvents = 0;
      fFills = 0;
   }

   void EndRun(TARunInfo* runinfo)
   {
      Flush_Buffer();
      if (fTrace)
         std::cout << "tpc events: " << fTPCEvents << "\tbar events: " << fBarEvents << "\t fills:"<<fFills<<"\n";
      EventTree->Write();
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if( fTrace )
         printf("TAGDetectorEventWriter::AnalyzeFlowEvent, run %d\n", runinfo->fRunNo);

      AgAnalysisFlow *af = flow->Find<AgAnalysisFlow>();
      AgBarEventFlow *bf = flow->Find<AgBarEventFlow>();

      if (af && !bf)
            std::cout <<"Analysis flow but not bar flow" <<std::endl;
      if (!af && bf)
            std::cout <<"Bar flow but not analysis flow flow" <<std::endl;
      if (!af && !bf)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }

      // prepare event to store in TTree
      if (af)
      {
         AddToBuffer(af->fEvent);
         fTPCEvents++;
      }
      if (bf)
      {
         AddToBuffer(bf->BarEvent);
         fBarEvents++;
      }

      if (fEvents.size() > WriteInterval)
         Flush_Buffer();
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("TAGDetectorEventWriter::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class TAGDetectorEventWriterFactory: public TAFactory
{
public:
   TAGDetectorEventWriterFlags fFlags;
public:
   void Help()
   {
      printf("TAGDetectorEventWriterFactory::Help\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      TString json="default";
      printf("TAGDetectorEventWriterFactory::Init!\n");
      for (unsigned i=0; i<args.size(); i++) {
         if( args[i]=="-h" || args[i]=="--help" )
            Help();
      }
   }

   void Finish()
   {
      printf("TAGDetectorEventWriterFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("TAGDetectorEventWriterFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new TAGDetectorEventWriter(runinfo,&fFlags);
   }
};


static TARegister tar(new TAGDetectorEventWriterFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
