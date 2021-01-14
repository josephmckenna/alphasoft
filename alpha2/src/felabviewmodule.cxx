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

//usleep
#include "unistd.h"

#include <iostream>
struct felabviewEventTreeStruct
{
   TTree* felabEventTree = NULL;
   TBranch* dataBranch = NULL;
   TBranch* IDBranch = NULL;
   TBranch* BNBranch = NULL;
};

class felabModuleFlags
{
public:
   bool fPrint = false;
   bool fFakeRealtime = false;
};
class felabviewModule: public TARunObject
{
private:
   std::vector<felabviewEventTreeStruct> rootTrees; 
   TTree* felabEventTree   = NULL;
public:
   felabModuleFlags* fFlags;
   bool fTrace = true;
   
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

  //Setters and Getters
  void AddStructToRootTree(felabviewEventTreeStruct tStruct)
  {
     rootTrees.push_back(tStruct);
  }             
   /*void                 SetfelabEventTree(TTree* m_felabtree)   { felabEventTree = m_felabtree; }
   TTree*               GetfelabEventTree()                     { return felabEventTree; }
   void                 SetdataBranch(TBranch* m_dataBranch)    { dataBranch = m_dataBranch; }
   TBranch*             GetdataBranch()                         { return dataBranch; }
   void                 SetIDBranch(TBranch* m_IDBranch)        { IDBranch = m_IDBranch; }
   TBranch*             GetIDBranch()                           { return IDBranch; }
   void                 SetBNBranch(TBranch* m_BNBranch)        { BNBranch = m_BNBranch; }
   TBranch*             GetBNBranch()                           { return BNBranch; }*/

   void BeginRun(TARunInfo* runinfo)
   {
      printf("\n \n \n DEBUG: felabviewModule::BeginRun. \n \n \n");
    //   if (fTrace)
    //      printf("RealTimeModule::BeginRun, run %d, file %s\n", flowevent->fRunNo, flowevent->fFileName.c_str());
    //   start_time=clock();
    }

   void EndRun(TARunInfo* runinfo)
   {
      printf("\n \n \n DEBUG: felabviewModule::EndRun. \n \n \n");
    //   if (fTrace)
    //      printf("RealTimeModule::EndRun, run %d\n", flowevent->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
  {
     {
         felabviewFlowEvent* mf = flow->Find<felabviewFlowEvent>();
         if(mf == 0x0)
         {
            printf("\n \n \n DEBUG: felabviewModule::AnalyzeFlowEvent has recieved a standard  TAFlowEvent. Returning flow and not analysing this event.\n\n\n");
            return flow;
         }
         printf("\n \n \n DEBUG: felabviewModule::AnalyzeFlowEvent.");
         //std::string             flm_BankName          = mf->GetBankName();
         std::vector<double>*    flm_data            = mf->GetData();
         //uint32_t*               flm_MIDAS_TIME        = mf->GetMIDAS_TIME();
         //uint32_t*               flm_run_time          = mf->GetRunTime();
         //double                  flm_labview_time      = mf->GetLabviewTime();
         //This should now take an felabview flow event and then print and save to tree and return the same flow I guess.

         //printf("For bank %s: MidasTime: %d, run_time: %d, labview_time: %f \n", mf->GetBankName().c_str(),*flm_MIDAS_TIME,*flm_run_time,flm_labview_time);

         int data_length = mf->GetData()->size();
         if(data_length>0)
         {
            printf("\n data_length = %d \n", data_length);
            for(int i=0; i<data_length; i++)
            {
               printf("For databank %d: Data: %f \n", i, (*mf->GetData())[i]);
            }
         }
         felabviewEventTreeStruct myTree;

         const char *c = mf->GetBankName().c_str();
         runinfo->fRoot->fOutputFile->cd();
         if (!felabEventTree)
            felabEventTree = new TTree("vector<double>","vectordata");
         TBranch* b_variable = felabEventTree->GetBranch("vectordata");
         if (!b_variable)
            felabEventTree->Branch("vectordata","vector<double>",&flm_data,16000,1);
         else
            felabEventTree->SetBranchAddress("vectordata",&flm_data);
         felabEventTree->Fill();

         /*
         myTree.felabEventTree = new TTree(c,c);

         myTree.felabEventTree->Branch("dataBranch","std::vector<double>",&m_data,16000,1);
         myTree.felabEventTree->SetBranchAddress("dataBranch",&m_data);
         myTree.dataBranch = myTree.felabEventTree->GetBranch("dataBranch");

         myTree.felabEventTree->Branch("IDBranch","int",&MIDAS_TIME,16000,1);
         myTree.felabEventTree->SetBranchAddress("IDBranch",&MIDAS_TIME);
         myTree.dataBranch = myTree.felabEventTree->GetBranch("IDBranch");

         myTree.felabEventTree->Branch("BNBranch","int",&run_time,16000,1);
         myTree.felabEventTree->SetBranchAddress("BNBranch",&run_time);
         myTree.dataBranch = myTree.felabEventTree->GetBranch("BNBranch");
      
         myTree.felabEventTree->Fill();
         */


         AddStructToRootTree(myTree);
      }


      return flow; 
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
     //Here we need convert a midas event with an ID of 6 and return a felabviewFlow event.
     if(me->event_id == 6)
      {
         printf("DEBUG: felabviewModule::Analyze. ID = %d\n", me->event_id);
         
         //We have the midas event we are looking for.
         //Convert to a new felabview flow event then return. 
         //So initialise a felabviewFE with the values found in me and return the flow.
         u32 time_stamp = me->time_stamp;
         u32 dataoff = me->data_offset;
         
         me->FindAllBanks();
         if(me->banks.size()>1)
         {
            printf("\n \n \n DEBUG: felabviewModule::Analyze. Has multiple banks. Exit with error code 11. \n \n \n");
            exit(11);
         }
         std::string BN = me->banks[0].name.c_str();

         double * rawmeData = (double*)me->GetBankData(&me->banks[0]);
         int N = (int)(me->banks[0].data_size / 8);
         //std::cout<<data1[0]<<"\t"<<data1[1]<<data1[2]<<"\t"<<data1[3]<<std::endl;
         std::vector<double> meData;
         for (int i=0; i<N; i++)
         {
            meData.push_back(rawmeData[i]);
         }

         felabviewFlowEvent* f = new felabviewFlowEvent(flow, BN, meData, me->time_stamp, me->time_stamp, meData[0]);
         flow = f;
      }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("\n \n \n DEBUG: felabviewModule::AnalyzeSpecialEvent. ID = %d \n \n \n", event->event_id);
      
       //if (fTrace)
          //printf("RealTimeModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                 //event->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
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
