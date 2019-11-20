// HandleVF48.cxx

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

#include "TFile.h"
#include "TH2.h"
#include "TF1.h"
#include "TLatex.h"
#include "TText.h"
#include "TBox.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TTree.h"


#include "manalyzer.h"
#include "midasio.h"

#include "TSettings.h"

#include "SiMod.h"
#include "UnpackVF48.h"
#include "A2Flow.h"

#include "TVF48SiMap.h"
#include "TSystem.h"

#define MAX_CHANNELS VF48_MAX_CHANNELS // defined in UnpackVF48.h
#define NUM_SI_MODULES nSil // defined in SiMod.h
#define NUM_VF48_MODULES nVF48 // defined in SiMod.h
#define NUM_SI_ALPHA1 nSiAlpha1 // defined in SiMod.h
#define NUM_SI_ALPHA2 nSiAlpha2 // defined in SiMod.h



class ASICFlags
{
public:
   static bool fPrint;
   static bool fUnpackOff;
  
   int ProcessSilMod=-1;
   int ProcessStride=0;
   static double nVASigma;
   static double pVASigma;
   
};
bool    ASICFlags::fPrint = false;
bool    ASICFlags::fUnpackOff = false;
double  ASICFlags::nVASigma = 2.375;//3.125;
double  ASICFlags::pVASigma = 2.75;//3.75;


class ASICModule: public TARunObject
{
public:
   ASICFlags* fFlags = NULL;
   bool fTrace = false;
   TString modulename;
   const double nVASigma;
   const double pVASigma;
   ASICModule(TARunInfo* runinfo, ASICFlags* flags)
     : TARunObject(runinfo), fFlags(flags), nVASigma(fFlags->nVASigma), pVASigma(fFlags->pVASigma)
   {
      modulename="ASICModule(";
      modulename+=fFlags->ProcessSilMod;
      modulename+="-";
      modulename+=fFlags->ProcessSilMod+fFlags->ProcessStride;
      modulename+=")";
   }
   ~ASICModule()
   {
      if (fTrace)
         printf("HitModule::dtor!\n");

   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("HitModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void PreEndRun(TARunInfo* runinfo, std::deque<TAFlowEvent*>* flow_queue)
   {
      if (fTrace)
         printf("HitModule::PreEndRun, run %d\n", runinfo->fRunNo);
      //time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      //printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("HitModule::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("HitModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }
   

   TSiliconEvent* CalcASICFilter(const int simodnum, const int mods, TSiliconEvent* SiliconEvent)
   {
      Double_t NSideRawHits=SiliconEvent->GetNsideNRawHits();
      Double_t PSideRawHits=SiliconEvent->GetPsideNRawHits();
      

      if (simodnum>NUM_SI_MODULES)
      {
          std::cout<<"Bad simodnum"<<std::endl;
          return SiliconEvent;
      }
      if (simodnum==NUM_SI_MODULES)
      {
         // == End construction of Silicon Event
         SiliconEvent->CompressSiliconModules();
         //SiliconEvent->Print();
         return SiliconEvent;
      }
      for (int i=simodnum; i<  simodnum+mods; i++)
      {
          TSiliconModule* SilMod=SiliconEvent->GetSiliconModule(i);
          //if (!SilMod) continue;
          //SilMod->Print();
          for (int j=1; j<=4; j++)
          {
             TSiliconVA* SiliconVA=SilMod->GetASIC(j);
             //if (!SiliconVA) continue;
             // Calculate the ASIC strip mean/rms
             SiliconVA->CalcRawADCMeanSigma();

             // Calculate the filtered ASIC strip mean by removing hit-like strips
             SiliconVA->CalcFilteredADCMean();

             // Subtract the mean (filted) ASIC strip value from each strip (pedestal subtraction)
             SiliconVA->CalcPedSubADCs_NoFit();
             //double a=90.;
             //SiliconVA->CalcPedSubADCs_LowPassFilter(a);
             if(SiliconVA->GetASICNumber()%4==2 || SiliconVA->GetASICNumber()%4==3)
             {
                PSideRawHits+=SiliconVA->CalcHits( pVASigma );
             }
             else
             {
                NSideRawHits+=SiliconVA->CalcHits( nVASigma );
             }
             //SiliconVA->Print();
             //SiliconVA->SuppressNoiseyStrips();


          }
          SilMod->CompressVAs();
      }
      SiliconEvent->SetPsideNRawHits( PSideRawHits );
      SiliconEvent->SetNsideNRawHits( NSideRawHits );
      return SiliconEvent;
   }



   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      if (fFlags->fUnpackOff)
         return flow;

      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif

      SilEventsFlow* sf=flow->Find<SilEventsFlow>();
      if (!sf)
         return flow;
      TSiliconEvent* SiliconEvent=sf->silevent;
      if (!SiliconEvent)
         return flow;
      SiliconEvent=CalcASICFilter(fFlags->ProcessSilMod, fFlags->ProcessStride, SiliconEvent);
      sf->silevent=SiliconEvent;
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,modulename.Data(),timer_start);
      #endif
      return flow;
   }
};



class ASICModuleFactory_mod: public TAFactory
{
public:
   ASICFlags fFlags;
   ASICModuleFactory_mod(int m, int s)
   {
      fFlags.ProcessSilMod=m*s;
      fFlags.ProcessStride=s;
   }

   void Init(const std::vector<std::string> &args)
   {
      printf("ASICModule(%d-%d)::Init!\n",fFlags.ProcessSilMod,fFlags.ProcessSilMod+fFlags.ProcessStride);
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new ASICModule(runinfo, &fFlags);
   }
};


static TARegister tar0(new ASICModuleFactory_mod(0,9));
static TARegister tar1(new ASICModuleFactory_mod(1,9));
static TARegister tar2(new ASICModuleFactory_mod(2,9));
static TARegister tar3(new ASICModuleFactory_mod(3,9));
static TARegister tar4(new ASICModuleFactory_mod(4,9));
static TARegister tar5(new ASICModuleFactory_mod(5,9));
static TARegister tar6(new ASICModuleFactory_mod(6,9));
static TARegister tar7(new ASICModuleFactory_mod(7,9));
static TARegister tar8(new ASICModuleFactory_mod(8,9));

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
