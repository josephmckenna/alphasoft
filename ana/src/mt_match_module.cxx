#include "AgFlow.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TMath.h"
#include "TSpectrum.h"
#include "TFitResult.h"
#include "Math/MinimizerOptions.h"

#include "SignalsType.h"
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

class MatchModuleInit: public TARunObject
{
public:
   MatchFlags* fFlags = NULL;
   bool fTrace = false;
   //bool fTrace = true;
   int fCounter = 0;
   bool diagnostic = false;

public:

   MatchModuleInit(TARunInfo* runinfo, MatchFlags* f)
      : TARunObject(runinfo)
   {
      if (fTrace)
         printf("MatchModule::ctor!\n");

      fFlags = f;
      diagnostic=fFlags->fDiag; // dis/en-able histogramming
   }

   ~MatchModuleInit()
   {
      if(fTrace)
         printf("MatchModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      //if(fTrace)
      printf("BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

   }
   void EndRun(TARunInfo* runinfo)
   {
      //if(fTrace)
      printf("MatchModule::EndRun, run %d    Total Counter %d\n", runinfo->fRunNo, fCounter);
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
            fCounter = 0;
      Match* match=new Match(fFlags->ana_settings);
      if(fTrace) 
         match->SetTrace(true);
      match->SetDiagnostic(diagnostic);
      if( diagnostic ) 
         match->Setup(runinfo->fRoot->fOutputFile);
      if( fTrace )
         printf("MatchModule::Analyze, AW # signals %d\n", int(SigFlow->awSig->size()));
      
      if( fTrace )
         printf("MatchModule::Analyze, PAD # signals %d\n", int(SigFlow->pdSig->size()));
         
      match->Init();
      
      flow=new AgMatchFlow(flow, match);
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"match_module(Init)",timer_start);
      #endif
      return flow;
   }
};

////////////////////////////////////////////////////////////////////////

class MatchModuleComb: public TARunObject
{
public:
   MatchFlags* fFlags = NULL;
   bool fTrace = false;
   //bool fTrace = true;
   int fCounter = 0;
   bool diagnostic = false;

public:

   MatchModuleComb(TARunInfo* runinfo, MatchFlags* f)
      : TARunObject(runinfo)
   {
      if (fTrace)
         printf("MatchModule::ctor!\n");

      fFlags = f;
      diagnostic=fFlags->fDiag; // dis/en-able histogramming
   }

   ~MatchModuleComb()
   {
      if(fTrace)
         printf("MatchModule::dtor!\n");
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
      if( ! SigFlow->pdSig ) return flow;
            fCounter = 0;
      AgMatchFlow* MatchFlow= flow->Find<AgMatchFlow>();
      if (! MatchFlow->match) return flow;
      MatchFlow->comb=MatchFlow->match->CombPads(SigFlow->pdSig);
      MatchFlow->match->NewCombinedPads(); //Prepare a new std::vector
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"match_module (Comb)",timer_start);
      #endif
      return flow;
  }
};      

////////////////////////////////////////////////////////////////////////

class MatchModulePart: public TARunObject
{
public:
   MatchFlags* fFlags = NULL;
   bool fTrace = false;
   //bool fTrace = true;
   int fCounter = 0;
   bool diagnostic = false;
   int DoThis=-1;
   int stride=-1;
   char ModuleName[50];
public:

   MatchModulePart(TARunInfo* runinfo, MatchFlags* f, int part, int _stride)
      : TARunObject(runinfo)
   {
      if (fTrace)
         printf("MatchModule::ctor!\n");
      DoThis=part;
      stride=_stride;
      sprintf(ModuleName,"match_module (part %d/%d)",DoThis,stride);
      fFlags = f;
      diagnostic=fFlags->fDiag; // dis/en-able histogramming
   }

   ~MatchModulePart()
   {
      if(fTrace)
         printf("MatchModule::dtor!\n");
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
      if( ! SigFlow->pdSig ) return flow;
            fCounter = 0;
      AgMatchFlow* MatchFlow= flow->Find<AgMatchFlow>();
      if (! MatchFlow) return flow;
      if (! MatchFlow->match) return flow;
      
      int combsize=MatchFlow->comb.size();
      //std::cout<<"COMBSIZE:"<<combsize<<std::endl;
      if (!combsize) return flow;
      if (DoThis>combsize-1) return flow;
      int combsize_inner=MatchFlow->comb.at(DoThis).size();
      //std::cout<<"COMBSIZE :"<<combsize_inner<<std::endl;
      MatchFlow->match->CombinePads(&MatchFlow->comb,DoThis,stride);
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,ModuleName,timer_start);
      #endif
      return flow;
  }
};

