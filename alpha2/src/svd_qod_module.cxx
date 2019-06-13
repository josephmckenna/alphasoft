//
// SVD_QOD 
//
// JTK McKenna

#include "manalyzer.h"
#include "midasio.h"
#include "AgFlow.h"

#include "TTree.h"
#include "TMath.h"
#include "TChrono_Event.h"
#include <iostream>
#include "chrono_module.h"
#include "TChronoChannelName.h"

#include <TBufferJSON.h>
#include <fstream>
#include "AnalysisTimer.h"

#include "A2Flow.h"
class SVD_QODFlags
{
public:
   bool fPrint = false;
   bool fSaveQOD = true;
};

class SVD_QOD: public TARunObject
{
private:
   
   
   TTree* SVDQODTree   = NULL;
   
public:
   SVD_QODFlags* fFlags;
   
   double last_vf48_ts;
      

   bool fTrace = true;

   SVD_QOD(TARunInfo* runinfo, SVD_QODFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("SVD_QOD::ctor!\n");
   }

   ~SVD_QOD()
   {
      if (fTrace)
         printf("SVD_QOD::dtor!\n");
   }
   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SVDQOD::BeginRun, run %d\n", runinfo->fRunNo);
      //printf("SVDQOD::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SVD_QOD::EndRun, run %d\n", runinfo->fRunNo);
  
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SVD_QOD::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
       return flow;
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {

      #ifdef _TIME_ANALYSIS_
         clock_t timer_start=clock();
      #endif
   
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"SVD_QOD_module",timer_start);
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("SVD_QOD::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n",
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class SVD_QODFactory: public TAFactory
{
public:
   SVD_QODFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("SVD_QODFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--printcorruption")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      printf("SVD_QODFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("SVD_QODFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new SVD_QOD(runinfo, &fFlags);
   }
};

static TARegister tar(new SVD_QODFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
