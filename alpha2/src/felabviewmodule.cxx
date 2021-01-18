//
// Print MIDAS event to ROOT Tree
//
// Lukas Golino
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include "TTree.h"
#include "A2Flow.h"
#include <sstream>
#include <vector>
#include <iterator>
#include "unistd.h"

#include <iostream>

class felabModuleFlags
{
public:
   bool fPrint = false;
   bool fInitTimeSaved = false;
   bool fFakeRealtime = false;
};
class felabviewModule: public TARunObject
{
private:
   TTree* felabEventTree   = NULL;
   uint32_t initialEventTime;
public:
   felabModuleFlags* fFlags;
   bool fTrace = true;
   std::vector<TTree*> trees;
   
   felabviewModule(TARunInfo* runinfo, felabModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      //ModuleName="Felab View Module";
      if (fTrace)
         printf("felabviewFlow::ctor!\n");
   }

   ~felabviewModule()
   {
      if (fTrace)
         printf("felabviewFlow::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("felabviewFlow::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
    }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("felabviewModule::EndRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
  {
     {
         #ifdef HAVE_CXX11_THREADS
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         #endif
         printf("DEBUG: felabviewModule::AnalyzeFlowEvent.\n");
         felabviewFlowEvent* mf = flow->Find<felabviewFlowEvent>();
         if(mf == 0x0)
         {
            printf("DEBUG: felabviewModule::AnalyzeFlowEvent has recieved a standard  TAFlowEvent. Returning flow and not analysing this event.\n");
            return flow;
         }
         std::string               flm_BankName          = mf->GetBankName();
         std::vector<double>       flm_data              = mf->GetData();
         uint32_t                  flm_MIDAS_TIME        = mf->GetMIDAS_TIME();
         uint32_t                  flm_run_time          = mf->GetRunTime();
         //double                  flm_labview_time      = mf->GetLabviewTime();

         std::string name = mf->GetBankName();
         int numberoftrees = trees.size();
         bool treeAlreadyExists;
         TTree* currentTree;

         for(int i=0; i<numberoftrees; i++)
         {
            std::string treename = trees[i]->GetName();
            std::string currenteventname = name.c_str();
            if(treename == currenteventname)
            {
               treeAlreadyExists = true;
               currentTree = trees[i]->GetTree();
            }
         }
         if(!treeAlreadyExists)
         {
            currentTree = new TTree(name.c_str(),"Tree with vectors");
         }

         printf("Number of branches = %d", currentTree->GetNbranches());
         printf("Number of branches = %lld", currentTree->GetEntries());
         currentTree->Branch("data",&flm_data);
         currentTree->Branch("t",&flm_MIDAS_TIME);
         currentTree->Branch("rt",&flm_run_time);
         currentTree->Fill();

         if(!treeAlreadyExists)
         {
            trees.push_back(currentTree);
         }
      }
      return flow; 
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
     if(me->event_id == 6)
      {
         if(!fFlags->fInitTimeSaved)
         {
            initialEventTime = me->time_stamp;
            fFlags->fInitTimeSaved = true;
         }
         
         u32 time_stamp = me->time_stamp;
         u32 dataoff = me->data_offset;
         
         me->FindAllBanks();
         if(me->banks.size()>1)
         {
            printf("\n \n \n DEBUG: felabviewModule::Analyze. Has multiple banks. Exit with error code 11. This should never be hit. \n \n \n");
            exit(11);
         }

         std::string BN = me->banks[0].name.c_str();

         double * rawmeData = (double*)me->GetBankData(&me->banks[0]);
         int N = (int)(me->banks[0].data_size / 8);
         std::vector<double> meData;
         for (int i=0; i<N; i++)
         {
            meData.push_back(rawmeData[i]);
         }

         felabviewFlowEvent* f = new felabviewFlowEvent(flow, BN, meData, me->time_stamp, (me->time_stamp - initialEventTime), meData[0]);
         flow = f;
      }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
       if (fTrace)
          printf("felabviewModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                 runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class FelabViewFactory: public TAFactory
{
public:
   felabModuleFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("FelabViewFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--fakerealtime")
            fFlags.fFakeRealtime = true;
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("FelabViewFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("FelabViewFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new felabviewModule(runinfo, &fFlags);
   }
};

class felabModulePrinter
{

};

static TARegister tar(new FelabViewFactory);
