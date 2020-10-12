#include "AgFlow.h"
#include "RecoFlow.h"

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
   bool fTrace = false;

   MatchFlags() // ctor
   { }

   ~MatchFlags() // dtor
   { }
};

class MatchModule: public TARunObject
{
public:
   MatchFlags* fFlags = NULL;
   bool fTrace;
   int fCounter = 0;
   bool diagnostic = false;

private:
   Match* match;

public:

   MatchModule(TARunInfo* runinfo, MatchFlags* f)
      : TARunObject(runinfo)
   {
      ModuleName="Match Module";
      if (fTrace)
         printf("MatchModule::ctor!\n");

      fFlags = f;
      diagnostic=fFlags->fDiag; // dis/en-able histogramming
      fTrace=fFlags->fTrace; // enable verbosity
   }

   ~MatchModule()
   {
      if(fTrace)
         printf("MatchModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if(fTrace)
         printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fCounter = 0;
      match=new Match(fFlags->ana_settings);
      match->SetTrace(fTrace);
      match->SetDiagnostic(diagnostic);
      match->Setup(runinfo->fRoot->fOutputFile);
   }
   void EndRun(TARunInfo* runinfo)
   {
      if(fTrace)
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
      if(fTrace)
         printf("MatchModule::Analyze, run %d, counter %d\n",
                runinfo->fRunNo, fCounter++);

      // turn off recostruction
      if (fFlags->fRecOff)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      

      const AgEventFlow* ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      
      if (fFlags->fTimeCut)
         {
            if (ef->fEvent->time<fFlags->start_time)
            {
               *flags|=TAFlag_SKIP_PROFILE;
                return flow;
            }
            if (ef->fEvent->time>fFlags->stop_time)
            {
               *flags|=TAFlag_SKIP_PROFILE;
               return flow;
            }
         }

      if (fFlags->fEventRangeCut)
         {
            if (ef->fEvent->counter<fFlags->start_event)
            {
               *flags|=TAFlag_SKIP_PROFILE;
               return flow;
            }
            if (ef->fEvent->counter>fFlags->stop_event)
            {
               *flags|=TAFlag_SKIP_PROFILE;
               return flow;
            }
         }

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) 
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }

      START_TIMER

      if( ! SigFlow->awSig )
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      if( fTrace )
      {
         printf("MatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig->size()));
         printf("MatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig->size()));
      }  
     match->Init();
     std::vector<signal>* combinedPads = NULL;
     if( SigFlow->pdSig )
         {
            std::vector< std::vector<signal> > comb = match->CombPads( SigFlow->pdSig );
            flow = new UserProfilerFlow(flow,"match_module(Comb)",timer_start);
            timer_start=CLOCK_NOW

            combinedPads = match->CombinePads( &comb );
            flow = new UserProfilerFlow(flow,"match_module(CombinePads)",timer_start);
            timer_start=CLOCK_NOW
         }

      // allow events without pwbs
      if( combinedPads )
         {
            if( fTrace )
               printf("MatchModule::Analyze, combined pads # %d\n", int(combinedPads->size()));
            SigFlow->DeletePadSignals(); //Replace pad signals with combined ones
            SigFlow->AddPadSignals( combinedPads );
            match->MatchElectrodes( SigFlow->awSig,combinedPads );
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

      //++fCounter;
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
            if( args[i] == "--trace" )
               fFlags.fTrace = true;
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
