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
class felabModuleFlags
{
public:
   bool fPrint = false;
   bool fFakeRealtime = false;
};
class felabviewModule: public TARunObject
{
private:
   TTree* felabEventTree = NULL;
   TBranch* dataBranch = NULL;
   TBranch* IDBranch = NULL;
   TBranch* BNBranch = NULL;
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
   void                 SetfelabEventTree(TTree* m_felabtree)   { felabEventTree = m_felabtree; }
   TTree*               GetfelabEventTree()                     { return felabEventTree; }
   void                 SetdataBranch(TBranch* m_dataBranch)    { dataBranch = m_dataBranch; }
   TBranch*             GetdataBranch()                         { return dataBranch; }
   void                 SetIDBranch(TBranch* m_IDBranch)        { IDBranch = m_IDBranch; }
   TBranch*             GetIDBranch()                           { return IDBranch; }
   void                 SetBNBranch(TBranch* m_BNBranch)        { BNBranch = m_BNBranch; }
   TBranch*             GetBNBranch()                           { return BNBranch; }

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
      felabviewFlowEvent* mf = flow->Find<felabviewFlowEvent>();
      printf("\n \n \n DEBUG: felabviewModule::AnalyzeFlowEvent.");
      printf("DEBUG: felabviewModule::AnalyzeFlowEvent. Just making sure the flow is an felabviewEvent \n \n \n");
      std::string            BankName          = mf->GetBankName();
      std::vector<double>    m_data            = mf->GetData();
      uint32_t               MIDAS_TIME        = mf->GetMIDAS_TIME();
      uint32_t               run_time          = mf->GetRunTime();
      double                 labview_time      = mf->GetLabviewTime();
      //This should now take an felabview flow event and then print and save to tree and return the same flow I guess.

       printf("For bank %s: MidasTime: %d, run_time: %d, labview_time: %f \n", BankName.c_str(),MIDAS_TIME,run_time,labview_time);

       int data_length = mf->GetData().size();
       if(data_length>0)
       {
         printf("\n data_length = %d \n", data_length);
         for(int i=0; i<data_length; i++)
         {
            printf("For databank %d: Data: %f \n", i, mf->GetData().at(i));
         }
       }

      return flow; 
  }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
     //Here we need convert a midas event with an ID of 6 and return a felabviewFlow event.
     if(me->event_id == 6)
     {
       printf("\n \n \n DEBUG: felabviewModule::Analyze. ID = %d \n \n \n", me->event_id);
       
       //We have the midas event we are looking for.
       //Convert to a new felabview flow event then return. 
       //So initialise a felabviewFE with the values found in me and return the flow.
       u32 time_stamp = me->time_stamp;
       u32 dataoff = me->data_offset;
       std::cout << "time_stamp = " << time_stamp << std::endl;
       std::cout << "data_offset = " << dataoff << std::endl;
       
       me->FindAllBanks();
       printf("For bank %d: BankName: %s, DataSize: %d, DataOffset: %d \n", 0, me->banks[0].name.c_str(), me->banks[0].data_size, me->banks[0].data_offset);

       int data_length = me->data.size();
       if(data_length>0)
       {
         printf("\n data_length = %d \n", data_length);
         for(int i=0; i<data_length; i++)
         {
            printf("For databank %d: Data: %d \n", i, me->data.at(i));
         }
       }

       char* m_pEventData = me->GetEventData();                     ///< get pointer to MIDAS event data
       char* m_pBankData = me->GetBankData(&me->banks[0]);
     }

     std::string BN = me->banks[0].name.c_str();

     double* a=(double*)me->GetEventData();
     std::cout << "We have just created a =" << a << std::endl;

     double * data1 = (double*)me->GetEventData();
     std::cout<<data1[0]<<"\t"<<data1[1]<<std::endl;
     std::cout<<*data1<<"\t"<< *(data1+1)<<std::endl;
     std::cout<< *data1++<<"\t"<<*data1++<<std::endl;

     std::vector<double> meData;
     int N = me->data.size() - me->data_offset;
     for (int i=0; i<N; i++)
     {
         std::cout << "We are pushing back a to create our vector of data. a[" << i << "] = " << a[i] <<std::endl;
         meData.push_back(*(a+i));
     }


      felabviewFlowEvent* f = new felabviewFlowEvent(flow, BN, meData, me->time_stamp, me->time_stamp, meData[0]);
      flow = f;
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("\n \n \n DEBUG: felabviewModule::AnalyzeSpecialEvent. ID = %d \n \n \n", event->event_id);
      if(felabEventTree == NULL)
      {
        felabEventTree = new TTree("felabEventTree","felabEventTree");
        SetfelabEventTree(felabEventTree);
      }
/*
    if(event->event_id == 6)
    {
      TBranch* b_data = felabEventTree->GetBranch("dataBranch");
      if (!b_data)
        felabEventTree->Branch("dataBranch","dataBranch",&event->data,16000,1);
      else
        felabEventTree->SetBranchAddress("dataBranch",&event->data);

      TBranch* b_ID = felabEventTree->GetBranch("IDBranch");
      if (!b_ID)
        felabEventTree->Branch("IDBranch","IDBranch",&event->event_id,16000,1);
      else
        felabEventTree->SetBranchAddress("IDBranch",&event->event_id);

      TBranch* b_BN = felabEventTree->GetBranch("BNBranch");
      if (!b_BN)
        felabEventTree->Branch("BNBranch","BNBranch",&event->banks[0],16000,1);
      else
        felabEventTree->SetBranchAddress("BNBranch",&event->banks[0]);

    
      felabEventTree->Fill();

    }
*/
    //   if (fTrace)
    //      printf("RealTimeModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
    //             flowevent->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
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

struct felabviewEventTree
{
    std::vector<TTree*> felabEventTree;
};
