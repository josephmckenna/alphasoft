

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"

#include "A2Flow.h"

#include "TApplication.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"

#include "AnalysisTimer.h"


#include "../OnlineMVA/weights/alphaClassification_BDTF.class.C"
class OnlineMVAFlags
{
public:
   bool fPrint = false;

};


class OnlineMVA: public TARunObject
{
private:
   ReadBDTF* r;
   TString gmethodName;
   TString gdir;
   double grfcut;
   
   std::vector<std::string> input_vars;
   std::vector<double>      input_vals;
  //TString gVarList="nhits,residual,r,S0rawPerp,S0axisrawZ,phi_S0axisraw,nCT,nGT,tracksdca,curvemin,curvemean,lambdamin,lambdamean,curvesign,";
  
public:
   OnlineMVAFlags* fFlags;
   bool fTrace = false;
   
   
   OnlineMVA(TARunInfo* runinfo, OnlineMVAFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("OnlineMVA::ctor!\n");
      
      input_vars={ "phi_S0axisraw", "S0axisrawZ", "S0rawPerp", "residual", "nhits", "phi", "r", "nCT", "nGT" };
      r=new ReadBDTF(input_vars);
      //~4mHz Background (42% efficiency)
      grfcut=0.398139;
      //45mHz Background (72% efficiency)
      //grfcut=0.230254;
      //100mHz Background (78% efficiency)
      //grfcut=0.163; 


      

      
   }

   ~OnlineMVA()
   {
      if (fTrace)
         printf("OnlineMVA::dtor!\n");
      delete r;
   }
   
   
   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OnlineMVA::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OnlineMVA::EndRun, run %d\n", runinfo->fRunNo);
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OnlineMVA::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }
  
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      A2OnlineMVAFlow* dumper_flow=flow->Find<A2OnlineMVAFlow>();
      if (!dumper_flow)
         return flow;
      OnlineMVAStruct* OnlineVars=dumper_flow->dumper_event;

      //    "phi_S0axisraw", "S0axisrawZ", "S0rawPerp", "residual", "nhits", "phi", "", "nCT", "nGT"
      input_vals.clear();
      input_vals.push_back(OnlineVars->phi_S0axisraw);
      input_vals.push_back(OnlineVars->S0axisrawZ);
      input_vals.push_back(OnlineVars->S0rawPerp);
      input_vals.push_back(OnlineVars->residual);
      input_vals.push_back(OnlineVars->nhits);
      input_vals.push_back(OnlineVars->phi);
      input_vals.push_back(OnlineVars->r);
      input_vals.push_back(OnlineVars->nCT);
      input_vals.push_back(OnlineVars->nGT);

      double rfout=r->GetMvaValue(input_vals);
      dumper_flow->rfout=rfout;
      dumper_flow->pass_online_mva=(rfout>grfcut);
      #ifdef _TIME_ANALYSIS_
         clock_t timer_start=clock();
      #endif
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"OnlineMVA_module",timer_start);
      #endif
      return flow; 
  }

};

class OnlineMVAFactory: public TAFactory
{
public:
   OnlineMVAFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("OnlineMVAFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      printf("OnlineMVAFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("OnlineMVAFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new OnlineMVA(runinfo, &fFlags);
   }
};

static TARegister tar(new OnlineMVAFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
