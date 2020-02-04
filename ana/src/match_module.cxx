#include "AgFlow.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TMath.h"
#include "TSpectrum.h"
#include "TFitResult.h"
#include "Math/MinimizerOptions.h"

#include "SignalsType.hh"
#include <set>
#include <iostream>

#include "AnalysisTimer.h"
#include "AnaSettings.h"
#include "Match.hh"

class MatchFlags
{
public:
   bool fRecOff = false; //Turn reconstruction off
   bool fTimeCut = false;
   bool fUseSpec = false;
   double start_time = -1.;
   double stop_time = -1.;
   bool fEventRangeCut = false;
   int start_event = -1;
   int stop_event = -1;
   AnaSettings* ana_settings=NULL;
   bool fDiag = false;
   MatchFlags() // ctor
   { }

   ~MatchFlags() // dtor
   { }
};

class MatchModule: public TARunObject
{
public:
   MatchFlags* fFlags = NULL;
   bool fTrace = false;
   //bool fTrace = true;
   int fCounter = 0;
   bool diagnostic = false;

private:
   Match* match;

public:

   MatchModule(TARunInfo* runinfo, MatchFlags* f)
      : TARunObject(runinfo)
   {
      if (fTrace)
         printf("MatchModule::ctor!\n");

      fFlags = f;
      diagnostic=fFlags->fDiag; // dis/en-able histogramming
   }

   ~MatchModule()
   {
      if(fTrace)
         printf("MatchModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      //if(fTrace)
      printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;
      match=new Match(fFlags->ana_settings);
      if(fTrace) 
         match->SetTrace(true);
      match->SetDiagnostic(diagnostic);
      //      if( diagnostic ) 
      match->Setup(runinfo->fRoot->fOutputFile);
   }
   void EndRun(TARunInfo* runinfo)
   {
      //if(fTrace)
      printf("MatchModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
      delete match;
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      // turn off recostruction
      if (fFlags->fRecOff)
         return flow;

      if(fTrace)
         printf("MatchModule::Analyze, run %d, counter %d\n",
                runinfo->fRunNo, fCounter);
      const AgEventFlow* ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      if (fFlags->fTimeCut)
         {
            if (ef->fEvent->time<fFlags->start_time)
               return flow;
            if (ef->fEvent->time>fFlags->stop_time)
               return flow;
         }

      if (fFlags->fEventRangeCut)
         {
            if (ef->fEvent->counter<fFlags->start_event)
               return flow;
            if (ef->fEvent->counter>fFlags->stop_event)
               return flow;
         }

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) return flow;
      #ifdef _TIME_ANALYSIS_
      START_TIMER
      #endif   
      if( ! SigFlow->awSig ) return flow;
      if( fTrace )
         printf("MatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig->size()));
      
      if( fTrace )
         printf("MatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig->size()));
         
     match->Init();
     if( SigFlow->pdSig )
         {
            match->CombinePads(SigFlow->pdSig);
            #ifdef _TIME_ANALYSIS_
            if (TimeModules) flow=new AgAnalysisReportFlow(flow,"match_module(CombinePads)",timer_start);
            timer_start=CLOCK_NOW
            #endif
         }

      // allow events without pwbs
      if( match->GetCombinedPads() )
         {
            printf("MatchModule::Analyze, combined pads # %d\n", int(match->GetCombinedPads()->size()));
            SigFlow->DeletePadSignals(); //Replace pad signals with combined ones
            SigFlow->AddPadSignals(match->GetCombinedPads());
            match->MatchElectrodes( SigFlow->awSig );
            match->CombPoints();
         }
      else // <-- this probably goes before, where there are no pad signals -- AC 2019-6-3
         {
            printf("MatchModule::Analyze, NO combined pads, Set Z=0\n");
//delete match->GetCombinedPads();?
            match->FakePads( SigFlow->awSig );
         }

      if( match->GetSpacePoints() )
         printf("MatchModule::Analyze, Spacepoints # %d\n", int(match->GetSpacePoints()->size()));
      else
         printf("MatchModule::Analyze Spacepoints should exists at this point\n");
      if( match->GetSpacePoints()->size() > 0 )
         SigFlow->AddMatchSignals( match->GetSpacePoints() );

      ++fCounter;
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"match_module",timer_start);
      #endif
      return flow;
   }
};


class MatchModuleFactory: public TAFactory
{
public:
   MatchFlags fFlags;

public:

   void Init(const std::vector<std::string> &args)
   {
      TString json="default";
      printf("MatchModuleFactory::Init!\n");
      for(unsigned i=0; i<args.size(); i++)
         {
            if( args[i] == "--usetimerange" )
               {
                  fFlags.fTimeCut=true;
                  i++;
                  fFlags.start_time=atof(args[i].c_str());
                  i++;
                  fFlags.stop_time=atof(args[i].c_str());
                  printf("Using time range for reconstruction: ");
                  printf("%f - %fs\n",fFlags.start_time,fFlags.stop_time);
               }
            if( args[i] == "--useeventrange" )
               {
                  fFlags.fEventRangeCut=true;
                  i++;
                  fFlags.start_event=atoi(args[i].c_str());
                  i++;
                  fFlags.stop_event=atoi(args[i].c_str());
                  printf("Using event range for reconstruction: ");
                  printf("Analyse from (and including) %d to %d\n",fFlags.start_event,fFlags.stop_event);
               }
            if (args[i] == "--recoff")
               fFlags.fRecOff = true;
            if( args[i] == "--diag" )
               fFlags.fDiag = true;
            if (args[i] == "--anasettings")
               {
                  i++;
                  json=args[i];
                  i++;
               }
         }
      fFlags.ana_settings=new AnaSettings(json);
      fFlags.ana_settings->Print();
   }

   void Finish()
   {
      printf("MatchModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("MatchModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new MatchModule(runinfo, &fFlags);
   }
};

static TARegister tar(new MatchModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
