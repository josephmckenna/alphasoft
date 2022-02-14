//Turning this whole mother off
#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"

#include "RecoFlow.h"
#include "A2Flow.h"

#include "TApplication.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"

//#include "../OnlineMVA/weights/alphaClassification_BDTF.class.C"
//#include "../OnlineMVA/weights/alphaClassification_BDTF.weights.xml"

class OfflineMVAFlags
{
public:
   bool fPrint = false;

};


class OfflineMVA: public TARunObject
{
private:
   //ReadBDTF* r;
   TString gmethodName;
   TString gdir;
   double grfcut;

   TMVA::Reader *reader;
   Float_t phi_S0axisraw, S0axisrawZ, S0rawPerp, residual, nhits, phi, r, nCT, nGT;
   MVAFlowEvent fMVAFlow;
   //vector ;
   
   std::vector<std::string> input_vars;
   std::vector<double>      input_vals;
  //TString gVarList="nhits,residual,r,S0rawPerp,S0axisrawZ,phi_S0axisraw,nCT,nGT,tracksdca,curvemin,curvemean,lambdamin,lambdamean,curvesign,";
  
public:
   OfflineMVAFlags* fFlags;
   bool fTrace = false;
   
   
   OfflineMVA(TARunInfo* runinfo, OfflineMVAFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
#ifdef MANALYZER_PROFILER
      ModuleName="Online MVA Module";
#endif
      if (fTrace)
         printf("OnlineMVA::ctor!\n");
      
      //~4mHz Background (42% efficiency)
      grfcut=0.398139;
      //45mHz Background (72% efficiency)
      //grfcut=0.230254;
      //100mHz Background (78% efficiency)
      //grfcut=0.163; 
   }

   ~OfflineMVA()
   {
      if (fTrace)
         printf("OnlineMVA::dtor!\n");
      delete reader;
   }
   
   
   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OnlineMVA::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      TMVA::Reader *reader = new TMVA::Reader();

      fMVAFlow->LoadVariablesToReader(reader);

      reader->AddVariable( "phi_S0axisraw", &phi_S0axisraw );
      reader->AddVariable( "S0axisrawZ", &S0axisrawZ );
      reader->AddVariable( "S0rawPerp", &S0rawPerp );
      reader->AddVariable( "residual", &residual );
      reader->AddVariable( "nhits", &nhits );
      reader->AddVariable( "phi", &phi );
      reader->AddVariable( "r", &r );
      reader->AddVariable( "nCT", &nCT );
      reader->AddVariable( "nGT", &nGT );

      reader->BookMVA( "BDTF",  "../OnlineMVA/weights/alphaClassification_BDTF.weights.xml" );
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
      {
#ifdef MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      OnlineMVAStruct* OnlineVars=dumper_flow->dumper_event;

      //    "phi_S0axisraw", "S0axisrawZ", "S0rawPerp", "residual", "nhits", "phi", "", "nCT", "nGT"
      phi_S0axisraw     = OnlineVars->phi_S0axisraw;
      S0axisrawZ        = OnlineVars->S0axisrawZ;
      S0rawPerp         = OnlineVars->S0rawPerp;
      residual          = OnlineVars->residual;
      nhits             = OnlineVars->nhits;
      phi               = OnlineVars->phi;
      r                 = OnlineVars->r;
      nCT               = OnlineVars->nCT;
      nGT               = OnlineVars->nGT;

      double rfout = reader->EvaluateMVA( "BDTF" );

      dumper_flow->rfout=rfout;
      dumper_flow->pass_online_mva=(rfout>grfcut);

      return flow; 
  }

};

class OfflineMVAFactory: public TAFactory
{
public:
   OfflineMVAFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("OfflineMVAFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("OfflineMVAFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("OfflineMVAFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new OfflineMVA(runinfo, &fFlags);
   }
};

static TARegister tar(new OfflineMVAFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */


