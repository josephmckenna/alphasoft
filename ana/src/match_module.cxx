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

   int ThreadID=-1;
   int TotalThreads=0;
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

      if (fTrace)
        printf("MatchModule::ctor!\n");

      fFlags = f;

      //First thread
      if (fFlags->ThreadID < 0)
        ModuleName="Match Module (CombPads)";
      //Multithreaded fitting
      else if ( fFlags->ThreadID < fFlags->TotalThreads ) 
        ModuleName="Match Module (Combine " +
          std::to_string(fFlags->ThreadID) + 
          "/" +
          std::to_string(fFlags->TotalThreads) +
          ")";
      else if (fFlags->TotalThreads==0 && fFlags->ThreadID==1)
        ModuleName="Match Module (spacepoints)";

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
      match->Init();
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
     
      if( SigFlow->pdSig )
        {
            //I am the first thread... 
            if (fFlags->ThreadID < 0)
            {
               SigFlow->comb = match->CombPads( SigFlow->pdSig );
               return flow;
            }
            else if ( fFlags->ThreadID < fFlags->TotalThreads )  //else process comb in multiple threads
            {
               size_t start=floor(SigFlow->comb.size() * fFlags->ThreadID/fFlags->TotalThreads);
               size_t stop=floor( SigFlow->comb.size() * (fFlags->ThreadID + 1) / fFlags->TotalThreads);

               //std::cout<<"SIZE:"<<SigFlow->comb.size()<<"\t"<<start<<" - "<<stop<<std::endl;
               for (size_t i=start; i<stop; i++)
                  SigFlow->combinedPads = match->CombineAPad( &SigFlow->comb,SigFlow->combinedPads,i );
               return flow;
            }
        }
      
      //Ending thread
      if (fFlags->TotalThreads==0 && fFlags->ThreadID==1)
      {
         // allow events without pwbs
         std::vector< std::pair<signal,signal> >* spacepoints = NULL;
         if( SigFlow->combinedPads )
            {
               if( fTrace )
                  printf("MatchModule::Analyze, combined pads # %d\n", int(SigFlow->combinedPads->size()));
               SigFlow->DeletePadSignals(); //Replace pad signals with combined ones
               SigFlow->AddPadSignals( SigFlow->combinedPads );
               spacepoints =
                  match->MatchElectrodes( SigFlow->awSig,SigFlow->combinedPads );
               spacepoints = match->CombPoints(spacepoints);
            }
         else // <-- this probably goes before, where there are no pad signals -- AC 2019-6-3
            {
               printf("MatchModule::Analyze, NO combined pads, Set Z=0\n");
   //delete match->GetCombinedPads();?
               spacepoints = match->FakePads( SigFlow->awSig );
            }

         if( spacepoints )
            printf("MatchModule::Analyze, Spacepoints # %d\n", int(spacepoints->size()));
         else
            printf("MatchModule::Analyze Spacepoints should exists at this point\n");
         if( spacepoints->size() > 0 )
            SigFlow->AddMatchSignals( spacepoints );

         //++fCounter;
         return flow;
      }
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
    MatchModuleFactory()
    {
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


class MatchModuleFactory_CombineAPad: public MatchModuleFactory
{
public:
   MatchModuleFactory_CombineAPad(int ThreadIndex, int NThreads)
   {
       fFlags.ThreadID=ThreadIndex;
       fFlags.TotalThreads=NThreads;
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new MatchModule(runinfo, &fFlags);
   }
};
class MatchModuleFactoryFinish: public MatchModuleFactory
{
public:
   MatchModuleFactoryFinish()
   {
       fFlags.ThreadID=1;
       fFlags.TotalThreads=0;
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new MatchModule(runinfo, &fFlags);
   }
};


static TARegister tar(new MatchModuleFactory);
//Choose how many threads you want here (2,4,8,16, 32 or 64)... more threads need more ram
#define MAX_THREADS 32
static TARegister tar1(new MatchModuleFactory_CombineAPad(0,MAX_THREADS));
static TARegister tar2(new MatchModuleFactory_CombineAPad(1,MAX_THREADS));
#if MAX_THREADS>2
static TARegister tar3(new MatchModuleFactory_CombineAPad(2,MAX_THREADS));
static TARegister tar4(new MatchModuleFactory_CombineAPad(3,MAX_THREADS));
#endif
#if MAX_THREADS>4
static TARegister tar5(new MatchModuleFactory_CombineAPad(4,MAX_THREADS));
static TARegister tar6(new MatchModuleFactory_CombineAPad(5,MAX_THREADS));
static TARegister tar7(new MatchModuleFactory_CombineAPad(6,MAX_THREADS));
static TARegister tar8(new MatchModuleFactory_CombineAPad(7,MAX_THREADS));
#endif
#if MAX_THREADS>8
static TARegister tar9(new MatchModuleFactory_CombineAPad(8,MAX_THREADS));
static TARegister tar10(new MatchModuleFactory_CombineAPad(9,MAX_THREADS));
static TARegister tar11(new MatchModuleFactory_CombineAPad(10,MAX_THREADS));
static TARegister tar12(new MatchModuleFactory_CombineAPad(11,MAX_THREADS));
static TARegister tar13(new MatchModuleFactory_CombineAPad(12,MAX_THREADS));
static TARegister tar14(new MatchModuleFactory_CombineAPad(13,MAX_THREADS));
static TARegister tar15(new MatchModuleFactory_CombineAPad(14,MAX_THREADS));
static TARegister tar16(new MatchModuleFactory_CombineAPad(15,MAX_THREADS));
#endif
#if MAX_THREADS>16
static TARegister tar17(new MatchModuleFactory_CombineAPad(16,MAX_THREADS));
static TARegister tar18(new MatchModuleFactory_CombineAPad(17,MAX_THREADS));
static TARegister tar19(new MatchModuleFactory_CombineAPad(18,MAX_THREADS));
static TARegister tar20(new MatchModuleFactory_CombineAPad(19,MAX_THREADS));
static TARegister tar21(new MatchModuleFactory_CombineAPad(20,MAX_THREADS));
static TARegister tar22(new MatchModuleFactory_CombineAPad(21,MAX_THREADS));
static TARegister tar23(new MatchModuleFactory_CombineAPad(22,MAX_THREADS));
static TARegister tar24(new MatchModuleFactory_CombineAPad(23,MAX_THREADS));
static TARegister tar25(new MatchModuleFactory_CombineAPad(24,MAX_THREADS));
static TARegister tar26(new MatchModuleFactory_CombineAPad(25,MAX_THREADS));
static TARegister tar27(new MatchModuleFactory_CombineAPad(26,MAX_THREADS));
static TARegister tar28(new MatchModuleFactory_CombineAPad(27,MAX_THREADS));
static TARegister tar29(new MatchModuleFactory_CombineAPad(28,MAX_THREADS));
static TARegister tar30(new MatchModuleFactory_CombineAPad(29,MAX_THREADS));
static TARegister tar31(new MatchModuleFactory_CombineAPad(30,MAX_THREADS));
static TARegister tar32(new MatchModuleFactory_CombineAPad(31,MAX_THREADS));
#endif
#if MAX_THREADS>32
static TARegister tar33(new MatchModuleFactory_CombineAPad(32,MAX_THREADS));
static TARegister tar34(new MatchModuleFactory_CombineAPad(33,MAX_THREADS));
static TARegister tar35(new MatchModuleFactory_CombineAPad(34,MAX_THREADS));
static TARegister tar36(new MatchModuleFactory_CombineAPad(35,MAX_THREADS));
static TARegister tar37(new MatchModuleFactory_CombineAPad(36,MAX_THREADS));
static TARegister tar38(new MatchModuleFactory_CombineAPad(37,MAX_THREADS));
static TARegister tar39(new MatchModuleFactory_CombineAPad(38,MAX_THREADS));
static TARegister tar40(new MatchModuleFactory_CombineAPad(39,MAX_THREADS));
static TARegister tar41(new MatchModuleFactory_CombineAPad(40,MAX_THREADS));
static TARegister tar42(new MatchModuleFactory_CombineAPad(41,MAX_THREADS));
static TARegister tar43(new MatchModuleFactory_CombineAPad(42,MAX_THREADS));
static TARegister tar44(new MatchModuleFactory_CombineAPad(43,MAX_THREADS));
static TARegister tar45(new MatchModuleFactory_CombineAPad(44,MAX_THREADS));
static TARegister tar46(new MatchModuleFactory_CombineAPad(45,MAX_THREADS));
static TARegister tar47(new MatchModuleFactory_CombineAPad(46,MAX_THREADS));
static TARegister tar48(new MatchModuleFactory_CombineAPad(47,MAX_THREADS));
static TARegister tar49(new MatchModuleFactory_CombineAPad(48,MAX_THREADS));
static TARegister tar50(new MatchModuleFactory_CombineAPad(49,MAX_THREADS));
static TARegister tar51(new MatchModuleFactory_CombineAPad(50,MAX_THREADS));
static TARegister tar52(new MatchModuleFactory_CombineAPad(51,MAX_THREADS));
static TARegister tar53(new MatchModuleFactory_CombineAPad(52,MAX_THREADS));
static TARegister tar54(new MatchModuleFactory_CombineAPad(53,MAX_THREADS));
static TARegister tar55(new MatchModuleFactory_CombineAPad(54,MAX_THREADS));
static TARegister tar56(new MatchModuleFactory_CombineAPad(55,MAX_THREADS));
static TARegister tar57(new MatchModuleFactory_CombineAPad(56,MAX_THREADS));
static TARegister tar58(new MatchModuleFactory_CombineAPad(57,MAX_THREADS));
static TARegister tar59(new MatchModuleFactory_CombineAPad(58,MAX_THREADS));
static TARegister tar60(new MatchModuleFactory_CombineAPad(59,MAX_THREADS));
static TARegister tar61(new MatchModuleFactory_CombineAPad(60,MAX_THREADS));
static TARegister tar62(new MatchModuleFactory_CombineAPad(61,MAX_THREADS));
static TARegister tar63(new MatchModuleFactory_CombineAPad(62,MAX_THREADS));
static TARegister tar64(new MatchModuleFactory_CombineAPad(63,MAX_THREADS));
#endif
static TARegister tarend(new MatchModuleFactoryFinish);


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
