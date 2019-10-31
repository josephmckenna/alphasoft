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


int SiModNumber[nVF48][48];
int ASIC[nVF48][48];
int FRCNumber[nVF48][48];
int FRCPort[nVF48][48];
int TTCChannel[nVF48][48];



constexpr double hmax=512;
constexpr double hmin=-hmax;
constexpr double strip_bin_width=0.1;
constexpr double strip_bins=(hmax-hmin)/strip_bin_width;
class Strip_ADC
{
   public:

      int histo[(int)strip_bins];

   inline int GetBin(const double &x)
   {
      return (int)((x - hmin) / strip_bin_width);
   }
   inline double GetX(const int &x)
   {
      return ((double)x)*strip_bin_width+hmin;
   }
   
   void InsertValue(const double &x)
   {
      if (fabs(x)>1024) return;
      int bin=GetBin(x);
      histo[bin]++;
   }
   
   double GetMean(const double _min=-9999999., const double _max=9999999.)
   {
      double mean=0.;
      int count=0;
      for (int i=0; i<(int)strip_bins; i++)
      {
         double x=GetX(i);
        // printf("bin:%d\tx:%f\n",i,x);
         if (x<_min) continue;
         if (x>_max) break;
         count++;
         mean+=x*histo[i];
      }
      return mean/(double)count;
   }
   double GetStdev(double mean,const double _min=-9999999., const double _max=9999999.)
   {
      int count=0;
      double stdev=0.;
      for (int i=0; i<(int)strip_bins; i++)
      {
         double x=GetX(i);
         if (x<_min) continue;
         if (x>_max) break;
         
         double diff=x-mean;
         stdev+=(double)histo[i]*diff*diff;

         count+=histo[i];
      }
      return sqrt(stdev/(double)count);
   }
}

Strip_ADCs[NUM_SI_MODULES*4*128];

class PedFlags
{
public:
   bool fPrint = false;
   bool fUnpackOff = false;
   bool fHitOff = false;
   int VF48commonThreshold = false;
   bool ForceStripsFile = false;
   TString CustomStripsFile;
   int ProcessVF48=-1;
   double NSIGMATHRES=3.;
};



void CountVF48Module(VF48event* e)//,const int vf48modnum)
{


   for( int vf48modnum=0; vf48modnum<NUM_VF48_MODULES; vf48modnum++ )
   {
      // Get the VF48 module
      VF48module* the_Module = e->modules[vf48modnum];
      if( !the_Module ) return;
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
         
         // Get the VF48 channel
         if( vf48chan >= MAX_CHANNELS )
         {
            printf("Exceeded MAX_CHANNELS\n");
            exit(1);
         }
         VF48channel* the_Channel = &the_Module->channels[vf48chan];
         int numSamples=the_Channel->numSamples;
         TSiliconVA* SiliconVA = new TSiliconVA( ASIC[vf48modnum][vf48chan], vf48chan );
         if(vf48chan%4==2 || vf48chan%4==3)
            SiliconVA->SetPSide( true );

         int s = gSOffset[vf48modnum];
         // Determine the raw ADC for each strip by subsampling the VF48 samples
         for( int k=0; k<128; k++){
            if( s >= numSamples ) continue;
            SiliconVA->RawADC[k]=the_Channel->samples[s];
            s += (int)gSubSample[vf48modnum];
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
         
         for (int k=0; k<128; k++)
         {
            Int_t stripNumber = k + 128*(ASIC[vf48modnum][vf48chan]-1) + 512*(SiModNumber[vf48modnum][vf48chan]);
            Strip_ADCs[stripNumber].InsertValue(SiliconVA->PedSubADC[k]);
         }
         delete SiliconVA;
      }//loop over VF48 channels
   } // loop over VF48 modules

   return;
}


class PedModule: public TARunObject
{
public:
   PedFlags* fFlags = NULL;
   bool fTrace = false;

   TVF48SiMap *gVF48SiMap = NULL;
   PedModule(TARunInfo* runinfo, PedFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("PedModule::ctor!\n");

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

      delete SettingsDB;
   }

   ~PedModule()
   {
      if (fTrace)
         printf("PedModule::dtor!\n");
      delete gVF48SiMap;
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

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("PedModule::EndRun, run %d\n", runinfo->fRunNo);

      // create extra root file
      char filename[80]; 
      sprintf(filename,"alphaStrips%05doffline.root", /*dir,*/ runinfo->fRunNo);
      TFile* file = new TFile(filename,"RECREATE");

      Int_t stripNumber=0;
      Float_t stripMean;
      Float_t stripRMS;
      Float_t stripMeanSubRMS;

      TTree* alphaStripTree = new TTree("alphaStrip Tree","alphaStrip Tree");
      alphaStripTree->Branch("stripNumber",&stripNumber, "stripNumber/I");
      alphaStripTree->Branch("stripMean",&stripMean, "stripMean/F");
      alphaStripTree->Branch("stripRMS",&stripRMS, "stripRMS/F");
      alphaStripTree->Branch("stripMeanSubRMS",&stripMeanSubRMS, "stripMeanSubRMS/F");

      double sigma=fFlags->NSIGMATHRES;
      for (int i=0; i<NUM_SI_MODULES*4*128; i++)
      {
         //Calculate mean:
         double mean=Strip_ADCs[i].GetMean();

         //Calculate RMS
         double stdev=Strip_ADCs[i].GetStdev(mean);

         //Find range around mean (filter)
         double min=mean-sigma*stdev;
         double max=mean+sigma*stdev;

         //Recalculate mean in range
         double clean_mean=Strip_ADCs[i].GetMean(min,max);
         
         double clean_stdev=Strip_ADCs[i].GetStdev(mean,min,max);
         printf("\nmean:%f\tstdev:%f\tmin:%f\tmax:%f\tclean_mean:%f\tclean_rms:%f\n", mean,stdev,min,max,clean_mean,clean_stdev);

         stripMean=mean;
         stripRMS=clean_stdev;
         stripMeanSubRMS=mean-stripRMS;
         alphaStripTree->Fill();
         stripNumber++;
      }
      file->Write();
      file->Close();
      delete file;
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
      CountVF48Module(fe->vf48event);
      //flow=new SilEventsFlow(flow,s);
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"strip_peds_module",timer_start);
      #endif
      return flow;
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
      printf("PedModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("PedModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new PedModule(runinfo, &fFlags);
   }
};

static TARegister tar(new PedModuleFactory);
/*static TARegister tar0(new PedModuleFactory_vf48_0);
static TARegister tar1(new PedModuleFactory_vf48_1);
static TARegister tar2(new PedModuleFactory_vf48_2);
static TARegister tar3(new PedModuleFactory_vf48_3);
static TARegister tar4(new PedModuleFactory_vf48_4);
static TARegister tar5(new PedModuleFactory_vf48_5);
static TARegister tar6(new PedModuleFactory_vf48_6);
static TARegister tar7(new PedModuleFactory_vf48_7);
*/
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
