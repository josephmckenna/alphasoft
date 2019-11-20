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





class HitFlags
{
public:
   static bool fPrint;
   static bool fUnpackOff;
   static bool fHitOff;
   static int VF48commonThreshold;
   static bool ForceStripsFile;
   static TString CustomStripsFile;
   int ProcessVF48=-1; //Cannot be static! Unique per thread inside this module
   static bool OldStripFileVariables;
   static double nVASigma;
   static double pVASigma;
   
   static double fStripRMSs[NUM_SI_MODULES*4*128];
   static double fStripMeans[NUM_SI_MODULES*4*128];
   void LoadAllStrips(const int RunNo)
   {
      char striprms_filename[256];
      if ( this->ForceStripsFile )
      {
         if (this->CustomStripsFile[0]=='/') //Probably absolute path
            sprintf(striprms_filename,"%s",this->CustomStripsFile.Data());
         else
            sprintf(striprms_filename,"%s/%s",getenv("A2DATAPATH"),this->CustomStripsFile.Data());
      }
      else
      {
         int strips_run=RunNo;
         int age_limit=100;
         while (strips_run > RunNo - age_limit)
         {
            sprintf( striprms_filename, "%s/alphaStrips%doffline.root", getenv("A2DATAPATH"), strips_run );
            FileStat_t filestat_buf;
            if (gSystem->GetPathInfo(striprms_filename, filestat_buf) ) // file doesn't exist
               std::cout<<"Stripfile for "<<strips_run<<"  not found ("<<striprms_filename<<")"<<std::endl;
            else
               break;
            strips_run--;
         }
         if (strips_run < RunNo - age_limit +1)
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
      if (this->OldStripFileVariables)
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
         fStripRMSs[i] = stripRMS;// fabs(stripRMS) < 200. ? stripRMS : 200.;
         fStripMeans[i]= stripMean;//fabs(stripMean)<200? stripMean : 0.;
      }
      
      delete striprms_tree;
      delete striprms_file;
   }
};
bool    HitFlags::fPrint = false;
bool    HitFlags::fUnpackOff = false;
bool    HitFlags::fHitOff = false;
int     HitFlags::VF48commonThreshold = false;
bool    HitFlags::ForceStripsFile = false;
TString HitFlags::CustomStripsFile="";
bool    HitFlags::OldStripFileVariables=false;
double  HitFlags::nVASigma = 2.375;//3.125;
double  HitFlags::pVASigma = 2.75;//3.75;
double  HitFlags::fStripMeans[NUM_SI_MODULES*4*128]={0};
double  HitFlags::fStripRMSs[NUM_SI_MODULES*4*128]={0};

class HitModule: public TARunObject
{
public:
   HitFlags* fFlags = NULL;
   bool fTrace = false;

   HitModule(TARunInfo* runinfo, HitFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("HitModule::ctor!\n");
   }

   ~HitModule()
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
      START_TIMER
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

class HitModule_vf48: public TARunObject
{
public:
   HitFlags* fFlags = NULL;
   TString modulename;
   TSettings *SettingsDB = NULL;
   TVF48SiMap *gVF48SiMap = NULL;

   double BadRMSValues=0;

   //Once instance per VF48 module
   int VF48Samples;
   double SubSample;
   int Offset;
   int SOffset;

   int SiModNumber[48];
   int ASIC[48];
   int FRCNumber[48];
   int FRCPort[48];
   int TTCChannel[48];
   
   
   double StripRMSs[48*128];
   double StripMeans[48*128];
   
   const double nVASigma;
   const double pVASigma;