////////////////////////////////////////////////////////////////////////

class MatchModuleFin: public TARunObject
{
public:
   MatchFlags* fFlags = NULL;
   bool fTrace = false;
   //bool fTrace = true;
   int fCounter = 0;
   bool diagnostic = false;

public:

   MatchModuleFin(TARunInfo* runinfo, MatchFlags* f)
      : TARunObject(runinfo)
   {
      if (fTrace)
         printf("MatchModule::ctor!\n");

      fFlags = f;
      diagnostic=fFlags->fDiag; // dis/en-able histogramming
   }

   ~MatchModuleFin()
   {
      if(fTrace)
         printf("MatchModule::dtor!\n");
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
            fCounter = 0;
      AgMatchFlow* MatchFlow= flow->Find<AgMatchFlow>();
      if (! MatchFlow) return flow;
      if (! MatchFlow->match) return flow;
      // allow events without pwbs
      if( MatchFlow->match->GetCombinedPads() )
         {
            printf("MatchModule::Analyze, combined pads # %d\n", int(MatchFlow->match->GetCombinedPads()->size()));
            SigFlow->DeletePadSignals(); //Replace pad signals with combined ones
            SigFlow->AddPadSignals(MatchFlow->match->GetCombinedPads());
            MatchFlow->match->MatchElectrodes( SigFlow->awSig );
            MatchFlow->match->CombPoints();
         }
      else // <-- this probably goes before, where there are no pad signals -- AC 2019-6-3
         {
            printf("MatchModule::Analyze, NO combined pads, Set Z=0\n");
//delete match->GetCombinedPads();?
            MatchFlow->match->FakePads( SigFlow->awSig );
         }

      if( MatchFlow->match->GetSpacePoints() )
         printf("MatchModule::Analyze, Spacepoints # %d\n", int(MatchFlow->match->GetSpacePoints()->size()));
      else
         printf("MatchModule::Analyze Spacepoints should exists at this point\n");
      if( MatchFlow->match->GetSpacePoints()->size() > 0 )
         SigFlow->AddMatchSignals( MatchFlow->match->GetSpacePoints() );

      ++fCounter;
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"match_module_AW",timer_start);
      #endif
      return flow;
   }
};


class MatchModuleFactoryInit: public TAFactory
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
      return new MatchModuleInit(runinfo, &fFlags);
   }
};
class MatchModuleFactoryComb: public TAFactory
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
   }

   void Finish()
   {
      printf("MatchModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("MatchModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new MatchModuleComb(runinfo, &fFlags);
   }
};
class MatchModuleFactoryPart: public TAFactory
{
public:
   MatchFlags fFlags;
   int part=-1;
   int stride=-1;
public:
   MatchModuleFactoryPart(int _part, int _stride)
   {
      part=_part;
      stride=_stride;
      //Stride must be greater than part, otherwise you will analyze the same data twice
      assert(part<stride);
   }
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
   }

   void Finish()
   {
      printf("MatchModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("MatchModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new MatchModulePart(runinfo, &fFlags, part,stride);
   }
};
class MatchModuleFactoryFin: public TAFactory
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
   }

   void Finish()
   {
      printf("MatchModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("MatchModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new MatchModuleFin(runinfo, &fFlags);
   }
};

static TARegister tar1(new MatchModuleFactoryInit);
static TARegister tar2(new MatchModuleFactoryComb);
static TARegister tar3a(new MatchModuleFactoryPart(0,1));
//static TARegister tar3b(new MatchModuleFactoryPart(1,5));
//static TARegister tar3c(new MatchModuleFactoryPart(2,5));
//static TARegister tar3d(new MatchModuleFactoryPart(3,5));
//static TARegister tar3e(new MatchModuleFactoryPart(4,5));
static TARegister tar4(new MatchModuleFactoryFin);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
