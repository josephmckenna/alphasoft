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


int gVF48Samples[NUM_VF48_MODULES];
double gSubSample[NUM_VF48_MODULES];
int gOffset[NUM_VF48_MODULES];
int gSOffset[NUM_VF48_MODULES];

   double StripRMSs[NUM_SI_MODULES*4*128];
   double StripMeans[NUM_SI_MODULES*4*128];
   double SumRMSs[NUM_SI_MODULES*4*128];
   
   double nVASigma = 2.375;//3.125;
   double pVASigma = 2.75;//3.75;


int SiModNumber[nVF48][48];
int ASIC[nVF48][48];
int FRCNumber[nVF48][48];
int FRCPort[nVF48][48];
int TTCChannel[nVF48][48];

class HitFlags
{
public:
   bool fPrint = false;
   bool fUnpackOff = false;
   bool fHitOff = false;
   int VF48commonThreshold = false;
   bool ForceStripsFile = false;
   TString CustomStripsFile;
   int ProcessVF48=-1;
   bool OldStripFileVariables=false;
};

class HitModule: public TARunObject
{
public:
   HitFlags* fFlags = NULL;
   bool fTrace = false;

   TVF48SiMap *gVF48SiMap = NULL;
   HitModule(TARunInfo* runinfo, HitFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("HitModule::ctor!\n");

      // load the sqlite3 db
      char dbName[255]; 
      sprintf(dbName,"%s/a2lib/main.db",getenv("AGRELEASE"));
      TSettings *SettingsDB = new TSettings(dbName,runinfo->fRunNo);      
      for (int m=0; m<NUM_VF48_MODULES; m++)
      {
         // extract VF48 sampling parameters from sqlite db
         gVF48Samples[m] = SettingsDB->GetVF48Samples( runinfo->fRunNo, m);
         gSubSample[m] = SettingsDB->GetVF48subsample( runinfo->fRunNo,m );
         gOffset[m] = SettingsDB->GetVF48offset( runinfo->fRunNo, m );
         gSOffset[m] = SettingsDB->GetVF48soffset( runinfo->fRunNo, m );
         if( gSubSample[m] < 1. || gOffset[m] < 0. || gSOffset[m] < 0. )
         {
            printf("PROBLEM: Unphysical VF48 sampling parameters:\n");
            printf("subsample = %f \t offset = %d \t soffset = %d \n", gSubSample[m], gOffset[m], gSOffset[m]);
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

      for( int n=0; n<NUM_VF48_MODULES; n++ )
         for( int m=0; m<48; m++ )
            gVF48SiMap->GetSil( n, m, SiModNumber[n][m], ASIC[n][m], FRCNumber[n][m], FRCPort[n][m], TTCChannel[n][m] );

      char striprms_filename[256];
      if ( fFlags->ForceStripsFile )
      {
         if (fFlags->CustomStripsFile[0]=='/') //Probably absolute path
            sprintf(striprms_filename,"%s",fFlags->CustomStripsFile.Data());
         else
            sprintf(striprms_filename,"%s/%s",getenv("A2DATAPATH"),fFlags->CustomStripsFile.Data());
      }
      else
      {
         int strips_run=runinfo->fRunNo;
         int age_limit=100;
         while (strips_run > runinfo->fRunNo - age_limit)
         {
            sprintf( striprms_filename, "%s/alphaStrips%doffline.root", getenv("A2DATAPATH"), strips_run );
            FileStat_t filestat_buf;
            if (gSystem->GetPathInfo(striprms_filename, filestat_buf) ) // file doesn't exist
               std::cout<<"Stripfile for "<<strips_run<<"  not found ("<<striprms_filename<<")"<<std::endl;
            else
               break;
            strips_run--;
         }
         if (strips_run < runinfo->fRunNo - age_limit +1)
         {
            std::cout<<"No strips file thats new enough found... aborting..."<<std::endl;
            exit(4);
         }
      }

      std::cout<<"Stripfile found: "<<striprms_filename<<std::endl;
      TFile * striprms_file = new TFile( striprms_filename, "READ" );
      TTree* striprms_tree = (TTree*) striprms_file->Get("alphaStrip Tree");
      Int_t stripNumber;
      Float_t stripRMS, stripMean;
      striprms_tree->SetBranchAddress("stripNumber", &stripNumber );
      if (fFlags->OldStripFileVariables)
         striprms_tree->SetBranchAddress("stripMeanSubRMS", &stripRMS );
      else
         striprms_tree->SetBranchAddress("stripRMS", &stripRMS );

      striprms_tree->SetBranchAddress("stripMean", &stripMean );
      Int_t BadRMSValues=0;
      for(Int_t i=0; i<(NUM_SI_MODULES*4*128); i++)
      {
         striprms_tree->GetEntry(i);
         assert(stripNumber < (NUM_SI_MODULES*4*128) );
         //RMSHisto->Fill( stripRMS );
         //StripRMSs[i] = fabs(stripRMS) < 200. ? stripRMS : 200.;  //This is a source of 'hot strips' bug...
         //StripMeans[i]= fabs(stripMean)<200? stripMean : 0.;    //Strips with high values of RMS or mean are set lower here... it results in the last strips of a ASIC often being 'noisey'
         if (stripRMS<0) BadRMSValues++;
         StripRMSs[i] = stripRMS;// fabs(stripRMS) < 200. ? stripRMS : 200.;
         StripMeans[i]= stripMean;//fabs(stripMean)<200? stripMean : 0.;
      }
      delete striprms_tree;
      delete striprms_file;

      delete SettingsDB;
   }

   ~HitModule()
   {
      if (fTrace)
         printf("HitModule::dtor!\n");
      delete gVF48SiMap;
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
   
   TSiliconEvent* BuildTSiliconEvent(VF48event* e)
   {
      TSiliconEvent* SiliconEvent = new TSiliconEvent();
      SiliconEvent->SetVF48NEvent(e->eventNo);
      SiliconEvent->SetVF48Timestamp(e->timestamp);
      SiliconEvent->SetVF48NTrigger(e->modules[1]->trigger);
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
      VF48EventFlow* fe=flow->Find<VF48EventFlow>();
      if (!fe)
         return flow;
      if (!fe->vf48event)
         return flow;
      TSiliconEvent* s=BuildTSiliconEvent(fe->vf48event);
      flow=new SilEventsFlow(flow,s);
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"hybrid_hits_module",timer_start);
      #endif
      return flow;
   }
};



TSiliconEvent* AddVF48Module(VF48event* e,const int vf48modnum, TSiliconEvent* SiliconEvent)
{

   TSiliconModule* SiliconModule = NULL;
   TSiliconVA* SiliconVA = NULL;
   //TSiliconStrip* SiliconStrip = NULL;

   Double_t NSideRawHits=SiliconEvent->GetNsideNRawHits();
   Double_t PSideRawHits=SiliconEvent->GetPsideNRawHits() ;
   
   //for( int vf48modnum=0; vf48modnum<NUM_VF48_MODULES; vf48modnum++ )
   {
      // Get the VF48 module
      VF48module* the_Module = e->modules[vf48modnum];
      if( !the_Module ) return SiliconEvent;
      // identify number of samples/strip runs
      //VF48channel channel = the_Module->channels[0];

      int subsample = (int)gSubSample[vf48modnum];
      int offset = gOffset[vf48modnum];
      int soffset = gSOffset[vf48modnum];

      //if (fFlags->fPrint)
      //   printf("HandleVF48Event: SQLLITE: Module %d \t subsample = %f \t offset = %d \t soffset = %d \t samples = %d \n", vf48modnum, subsample, offset, soffset, gVF48Samples[vf48modnum] );

      int vf48group=-1;
      for( int vf48chan=0; vf48chan<48; vf48chan++ )
      {
         if( vf48chan%16==0 )
            vf48group++;
         if( SiModNumber[vf48modnum][vf48chan] < 0 )
            continue;
         if( vf48chan%4 == 0 ) 
         {
            SiliconModule = new TSiliconModule( SiModNumber[vf48modnum][vf48chan], vf48modnum, vf48group, FRCNumber[vf48modnum][vf48chan], FRCPort[vf48modnum][vf48chan] );
         }
         // Get the VF48 channel
         if( vf48chan >= MAX_CHANNELS )
         {
            printf("Exceeded MAX_CHANNELS\n");
            exit(1);
         }
         VF48channel* the_Channel = &the_Module->channels[vf48chan];
         int numSamples=the_Channel->numSamples;
         SiliconVA = new TSiliconVA( ASIC[vf48modnum][vf48chan], vf48chan );
         if(vf48chan%4==2 || vf48chan%4==3)
            SiliconVA->SetPSide( true );
         if ( numSamples> 0)
         {
            // N.B.: if channel is suppressed numSamples = 0
            int s = soffset;
            int firststrip=128*(ASIC[vf48modnum][vf48chan]-1) + 512*(SiModNumber[vf48modnum][vf48chan]);
            for( int k=0; k<128; k++) // loop over the strips
            {
               if( s >= numSamples ) break;
               Int_t stripNumber = k +firststrip;
               Double_t stripMean= StripMeans[stripNumber];
               //SiliconStrip = new TSiliconStrip( k, the_Channel.samples[s+offset] - TMath::Nint(stripMean));
               //SiliconVA->AddStrip( SiliconStrip );
               SiliconVA->AddStrip(k,the_Channel->samples[s+offset] - TMath::Nint(stripMean),StripRMSs[stripNumber]);
               s += subsample;
            }
         }
         else 
         {
            for( int k=0; k<128; k++)
            {
               Int_t stripNumber = k + 128*(ASIC[vf48modnum][vf48chan]-1) + 512*(SiModNumber[vf48modnum][vf48chan]);
               SiliconVA->AddStrip(k,0,StripRMSs[stripNumber]);
               //SiliconStrip = new TSiliconStrip( k, 0);
               //SiliconVA->AddStrip( SiliconStrip );
            }
         }
         if( SiliconVA->NoStrips() )
         {
            delete SiliconVA;
            SiliconVA = NULL;
            continue;
         }
         // Calculate the ASIC strip mean/rms
         SiliconVA->CalcRawADCMeanSigma();

         // Calculate the filtered ASIC strip mean by removing hit-like strips
         SiliconVA->CalcFilteredADCMean();

         // Subtract the mean (filted) ASIC strip value from each strip (pedestal subtraction)
         SiliconVA->CalcPedSubADCs_NoFit();
         //double a=90.;
         //SiliconVA->CalcPedSubADCs_LowPassFilter(a);
         if(vf48chan%4==2 || vf48chan%4==3)
         {
            PSideRawHits+=SiliconVA->CalcHits( pVASigma, SiModNumber[vf48modnum][vf48chan] );
         }
         else
         {
            NSideRawHits+=SiliconVA->CalcHits( nVASigma, SiModNumber[vf48modnum][vf48chan] );
         }
         //SiliconVA->Print();
         //SiliconVA->SuppressNoiseyStrips();
         SiliconModule->AddASIC( SiliconVA );

         // when reached the last channel (asic) in the module, add silicon module
         if( vf48chan%4 == 3 )
         {
            SiliconEvent->AddSiliconModule( SiliconModule );
         }
      }//loop over VF48 channels
   } // loop over VF48 modules

   // == End construction of Silicon Event
   if (vf48modnum==NUM_VF48_MODULES-1)
   {   
      SiliconEvent->CompressSiliconModules();
   }
   SiliconEvent->SetPsideNRawHits( PSideRawHits );
   SiliconEvent->SetNsideNRawHits( NSideRawHits );

   //SiliconEvent->Print();
   return SiliconEvent;
}

class HitModule_vf48: public TARunObject
{
public:
   HitFlags* fFlags = NULL;
   TString modulename;
   HitModule_vf48(TARunInfo* runinfo, HitFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
      modulename="hybrid_hits_module_vf48(";
      modulename+=fFlags->ProcessVF48;
      modulename+=")";
   }
   ~HitModule_vf48(){ }
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
      
      SilEventsFlow* sf=flow->Find<SilEventsFlow>();
      if (!sf)
         return flow;
      TSiliconEvent* SiliconEvent=sf->silevent;
      if (!SiliconEvent)
         return flow;
      SiliconEvent=AddVF48Module(fe->vf48event,fFlags->ProcessVF48, SiliconEvent);
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,modulename.Data(),timer_start);
      #endif
      return flow;
   }
};

