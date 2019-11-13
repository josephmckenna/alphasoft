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

#define VF48_COINCTIME 0.000010



#include "TStripPed.h"
TStripPed Strip_ADCs[NUM_SI_MODULES*4*128];

class PedFlags
{
public:
   static bool fPrint;
   static bool fUnpackOff;
   static bool fHitOff;
   static int VF48commonThreshold;
   static bool ForceStripsFile;
   static TString CustomStripsFile;
   static double NSIGMATHRES;
   int ProcessVF48=-1;
};
bool    PedFlags::fPrint = false;
bool    PedFlags::fUnpackOff = false;
bool    PedFlags::fHitOff = false;
int     PedFlags::VF48commonThreshold = false;
bool    PedFlags::ForceStripsFile = false;
TString PedFlags::CustomStripsFile="";
double  PedFlags::NSIGMATHRES=3.;



class PedModule: public TARunObject
{
public:
   PedFlags* fFlags = NULL;
   bool fTrace = false;

   PedModule(TARunInfo* runinfo, PedFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("PedModule::ctor!\n");
   }

   ~PedModule()
   {
      if (fTrace)
         printf("PedModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("PedModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void PreEndRun(TARunInfo* runinfo, std::deque<TAFlowEvent*>* flow_queue)
   {
      if (fTrace)
         printf("PedModule::PreEndRun, run %d\n", runinfo->fRunNo);
      //time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      //printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));
   }



   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("PedModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

};

class PedModule_vf48: public TARunObject
{
public:
   PedFlags* fFlags = NULL;
   TString modulename;
   TSettings *SettingsDB = NULL;
   TVF48SiMap *gVF48SiMap = NULL;
   
   //Once instance per VF48 module
   int strip_ped_VF48Samples;
   double strip_ped_SubSample;
   int strip_ped_Offset;
   int strip_ped_SOffset;

   int strip_ped_SiModNumber[48];
   int strip_ped_ASIC[48];
   int strip_ped_FRCNumber[48];
   int strip_ped_FRCPort[48];
   int strip_ped_TTCChannel[48];

   PedModule_vf48(TARunInfo* runinfo, PedFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
      modulename="ped_module_vf48(";
      modulename+=fFlags->ProcessVF48;
      modulename+=")";
      
      // load the sqlite3 db
      char dbName[255]; 
      sprintf(dbName,"%s/a2lib/main.db",getenv("AGRELEASE"));
      SettingsDB = new TSettings(dbName,runinfo->fRunNo);      
      const int m=fFlags->ProcessVF48;
      {
         // extract VF48 sampling parameters from sqlite db
         strip_ped_VF48Samples = SettingsDB->GetVF48Samples(  m);
         strip_ped_SubSample = SettingsDB->GetVF48subsample( m );
         strip_ped_Offset = SettingsDB->GetVF48offset( m );
         strip_ped_SOffset = SettingsDB->GetVF48soffset(  m );
         if( strip_ped_SubSample < 1. || strip_ped_Offset < 0. || strip_ped_SOffset < 0. )
         {
            printf("PROBLEM: Unphysical VF48 sampling parameters:\n");
            printf("subsample = %f \t offset = %d \t soffset = %d \n", strip_ped_SubSample, strip_ped_Offset, strip_ped_SOffset);
            exit(0);
         }
      }

      char name[200];
      sprintf(name,"%s%s%s",
         getenv("AGRELEASE"),
         SettingsDB->GetVF48MapDir().Data(),
         SettingsDB->GetVF48Map(runinfo->fRunNo).Data());
      printf("name: %s\n",name);
      gVF48SiMap = new TVF48SiMap(name);
      for( int n=0; n<48; n++ )
         gVF48SiMap->GetSil( m, n, strip_ped_SiModNumber[n], strip_ped_ASIC[n], strip_ped_FRCNumber[n], strip_ped_FRCPort[n], strip_ped_TTCChannel[n] );

   }
   ~PedModule_vf48()
   {
      delete SettingsDB;
      delete gVF48SiMap;
   }

   void CountVF48Module(VF48event* e,const int vf48modnum)
   {
      TSiliconVA SiliconVA;
      //for( int vf48modnum=0; vf48modnum<NUM_VF48_MODULES; vf48modnum++ )
      {
         // Get the VF48 module
         VF48module* the_Module = e->modules[vf48modnum];
         if( !the_Module ) return;
         // identify number of samples/strip runs
         //VF48channel channel = the_Module->channels[0];

         //if (fFlags->fPrint)
         //   printf("HandleVF48Event: SQLLITE: Module %d \t subsample = %f \t offset = %d \t soffset = %d \t samples = %d \n", vf48modnum, subsample, offset, soffset, gVF48Samples[vf48modnum] );

         int vf48group=-1;
         for( int vf48chan=0; vf48chan<48; vf48chan++ )
         {
            if( vf48chan%16==0 )
               vf48group++;
            if( strip_ped_SiModNumber[vf48chan] < 0 )
               continue;
            // Get the VF48 channel
            if( vf48chan >= MAX_CHANNELS )
            {
               printf("Exceeded MAX_CHANNELS\n");
               exit(1);
            }
            VF48channel* the_Channel = &the_Module->channels[vf48chan];
            int numSamples=the_Channel->numSamples;
            //TSiliconVA* SiliconVA = new TSiliconVA( strip_ped_ASIC[vf48modnum][vf48chan], vf48chan );
            SiliconVA.Reset();
            if(vf48chan%4==2 || vf48chan%4==3)
               SiliconVA.SetPSide( true );

            int s = strip_ped_SOffset;
            // Determine the raw ADC for each strip by subsampling the VF48 samples
            for( int k=0; k<128; k++)
            {
               if( s >= numSamples ) continue;
               //SiliconVA->RawADC[k]=the_Channel->samples[s];
               SiliconVA.AddStrip(k,the_Channel->samples[s],-999.);
               s += (int)strip_ped_SubSample;
            }

            if( SiliconVA.GetNoStrips() != 128 )
            {
               continue;
            }
            // Calculate the ASIC strip mean/rms
            SiliconVA.CalcRawADCMeanSigma();

            // Calculate the filtered ASIC strip mean by removing hit-like strips
            SiliconVA.CalcFilteredADCMean();

            // Subtract the mean (filted) ASIC strip value from each strip (pedestal subtraction)
            SiliconVA.CalcPedSubADCs_NoFit();

            //double filt=90;
            //SiliconVA->CalcPedSubADCs_LowPassFilter(filt);
            const int firstStrip = 128*(strip_ped_ASIC[vf48chan]-1) + 512*(strip_ped_SiModNumber[vf48chan]);
            for (int k=0; k<128; k++)
            {
               Strip_ADCs[firstStrip + k].InsertValue(SiliconVA.PedSubADC[k],SiliconVA.RawADC[k]);
            }
         }//loop over VF48 channels
      } // loop over VF48 modules
      //delete SiliconVA;
      return;
   }

   
    TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      if (fFlags->fUnpackOff)
         return flow;

      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif

      VF48EventFlow* fe=flow->Find<VF48EventFlow>();
      if (!fe)
         return flow;
      if (!fe->vf48event)
         return flow;
      CountVF48Module(fe->vf48event,fFlags->ProcessVF48);

      //flow=new SilEventsFlow(flow,s);
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,modulename.Data(),timer_start);
      #endif
      return flow;
   }
   
   void EndRun(TARunInfo* runinfo)
   {
      if (fFlags->fPrint)
         printf("PedModule::EndRun, run %d\n", runinfo->fRunNo);
      if (fFlags->ProcessVF48!=0) return;
      // create extra root file
      char filename[80]; 
      sprintf(filename,"alphaStrips%05doffline.root", /*dir,*/ runinfo->fRunNo);
      TFile* file = new TFile(filename,"RECREATE");

      Int_t stripNumber=0;
      Float_t stripMean;
      Float_t stripRMS;
      Float_t stripRMSAfterFilter;
      Float_t stripMeanSubRMS;

      TTree* alphaStripTree = new TTree("alphaStrip Tree","alphaStrip Tree");
      alphaStripTree->Branch("stripNumber",&stripNumber, "stripNumber/I");
      alphaStripTree->Branch("stripMean",&stripMean, "stripMean/F");
      alphaStripTree->Branch("stripRMS",&stripRMS, "stripRMS/F");
      alphaStripTree->Branch("stripRMSAfterFilter",&stripRMSAfterFilter, "stripRMSAfterFilter/F");
      alphaStripTree->Branch("stripMeanSubRMS",&stripMeanSubRMS, "stripMeanSubRMS/F");

      for (int i=0; i<NUM_SI_MODULES*4*128; i++)
      {
         Strip_ADCs[i].sigma=fFlags->NSIGMATHRES;
         Strip_ADCs[i].CalculatePed();
         stripMean      =(float)Strip_ADCs[i].stripMean;
         stripRMS       =(float)Strip_ADCs[i].stripRMS;
         stripRMSAfterFilter=(float)Strip_ADCs[i].StripRMSsAfterFilter;
         stripMeanSubRMS=(float)Strip_ADCs[i].stripMeanSubRMS;
         
         alphaStripTree->Fill();
         stripNumber++;
      }
      file->Write();
      file->Close();
      delete file;
   }
};


class PedModuleFactory_vf48_0: public TAFactory
{
public:
   PedFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=0;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new PedModule_vf48(runinfo, &fFlags);
   }
};
class PedModuleFactory_vf48_1: public TAFactory
{
public:
   PedFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=1;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new PedModule_vf48(runinfo, &fFlags);
   }
};
class PedModuleFactory_vf48_2: public TAFactory
{
public:
   PedFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=2;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new PedModule_vf48(runinfo, &fFlags);
   }

};
class PedModuleFactory_vf48_3: public TAFactory
{
public:
   PedFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=3;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new PedModule_vf48(runinfo, &fFlags);
   }

};
class PedModuleFactory_vf48_4: public TAFactory
{
public:
   PedFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=4;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new PedModule_vf48(runinfo, &fFlags);
   }

};
class PedModuleFactory_vf48_5: public TAFactory
{
public:
   PedFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=5;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new PedModule_vf48(runinfo, &fFlags);
   }

};
class PedModuleFactory_vf48_6: public TAFactory
{
public:
   PedFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=6;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new PedModule_vf48(runinfo, &fFlags);
   }

};
class PedModuleFactory_vf48_7: public TAFactory
{
public:
   PedFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=7;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new PedModule_vf48(runinfo, &fFlags);
   }
};
   
class PedModuleFactory: public TAFactory
{
public:
   PedFlags fFlags;

public:
   void Help()
   {
      printf("PedModuleFactory::Help!\n");
      printf("\t--nounpack   Turn unpacking of TPC data (turn off reconstruction completely)\n");
   }
   void Usage()
   {
     Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("PedModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++)
      {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--nounpack")
            fFlags.fUnpackOff = true;
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("PedModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("PedModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new PedModule(runinfo, &fFlags);
   }
};

static TARegister tar0(new PedModuleFactory_vf48_0);
static TARegister tar1(new PedModuleFactory_vf48_1);
static TARegister tar2(new PedModuleFactory_vf48_2);
static TARegister tar3(new PedModuleFactory_vf48_3);
static TARegister tar4(new PedModuleFactory_vf48_4);
static TARegister tar5(new PedModuleFactory_vf48_5);
static TARegister tar6(new PedModuleFactory_vf48_6);
static TARegister tar7(new PedModuleFactory_vf48_7);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
