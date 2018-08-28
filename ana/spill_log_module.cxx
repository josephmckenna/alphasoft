//
// spill_log_module
//
// JTK McKenna
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include "TTree.h"

#include <list>
#include <vector>
#include "TSpill.h"

#include "TGFrame.h"
#include "TGListBox.h"
#include "TGTextEdit.h"
#include "TGNumberEntry.h"

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

#define CLOCK_SPEED 50000000
#define N_CHRONOBOARDS 1
#define CLOCK_CHANNEL 58

class SpillLogFlags
{
public:
   bool fPrint = false;


};

class SpillLog: public TARunObject
{
public: 
   SpillLogFlags* fFlags;
   bool fTrace = false;
private:

public:



  //Chronobox channels
  Int_t clock[N_CHRONOBOARDS];
  
  
  Int_t DetectorChans[2];
  std::vector<Double_t> DetectorTS[2];
  std::vector<Int_t> DetectorCounts[2];
  

  time_t gTime; // system timestamp of the midasevent

  std::list<TSpill*> Spill_List;

  TGMainFrame* fMainFrameGUI = NULL;
  TGListBox* fListBoxSeq[4]; 
  TGListBox* fListBoxLogger;
  TGTextEdit* fTextEditBuffer;
  TGTextButton *fTextButtonCopy;

  TGNumberEntry* fNumberEntryDump[4];
  TGNumberEntry* fNumberEntryTS[4];

   
   SpillLog(TARunInfo* runinfo, SpillLogFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("SpillLog::ctor!\n");
   }

   ~SpillLog()
   {
      if (fTrace)
         printf("SpillLog::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      
      for (int i=0; i<N_CHRONOBOARDS; i++)
        clock[i]=CLOCK_CHANNEL;
      DetectorChans[0]=16;
      DetectorChans[1]=17;
      

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::EndRun, run %d\n", runinfo->fRunNo);
      for (int i=0; i<2; i++)
      {
         printf("DETSIZE:%d\n",DetectorTS[i].size());
         printf("COUNTSIZE:%d\n",DetectorCounts[i].size());
      }

   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      
      //AgEventFlow *ef = flow->Find<AgEventFlow>();
      //if (!ef || !ef->fEvent)
      //   return flow;
      const AgChronoFlow* ChronoFlow = flow->Find<AgChronoFlow>();
      if (!ChronoFlow) 
      {
        const AgDumpFlow* DumpFlow = flow->Find<AgDumpFlow>();
        if (!DumpFlow) return flow;
        else
        { // I am a Dump Flow
        
        }
      }
      else //I am a chrono flow
      {
        
         for (int i=0; i<2; i++)
         {
            if (!ChronoFlow->Counts[DetectorChans[i]]) continue;
            DetectorTS[i].push_back(ChronoFlow->RunTime[DetectorChans[i]]);
            DetectorCounts[i].push_back(ChronoFlow->Counts[DetectorChans[i]]);
         }
      }
      //delete flow?
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("SpillLog::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class SpillLogFactory: public TAFactory
{
public:
   SpillLogFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("SpillLogFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      printf("SpillLogFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("SpillLogFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new SpillLog(runinfo, &fFlags);
   }
};

static TARegister tar(new SpillLogFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
