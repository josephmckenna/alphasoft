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
#include "TStoreLabVIEWEvent.h"

#include <iostream>

class felabModuleFlags
{
public:
   bool fPrint = false;
   bool fFakeRealtime = false;
};

class felabViewModuleWriter
{
   private:
      std::vector<TTree*> fTrees;
      std::vector<TBranch*> fDataBranches;
      std::vector<TBranch*> fMIDASTimeBranches;
      std::vector<TBranch*> fRunTimeBranches;
      int fRunNo;
   private:
      void BranchTreeFromData(TTree* tree, TStoreLabVIEWEvent* labviewEvent)
      {
         TBranch* branch = tree->GetBranch("TStoreLabVIEWEvent");
         if (!branch)
            tree->Branch("TStoreLabVIEWEvent",&labviewEvent);
         {
            tree->SetBranchAddress("TStoreLabVIEWEvent",&labviewEvent);
         }
         tree->Fill();
      }
      TTree* FindOrCreateTree(TARunInfo* runInfo, felabviewFlowEvent* flowEvent)
      {
         std::string name = flowEvent->GetBankName();
         int numberOfTrees = fTrees.size();
         bool treeAlreadyExists = false;
         TTree* currentTree;

         for(int i=0; i<numberOfTrees; i++)
         {
            std::string treeName = fTrees[i]->GetName();
            std::string currentEventName = name.c_str();
            if(treeName == currentEventName)
            {
               treeAlreadyExists = true;
               currentTree = fTrees[i]->GetTree();
            }
         }
         if(!treeAlreadyExists)
         {
            runInfo->fRoot->fOutputFile->cd("felabview");
            currentTree = new TTree(name.c_str(), "Tree with vectors");
            fTrees.push_back(currentTree);
         }
         return currentTree;
      }
      TStoreLabVIEWEvent CreateTAObjectFromFlow(TARunInfo* runInfo, felabviewFlowEvent* flowEvent)
      {
         std::string bankName = flowEvent->GetBankName();
         std::vector<double> data = *flowEvent->GetData();
         uint32_t midasTime = flowEvent->GetMIDAS_TIME();
         double runTime = flowEvent->GetRunTime();
         double labviewTime = flowEvent->GetLabviewTime();
         return TStoreLabVIEWEvent(bankName, data, midasTime, runTime, labviewTime, fRunNo);
      }
      public:
      void WriteTrees(TARunInfo* runInfo)
      {
         #ifdef HAVE_CXX11_THREADS
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         #endif

         runInfo->fRoot->fOutputFile->cd("felabview");
         for (TTree* tree: fTrees)
            tree->Write();
      }
      void SaveToTree(TARunInfo* runInfo, felabviewFlowEvent* flowEvent)
      {
         #ifdef HAVE_CXX11_THREADS
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         #endif

         TTree* tree = FindOrCreateTree(runInfo, flowEvent);
         TStoreLabVIEWEvent labviewEvent = CreateTAObjectFromFlow(runInfo, flowEvent);
         BranchTreeFromData(tree, &labviewEvent);  
      }
      void SetRunNumber(int runNumber)
      {
         fRunNo = runNumber;
      }

};

class felabviewModule: public TARunObject
{
private:
   TTree* fFELabEventTree   = NULL;
   uint32_t fInitialEventTime;
   felabViewModuleWriter fTreeWriter;
   bool fInitTimeSaved = false;

public:
   felabModuleFlags* fFlags;
   bool fTrace = false;
   std::vector<TTree*> fTrees;
   
   felabviewModule(TARunInfo* runInfo, felabModuleFlags* flags)
      : TARunObject(runInfo), fFlags(flags)
   {
#ifdef MANALYZER_PROFILER
      ModuleName="felabview Module";
#endif
      if (fTrace)
         printf("felabviewFlow::ctor!\n");
   }

   ~felabviewModule()
   {
      if (fTrace)
         printf("felabviewFlow::dtor!\n");
   }

   void BeginRun(TARunInfo* runInfo)
   {
      runInfo->fRoot->fOutputFile->cd();
      fTreeWriter.SetRunNumber(runInfo->fRunNo);
      gDirectory->mkdir("felabview")->cd();
      if (fTrace)
         printf("felabviewFlow::BeginRun, run %d, file %s\n", runInfo->fRunNo, runInfo->fFileName.c_str());
    }