   HitModule_vf48(TARunInfo* runinfo, HitFlags* flags)
     : TARunObject(runinfo), fFlags(flags), nVASigma(fFlags->nVASigma), pVASigma(fFlags->pVASigma)
   {
      modulename="hybrid_hits_module_vf48(";
      modulename+=fFlags->ProcessVF48;
      modulename+=")";

      // load the sqlite3 db
      char dbName[255]; 
      sprintf(dbName,"%s/a2lib/main.db",getenv("AGRELEASE"));
      SettingsDB = new TSettings(dbName,runinfo->fRunNo);      
      const int m=fFlags->ProcessVF48;
      {
         // extract VF48 sampling parameters from sqlite db
         VF48Samples = SettingsDB->GetVF48Samples(  m);
         SubSample = SettingsDB->GetVF48subsample( m );
         Offset = SettingsDB->GetVF48offset( m );
         SOffset = SettingsDB->GetVF48soffset(  m );
         if( SubSample < 1. || Offset < 0. || SOffset < 0. )
         {
            printf("PROBLEM: Unphysical VF48 sampling parameters:\n");
            printf("subsample = %f \t offset = %d \t soffset = %d \n", SubSample, Offset, SOffset);
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
         gVF48SiMap->GetSil( m, n, SiModNumber[n], ASIC[n], FRCNumber[n], FRCPort[n], TTCChannel[n] );

      int StripCount=0;
      for( int vf48chan=0; vf48chan<48; vf48chan++ )
      {
         if( SiModNumber[vf48chan] < 0 )
            continue;
         const int firststrip=128*(ASIC[vf48chan]-1) + 512*(SiModNumber[vf48chan]);
         for( int k=0; k<128; k++) // loop over the strips
         {
            int i=firststrip+k;
            double stripRMS=fFlags->fStripRMSs[i];
            double stripMean=fFlags->fStripMeans[i];
            assert(i < (NUM_SI_MODULES*4*128) );
            //RMSHisto->Fill( stripRMS );
            //StripRMSs[i] = fabs(stripRMS) < 200. ? stripRMS : 200.;  //This is a source of 'hot strips' bug...
            //StripMeans[i]= fabs(stripMean)<200? stripMean : 0.;    //Strips with high values of RMS or mean are set lower here... it results in the last strips of a ASIC often being 'noisey'
            if (stripRMS<0) BadRMSValues++;
            StripRMSs[StripCount] = stripRMS;// fabs(stripRMS) < 200. ? stripRMS : 200.;
            StripMeans[StripCount]= stripMean;//fabs(stripMean)<200? stripMean : 0.;
            ++StripCount;
         }
      }
   }
   ~HitModule_vf48(){ 
      delete SettingsDB;
      delete gVF48SiMap;
   }
   
   TSiliconEvent* AddVF48Module(VF48event* e,const int vf48modnum, TSiliconEvent* SiliconEvent)
   {

      TSiliconModule* SiliconModule = NULL;
      TSiliconVA* SiliconVA = NULL;
      //TSiliconStrip* SiliconStrip = NULL;

      Double_t NSideRawHits=SiliconEvent->GetNsideNRawHits();
      Double_t PSideRawHits=SiliconEvent->GetPsideNRawHits() ;
      
      int StripCount=0;
      //for( int vf48modnum=0; vf48modnum<NUM_VF48_MODULES; vf48modnum++ )
      {
         // Get the VF48 module
         VF48module* the_Module = e->modules[vf48modnum];
         if( !the_Module ) return SiliconEvent;
         // identify number of samples/strip runs
         //VF48channel channel = the_Module->channels[0];

         int subsample = (int)SubSample;

         //if (fFlags->fPrint)
         //   printf("HandleVF48Event: SQLLITE: Module %d \t subsample = %f \t offset = %d \t soffset = %d \t samples = %d \n", vf48modnum, subsample, offset, soffset, gVF48Samples[vf48modnum] );

         int vf48group=-1;
         for( int vf48chan=0; vf48chan<48; vf48chan++ )
         {
            if( vf48chan%16==0 )
               vf48group++;
            if( SiModNumber[vf48chan] < 0 )
               continue;
            if( vf48chan%4 == 0 ) 
            {
               SiliconModule = new TSiliconModule( SiModNumber[vf48chan], vf48modnum, vf48group, FRCNumber[vf48chan], FRCPort[vf48chan] );
            }
            // Get the VF48 channel
            if( vf48chan >= MAX_CHANNELS )
            {
               printf("Exceeded MAX_CHANNELS\n");
               exit(1);
            }
            VF48channel* the_Channel = &the_Module->channels[vf48chan];
            const int numSamples=the_Channel->numSamples;
            SiliconVA = new TSiliconVA( ASIC[vf48chan], vf48chan );
            if(vf48chan%4==2 || vf48chan%4==3)
               SiliconVA->SetPSide( true );
            if ( numSamples> 0)
            {
               // N.B.: if channel is suppressed numSamples = 0
               int s = SOffset;
               //const int firststrip=128*(ASIC[vf48chan]-1) + 512*(SiModNumber[vf48chan]);
               for( int k=0; k<128; k++) // loop over the strips
               {
                  if( s >= numSamples ) break;
                  //Int_t stripNumber = k +firststrip;
                  int stripNumber=StripCount++;
                  Double_t stripMean= StripMeans[stripNumber];
                  //SiliconStrip = new TSiliconStrip( k, the_Channel.samples[s+offset] - TMath::Nint(stripMean));
                  //SiliconVA->AddStrip( SiliconStrip );
                  SiliconVA->AddStrip(k,the_Channel->samples[s+Offset] - TMath::Nint(stripMean),StripRMSs[stripNumber]);
                  s += subsample;
               }
            }
            else 
            {
               for( int k=0; k<128; k++)
               {
                  int stripNumber=StripCount++;
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
               PSideRawHits+=SiliconVA->CalcHits( pVASigma );
            }
            else
            {
               NSideRawHits+=SiliconVA->CalcHits( nVASigma );
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
         SiliconEvent->CompressSiliconVAs();
         SiliconEvent->CompressSiliconModules();
      }
      SiliconEvent->SetPsideNRawHits( PSideRawHits );
      SiliconEvent->SetNsideNRawHits( NSideRawHits );

      //SiliconEvent->Print();
      return SiliconEvent;
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      if (fFlags->fUnpackOff)
         return flow;

      #ifdef _TIME_ANALYSIS_
      START_TIMER
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

class HitModuleFactory_vf48: public TAFactory
{
public:
   HitFlags fFlags;
   HitModuleFactory_vf48(int VF48)
   {
       fFlags.ProcessVF48=VF48;
   }

   void Init(const std::vector<std::string> &args)
   {
      printf("HitModuleFactory_vf48(%d)::Init!\n",fFlags.ProcessVF48);
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
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
      printf("\t--nVASigma XXX \t Set the threshold on n strips (default:%f)\n",fFlags.nVASigma);
      printf("\t--pVASigma XXX \t Set the threshold on p strips (default:%f)\n",fFlags.pVASigma);
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
         if (args[i] == "--nVASigma")
            fFlags.nVASigma = atof(args[++i].c_str());
         if (args[i] == "--pVASigma")
            fFlags.pVASigma = atof(args[++i].c_str());
      }
      
      
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("HitModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("HitModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fFlags.LoadAllStrips(runinfo->fRunNo);
      return new HitModule(runinfo, &fFlags);
   }
};

static TARegister tar(new HitModuleFactory);
static TARegister tar0(new HitModuleFactory_vf48(0));
static TARegister tar1(new HitModuleFactory_vf48(1));
static TARegister tar2(new HitModuleFactory_vf48(2));
static TARegister tar3(new HitModuleFactory_vf48(3));
static TARegister tar4(new HitModuleFactory_vf48(4));
static TARegister tar5(new HitModuleFactory_vf48(5));
static TARegister tar6(new HitModuleFactory_vf48(6));
static TARegister tar7(new HitModuleFactory_vf48(7));

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
