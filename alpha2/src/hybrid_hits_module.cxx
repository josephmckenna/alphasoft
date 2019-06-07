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
   bool fPrint = false;
   bool fUnpackOff = false;
   bool fHitOff = false;
   int VF48commonThreshold = false;
   bool ForceStripsFile = false;
   TString CustomStripsFile;
};

class HitModule: public TARunObject
{
public:
   HitFlags* fFlags = NULL;
   bool fTrace = false;

   int gVF48Samples[NUM_VF48_MODULES];
   double gSubSample[NUM_VF48_MODULES];
   int gOffset[NUM_VF48_MODULES];
   int gSOffset[NUM_VF48_MODULES];

   double StripRMSs[NUM_SI_MODULES*4*128];
   double StripMeans[NUM_SI_MODULES*4*128];
   double SumRMSs[NUM_SI_MODULES*4*128];

   double nVASigma = 2.375;//3.125;
   double nClusterSigma = 3.5;//nVASigma;
   double pVASigma = 2.75;//3.75;
   double pClusterSigma = 6;//pVASigma;

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
      striprms_tree->SetBranchAddress("stripMeanSubRMS", &stripRMS );
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
      // VF48 - SiModule mapping variables
      int SiModNumber = -1;  // 0 <= SiModNumber < NUM_SI_MODULES
      int ASIC = -1;         // 1 <= ASIC <= 4
      int FRCNumber = -1;    //
      int FRCPort = -1;      //
      int TTCChannel = -1;

      TSiliconModule* SiliconModule = NULL;
      TSiliconVA* SiliconVA = NULL;
      TSiliconStrip* SiliconStrip = NULL;

      Double_t NSideRawHits=0;
      Double_t PSideRawHits=0;

      for( int vf48modnum=0; vf48modnum<NUM_VF48_MODULES; vf48modnum++ )
      {
         // Get the VF48 module
         VF48module* the_Module = e->modules[vf48modnum];
         if( !the_Module ) continue;

         // identify number of samples/strip runs
         //VF48channel channel = the_Module->channels[0];

         double subsample = gSubSample[vf48modnum];
         int offset = gOffset[vf48modnum];
         int soffset = gSOffset[vf48modnum];

         if (fFlags->fPrint)
            printf("HandleVF48Event: SQLLITE: Module %d \t subsample = %f \t offset = %d \t soffset = %d \t samples = %d \n", vf48modnum, subsample, offset, soffset, gVF48Samples[vf48modnum] );

         int vf48group=-1;
         for( int vf48chan=0; vf48chan<48; vf48chan++ )
         {
            if( vf48chan%16==0 )
               vf48group++;
            gVF48SiMap->GetSil( vf48modnum, vf48chan, SiModNumber, ASIC, FRCNumber, FRCPort, TTCChannel );
            if( SiModNumber < 0 )
               continue;
            if( vf48chan%4 == 0 ) 
            {
               SiliconModule = new TSiliconModule( SiModNumber, vf48modnum, vf48group, FRCNumber, FRCPort );
            }
            // Get the VF48 channel
            if( vf48chan >= MAX_CHANNELS )
            {
               printf("Exceeded MAX_CHANNELS\n");
               exit(1);
            }
            VF48channel the_Channel = the_Module->channels[vf48chan];
            SiliconVA = new TSiliconVA( ASIC, vf48chan );
            if(vf48chan%4==2 || vf48chan%4==3)
               SiliconVA->SetPSide( true );

            if (the_Channel.numSamples > 0)
            {
               // N.B.: if channel is suppressed numSamples = 0
              int s = soffset;
              for( int k=0; k<128; k++) // loop over the strips
              {
                 if( s >= the_Channel.numSamples ) continue;
                 Int_t stripNumber = k + 128*(ASIC-1) + 512*(SiModNumber);
                 Double_t stripMean= StripMeans[stripNumber];
                 SiliconStrip = new TSiliconStrip( k, the_Channel.samples[s+offset] - TMath::Nint(stripMean));
                 SiliconVA->AddStrip( SiliconStrip );
                 s += (int)subsample;
              }
           }
           else 
           {
              for( int k=0; k<128; k++)
              {
                 SiliconStrip = new TSiliconStrip( k, 0);
                 SiliconVA->AddStrip( SiliconStrip );
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
            
           if(vf48chan%4==2 || vf48chan%4==3)
           {
              Double_t thesigma(pVASigma);
              PSideRawHits+=SiliconVA->CalcHits( thesigma, StripRMSs, SiModNumber );
           }
           else
           {
              Double_t thesigma(nVASigma);
              NSideRawHits+=SiliconVA->CalcHits( thesigma, StripRMSs, SiModNumber );
           }
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

     SiliconEvent->CompressSiliconModules();

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
      clock_t timer_start=clock();
      #endif
      VF48EventFlow* fe=flow->Find<VF48EventFlow>();
      if (!fe)
         return flow;
      TSiliconEvent* s=BuildTSiliconEvent(fe->vf48event);
      flow=new SilEventsFlow(flow,s);
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"hybrid_hits_module",timer_start);
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("HitModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
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

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