class HitModuleFactory_vf48_0: public TAFactory
{
public:
   HitFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=0;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new HitModule_vf48(runinfo, &fFlags);
   }
};
class HitModuleFactory_vf48_1: public TAFactory
{
public:
   HitFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=1;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new HitModule_vf48(runinfo, &fFlags);
   }
};
class HitModuleFactory_vf48_2: public TAFactory
{
public:
   HitFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=2;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new HitModule_vf48(runinfo, &fFlags);
   }
};
class HitModuleFactory_vf48_3: public TAFactory
{
public:
   HitFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=3;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new HitModule_vf48(runinfo, &fFlags);
   }
};
class HitModuleFactory_vf48_4: public TAFactory
{
public:
   HitFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=4;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new HitModule_vf48(runinfo, &fFlags);
   }
};
class HitModuleFactory_vf48_5: public TAFactory
{
public:
   HitFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=5;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new HitModule_vf48(runinfo, &fFlags);
   }
};
class HitModuleFactory_vf48_6: public TAFactory
{
public:
   HitFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=6;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new HitModule_vf48(runinfo, &fFlags);
   }
};
class HitModuleFactory_vf48_7: public TAFactory
{
public:
   HitFlags fFlags;
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      fFlags.ProcessVF48=7;
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new HitModule_vf48(runinfo, &fFlags);
   }
};

   
class HitModuleFactory: public TAFactory
{
public:
   HitFlags fFlags;

public:
   void Help()
   {
      printf("HitModuleFactory::Help!\n");
      printf("\t--nounpack   Turn unpacking of TPC data (turn off reconstruction completely)\n");
      printf("\t--stripsfile filename.root \t Force custom stripsfile\n");
      printf("\t--oldstripsfile \t Use strips file in the old way (old variables)\n");
   }
   void Usage()
   {
     Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("HitModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++)
      {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--nounpack")
            fFlags.fUnpackOff = true;
         if (args[i] == "--stripsfile")
         {
            fFlags.ForceStripsFile = true;
            fFlags.CustomStripsFile= args[i+1];
            i++;
         }
         if (args[i] == "--oldstripsfile")
            fFlags.OldStripFileVariables = true;
      }
   }

   void Finish()
   {
      printf("HitModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("HitModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new HitModule(runinfo, &fFlags);
   }
};

static TARegister tar(new HitModuleFactory);
static TARegister tar0(new HitModuleFactory_vf48_0);
static TARegister tar1(new HitModuleFactory_vf48_1);
static TARegister tar2(new HitModuleFactory_vf48_2);
static TARegister tar3(new HitModuleFactory_vf48_3);
static TARegister tar4(new HitModuleFactory_vf48_4);
static TARegister tar5(new HitModuleFactory_vf48_5);
static TARegister tar6(new HitModuleFactory_vf48_6);
static TARegister tar7(new HitModuleFactory_vf48_7);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
