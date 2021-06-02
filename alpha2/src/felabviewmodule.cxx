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
      std::vector<TTree*> trees;
      std::vector<TBranch*> dataBranchVec;
      std::vector<TBranch*> MIDAS_TIMEBranchVec;
      std::vector<TBranch*> run_timeBranchVec;
      int runNumber;
   private:
      void BranchTreeFromData(TTree* t, TStoreLabVIEWEvent* LVEvent)
      {
         TBranch* TAObj_b = t->GetBranch("TStoreLabVIEWEvent");
         if (!TAObj_b)
            t->Branch("TStoreLabVIEWEvent",&LVEvent);
         {
            t->SetBranchAddress("TStoreLabVIEWEvent",&LVEvent);
         }
         
         t->Fill();
      }
      TTree* FindOrCreateTree(TARunInfo* runinfo, felabviewFlowEvent* mf)
      {
         std::string name = mf->GetBankName();
         int numberoftrees = trees.size();
         bool treeAlreadyExists = false;
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
            runinfo->fRoot->fOutputFile->cd("felabview");
            currentTree = new TTree(name.c_str(),"Tree with vectors");
            trees.push_back(currentTree);
         }
         return currentTree;
      }
      TStoreLabVIEWEvent CreateTAObjectFromFlow(TARunInfo* runinfo, felabviewFlowEvent* mf)
      {
         std::string t_BankName = mf->GetBankName();
         std::vector<double> t_data = *mf->GetData();
         uint32_t t_MIDAS_TIME = mf->GetMIDAS_TIME();
         double t_run_time = mf->GetRunTime();
         double t_labview_time = mf->GetLabviewTime();
         return TStoreLabVIEWEvent(t_BankName, t_data, t_MIDAS_TIME, t_run_time, t_labview_time, runNumber);
      }
      public:
      void WriteTrees(TARunInfo* runinfo)
      {
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         runinfo->fRoot->fOutputFile->cd("felabview");
         for (TTree* t: trees)
            t->Write();
      }
      void SaveToTree(TARunInfo* runinfo, felabviewFlowEvent* mf)
      {
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         TTree* t = FindOrCreateTree(runinfo, mf);
         TStoreLabVIEWEvent m_LabViewEvent = CreateTAObjectFromFlow(runinfo, mf);
         BranchTreeFromData(t, &m_LabViewEvent);  
      }
      void SetRunNumber(int RunNumber)
      {
         runNumber = RunNumber;
      }

};

class felabviewModule: public TARunObject
{
private:
   TTree* felabEventTree   = NULL;
   uint32_t initialEventTime;
   felabViewModuleWriter treeWriter;
   bool fInitTimeSaved = false;

public:
   felabModuleFlags* fFlags;
   bool fTrace = false;
   std::vector<TTree*> trees;
   
   felabviewModule(TARunInfo* runinfo, felabModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
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

   void BeginRun(TARunInfo* runinfo)
   {
      runinfo->fRoot->fOutputFile->cd();
      treeWriter.SetRunNumber(runinfo->fRunNo);
      gDirectory->mkdir("felabview")->cd();
      if (fTrace)
         printf("felabviewFlow::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
    }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("felabviewModule::EndRun, run %d\n", runinfo->fRunNo);
      treeWriter.WriteTrees(runinfo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
  {
     {
         felabviewFlowEvent* mf = flow->Find<felabviewFlowEvent>();
         if(mf == 0x0)
         {
#ifdef MANALYZER_PROFILER
            *flags |= TAFlag_SKIP_PROFILE;
#endif
            //printf("DEBUG: felabviewModule::AnalyzeFlowEvent has recieved a standard  TAFlowEvent. Returning flow and not analysing this event.\n");
            return flow;
         }
         treeWriter.SaveToTree(runinfo, mf);
      }
      return flow; 
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
     if(me->event_id == 6)
      {
         if(!fInitTimeSaved)
         {
            initialEventTime = me->time_stamp;
            fInitTimeSaved = true;
         }
         
         uint32_t time_stamp = me->time_stamp;
         
         me->FindAllBanks();
         if(me->banks.size()>1)
         {
            //printf("\n \n \n DEBUG: felabviewModule::Analyze. Has multiple banks. Exit with error code 11. This should never be hit. \n \n \n");
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
         // I need a range check to assure meData[0] is the right format
         double runTime = meData[0] - (double)initialEventTime - (double)2082844800;
         felabviewFlowEvent* f = new felabviewFlowEvent(flow, BN, meData, me->time_stamp, runTime, meData[0]);
         flow = f;
         if (fTrace)
         if (BN=="D243")
         {
             
             if(meData[0]-3525550000 > 0)
                printf("Timestamp of this event is = %f \n", meData[0]-3525550000);
                for (int i=0; i<meData.size(); i++)
                   std::cout<<meData[i]<<std::endl;
			}
      }
      else
      {  //No work done... skip profiler
#ifdef MANALYZER_PROFILER
         *flags |= TAFlag_SKIP_PROFILE;
#endif
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

static TARegister tar(new FelabViewFactory);
