//
// write_to_tree_module.cxx
//
// basic class to write TStoreEvent in the tree at the very end of the flow.
//
// L GOLINO
//

#include <stdio.h>
#include <cassert>
#include <numeric>
#include <algorithm> 
#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

#include "TTree.h"
#include "TBranch.h"

class WriteToTree: public TARunObject
{
public:
   TStoreEvent *fCompleteEvent;
   TTree *fTStoreEventTree;

   WriteToTree(TARunInfo* runinfo): TARunObject(runinfo)
   {
      if(fTrace)
         printf("WriteToTree::ctor!\n");
   }

   ~WriteToTree()
   {
      if(fTrace)
         printf("WriteToTree::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if(fTrace)
         printf("WriteToTree::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      fCompleteEvent = new TStoreEvent;
      fTStoreEventTree = new TTree("StoreEventTree", "StoreEventTree");
      fTStoreEventTree->Branch("StoredEvent", &fCompleteEvent, 32000, 0);
      delete fCompleteEvent;
      fCompleteEvent=NULL;
   }

   void EndRun(TARunInfo* runinfo)
   {
      if(fTrace)
         printf("WriteToTree::EndRun, run %d\n", runinfo->fRunNo);
      fTStoreEventTree->Write();
      runinfo->fRoot->fOutputFile->Write();
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if(fTrace)
         printf("WriteToTree::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if(fTrace)
         printf("WriteToTree::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
       AgAnalysisFlow *ef = flow->Find<AgAnalysisFlow>();

      fCompleteEvent = ef->fEvent;
      {std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      fTStoreEventTree->SetBranchAddress("StoredEvent", &fCompleteEvent);
      fTStoreEventTree->Fill();}
               

      return flow;
   }


   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("WriteToTree::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }

};

class WriteToTreeFactory: public TAFactory
{
public:
   void Init(const std::vector<std::string> &args)
   {
      printf("WriteToTreeFactory::Init!\n");
   }

   void Finish()
   {
      printf("WriteToTreeFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("WriteToTree::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new WriteToTree(runinfo);
   }
};

static TARegister tar(new WriteToTreeFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