   void EndRun(TARunInfo* runInfo)
   {
      if (fTrace)
         printf("felabviewModule::EndRun, run %d\n", runInfo->fRunNo);
      fTreeWriter.WriteTrees(runInfo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runInfo, TAFlags* flags, TAFlowEvent* flow)
  {
     {
         felabviewFlowEvent* flowEvent = flow->Find<felabviewFlowEvent>();
         if(flowEvent == 0x0)
         {
#ifdef MANALYZER_PROFILER
            *flags |= TAFlag_SKIP_PROFILE;
#endif
            //printf("DEBUG: felabviewModule::AnalyzeFlowEvent has recieved a standard  TAFlowEvent. Returning flow and not analysing this event.\n");
            return flow;
         }
         fTreeWriter.SaveToTree(runInfo, flowEvent);
      }
      return flow; 
   }

   TAFlowEvent* Analyze(TARunInfo* runInfo, TMEvent* midasEvent, TAFlags* flags, TAFlowEvent* flow)
   {
     if(midasEvent->event_id == 6)
      {
         if(!fInitTimeSaved)
         {
            fInitialEventTime = midasEvent->time_stamp;
            fInitTimeSaved = true;
         }
         
         u32 timeStamp = midasEvent->time_stamp;
         u32 dataOffset = midasEvent->data_offset;

         midasEvent->FindAllBanks();
         if(midasEvent->banks.size()>1)
         {
            //printf("\n \n \n DEBUG: felabviewModule::Analyze. Has multiple banks. Exit with error code 11. This should never be hit. \n \n \n");
            exit(11);
         }
         

         std::string currentBankName = midasEvent->banks[0].name.c_str();
         double * rawMIDASData = (double*)midasEvent->GetBankData(&midasEvent->banks[0]);
         int dataSize = (int)(midasEvent->banks[0].data_size / 8);
         std::vector<double> midasEventData;
         for (int i=0; i<dataSize; i++)
         {
            midasEventData.push_back(rawMIDASData[i]);
         }

         double runTime;
         // I need a range check to assure meData[0] is the right format if not use MIDAS time.
         if( (midasEventData[0]-2082844800) - (midasEvent->time_stamp) > 5)
         {
            runTime = midasEvent->time_stamp;
         }
         felabviewFlowEvent* flowEvent = new felabviewFlowEvent(flow, currentBankName, midasEventData, midasEvent->time_stamp, runTime, midasEventData[0]);
         flow = flowEvent;
         

         //=== Debug Logging ===
         /*if( (midasEventData[0]-2082844800) - (midasEvent->time_stamp) > 5)
         {
            printf("Timestamp clash: (lv time) %f - %d (midas time) = %f for bank: %s \n", midasEventData[0]-2082844800, midasEvent->time_stamp, midasEventData[0] - 2082844800 - midasEvent->time_stamp, currentBankName.c_str());
         }
         std::string bankName = "HPRO";
         if (BN==bn)
         {
             
             if(meData[0]-3525550000 > 0)
                printf("\n\n   LV Timestamp of this event is = %f \n", meData[0]-2082844800);
                printf("   MI Timestamp of this event is = %d \n\n", me->time_stamp);

                //for (int i=0; i<meData.size(); i++)
                   //std::cout<<meData[i]<<std::endl;
			}*/
         //=== End of Logging ===


      }
      else
      {  //No work done... skip profiler
#ifdef MANALYZER_PROFILER
         *flags |= TAFlag_SKIP_PROFILE;
#endif
      }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runInfo, TMEvent* event)
   {
       if (fTrace)
          printf("felabviewModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                 runInfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
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
   
   TARunObject* NewRunObject(TARunInfo* runInfo)
   {
      printf("FelabViewFactory::NewRunObject, run %d, file %s\n", runInfo->fRunNo, runInfo->fFileName.c_str());
      return new felabviewModule(runInfo, &fFlags);
   }
};

static TARegister tar(new FelabViewFactory);
