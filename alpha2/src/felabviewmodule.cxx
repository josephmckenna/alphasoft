//
// Print MIDAS event to ROOT Tree
//
// Lukas Golino
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include "TTree.h"

//usleep
#include "unistd.h"

#include <iostream>
class felabModuleFlags
{
public:
   bool fPrint = false;
   bool fFakeRealtime = false;
};
class felabviewFlow: public TAFlowEvent
{
private:
   std::string BankName;
   std::vector<double> data;
   uint32_t MIDAS_TIME;
   uint32_t run_time;
   TTree* felabEventTree = NULL;
   TBranch* dataBranch = NULL;
   TBranch* IDBranch = NULL;
   TBranch* BNBranch = NULL;
public:
   felabModuleFlags* fFlags;
   bool fTrace = false;

   //Setters and Getters
   void                 SetBankName(std::string m_BankName)     { BankName = m_BankName; }
   std::string          GetBankName()                           { return BankName; }
   void                 SetData(std::vector<double> m_data)     { data = m_data; }
   std::vector<double>  GetData()                               { return data; }
   void                 SetMIDAS_TIME(uint32_t m_MIDAS_TIME)    { MIDAS_TIME = m_MIDAS_TIME; }
   uint32_t             GetMIDAS_TIME()                         { return MIDAS_TIME; }
   void                 SetRunTime(uint32_t m_run_time)         { run_time = m_run_time; }
   uint32_t             GetRunTime()                            { return run_time; }
   void                 SetfelabEventTree(TTree* m_felabtree)   { felabEventTree = m_felabtree; }
   TTree*               GetfelabEventTree()                     { return felabEventTree; }
   void                 SetdataBranch(TBranch* m_dataBranch)    { dataBranch = m_dataBranch; }
   TBranch*             GetdataBranch()                         { return dataBranch; }
   void                 SetIDBranch(TBranch* m_IDBranch)        { IDBranch = m_IDBranch; }
   TBranch*             GetIDBranch()                           { return IDBranch; }
   void                 SetBNBranch(TBranch* m_BNBranch)        { BNBranch = m_BNBranch; }
   TBranch*             GetBNBranch()                           { return BNBranch; }
   
   felabviewFlow(TAFlowEvent* flowevent, felabModuleFlags* flags)
      : TAFlowEvent(flowevent), fFlags(flags)
   {
      //ModuleName="Felab View Module";
      if (fTrace)
         printf("felabviewFlow::ctor!\n");
   }

   ~felabviewFlow()
   {
      if (fTrace)
         printf("felabviewFlow::dtor!\n");
   }

   void BeginRun(TAFlowEvent* flowevent)
   {
    //   if (fTrace)
    //      printf("RealTimeModule::BeginRun, run %d, file %s\n", flowevent->fRunNo, flowevent->fFileName.c_str());
    //   start_time=clock();
    }

   void EndRun(TAFlowEvent* flowevent)
   {
    //   if (fTrace)
    //      printf("RealTimeModule::EndRun, run %d\n", flowevent->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TAFlowEvent* flowevent, TAFlags* flags, TAFlowEvent* flow)
  {
            //Print to tree here.
      return flow; 
  }

   TAFlowEvent* Analyze(TAFlowEvent* flowevent, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
    //   if (fFlags->fFakeRealtime )
    //   {
    //     FirstEvent=me->time_stamp;
    //     clock_t time_now=(clock()-start_time)/CLOCKS_PER_SEC;
    //      while (me->time_stamp-FirstEvent>time_now)
    //      {
    //         time_now=(clock()-start_time)/CLOCKS_PER_SEC;
    //         usleep(1000);
    //      }
    //   }
      return flow;
   }

   void AnalyzeSpecialEvent(TAFlowEvent* flowevent, TMEvent* event)
   {
      if(felabEventTree == NULL)
      {
        felabEventTree = new TTree("felabEventTree","felabEventTree");
        SetfelabEventTree(felabEventTree);
      }

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

    //   if (fTrace)
    //      printf("RealTimeModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
    //             flowevent->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

struct felabviewEventTree
{
    std::vector<TTree*> felabEventTree;



};
