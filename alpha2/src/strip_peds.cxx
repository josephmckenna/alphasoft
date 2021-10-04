// HandleVF48.cxx

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <iomanip>
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
#include "TBranch.h"


#include "manalyzer.h"
#include "midasio.h"

#include "TSettings.h"
#include "ALPHA2SettingsDatabase.h"
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
   static int nPedBins;
   static double pedBinWidth;
};
bool    PedFlags::fPrint = false;
bool    PedFlags::fUnpackOff = false;
bool    PedFlags::fHitOff = false;
int     PedFlags::VF48commonThreshold = false;
bool    PedFlags::ForceStripsFile = false;
TString PedFlags::CustomStripsFile="";
double  PedFlags::NSIGMATHRES=3.;
int 	PedFlags::nPedBins = 512;
double 	PedFlags::pedBinWidth = 1.0;


class PedModule_vf48: public TARunObject
{
public:
   PedFlags* fFlags = NULL;

   TSettings *SettingsDB = NULL;
   TVF48SiMap *gVF48SiMap = NULL;
   
   //Static so its shared between all VF48 threads
   //Old array
   //static TStripPed Strip_ADCs[NUM_SI_MODULES*4*128];
   //New vector below
   static std::vector<TStripPed*> Strip_ADCs;

   
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
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="ped_module_vf48(" + std::to_string(fFlags->ProcessVF48) + ")";
#endif
	  
	  //New declaration in initiator. 
	  //std::vector<TStripPed> vec;
	  //printf("Strip_ADC has size %d \n", Strip_ADCs.size());
	  printf("nPedBins = %d, and pedBinWidth = %f \n", fFlags->nPedBins, fFlags->pedBinWidth);
	  
	  if(Strip_ADCs.size() == 0)
	  {
		  Strip_ADCs.reserve(NUM_SI_MODULES * 4 * 128);
		  //printf("Strip_ADC has size %d \n", Strip_ADCs.size());
		  for (int i = 0; i < NUM_SI_MODULES * 4 * 128; i++)
		  {
			  //printf("Strip_ADC has size %d \n", Strip_ADCs.size());
           Strip_ADCs.push_back(new TStripPed(fFlags->nPedBins, fFlags->pedBinWidth));
		  }
	  }
	  //Strip_ADCs = &vec;
	  //Strip_ADCs(NUM_SI_MODULES*4*128,TStripPed(1024,0.1));
	  

      // load the sqlite3 db
      SettingsDB = ALPHA2SettingsDatabase::GetTSettings(runinfo->fRunNo);
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
		//LMG Print statement here pls
   }
   ~PedModule_vf48()
   {
      for (TStripPed* s: Strip_ADCs)
      {
         if(s) 
         {
            delete s;
            s = NULL;
         }
      }
      Strip_ADCs.clear();
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
               Strip_ADCs[firstStrip + k]->InsertValue(SiliconVA.PedSubADC[k],SiliconVA.RawADC[k]);
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
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags |= TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }

      VF48EventFlow* fe=flow->Find<VF48EventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags |= TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      if (!fe->vf48event)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags |= TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      CountVF48Module(fe->vf48event,fFlags->ProcessVF48);

      return flow;
   }
   void PreEndRun(TARunInfo* runinfo) 
   {
      if (fFlags->fPrint)
         printf("PedModule::PreEndRun, run %d\n", runinfo->fRunNo);
   }
   void EndRun(TARunInfo* runinfo)
   {
      if (fFlags->fPrint)
         printf("PedModule::EndRun, run %d\n", runinfo->fRunNo);
      // Only write root file with 0th VF48 module thread
      if (fFlags->ProcessVF48!=0) return;
      // create extra root file
      //char filename[80]; 
      std::ostringstream filename;
      filename << getenv("A2DATAPATH") << "/alphaStrips" << std::setw(5) << std::setfill('0') << runinfo->fRunNo << "offline.root" ;
      //sprintf(filename,"%s/alphaStrips%05doffline.root",  getenv("A2DATAPATH"), runinfo->fRunNo);
      TFile* file = new TFile(filename.str().c_str(),"RECREATE");

      Int_t stripNumber=0;
      Float_t stripMean;
      Float_t stripRMS;
      Float_t stripRMSAfterFilter;
      Float_t stripMeanSubRMS;

      TTree* alphaStripTree = new TTree("alphaStrip Tree","alphaStrip Tree");
      TBranch* stripNumberBranch     = alphaStripTree->Branch("stripNumber",&stripNumber, "stripNumber/I");
      TBranch* stripMeanBranch       = alphaStripTree->Branch("stripMean",&stripMean, "stripMean/F");
      TBranch* stripRMSBranch        = alphaStripTree->Branch("stripRMS",&stripRMS, "stripRMS/F");
      TBranch* stripMeanSubRMSBranch = alphaStripTree->Branch("stripMeanSubRMS",&stripRMSAfterFilter, "stripMeanSubRMS/F");
      stripNumberBranch->SetFile(filename.str().c_str());
      stripMeanBranch->SetFile(filename.str().c_str());
      stripRMSBranch->SetFile(filename.str().c_str());
      stripMeanSubRMSBranch->SetFile(filename.str().c_str());

      for (int i=0; i<NUM_SI_MODULES*4*128; i++)
      {
         Strip_ADCs[i]->sigma=fFlags->NSIGMATHRES;
         Strip_ADCs[i]->CalculatePed();
         stripMean      =(float)Strip_ADCs[i]->stripMean;
         stripRMS       =(float)Strip_ADCs[i]->stripRMS;
         stripMeanSubRMS=(float)Strip_ADCs[i]->stripMeanSubRMS;
         stripRMSAfterFilter=(float)Strip_ADCs[i]->StripRMSsAfterFilter;
         
         
         alphaStripTree->Fill();
         stripNumber++;
      }
      std::cout<<"Writing strip root file"<<std::endl;
      file->Write();
      //std::cout<<"Close"<<std::endl;
      /*file->Close should delete all of these for us:
       * delete stripNumberBranch;
       * delete stripMeanBranch;
       * delete stripRMSBranch;
       * delete stripMeanSubRMSBranch;*/
      file->Close();
      //std::cout<<"delete"<<std::endl;
      
      delete file;
      std::cout<<"done"<<std::endl;
   }
};


std::vector<TStripPed*> PedModule_vf48::Strip_ADCs = std::vector<TStripPed*> ();

/*for (int i = 0; i < NUM_SI_MODULES * 4 * 128; i++)
{
	PedModule_vf48::Strip_ADCs.push_back(TStripPed(1024, 0.1));
}*/
//TStripPed PedModule_vf48::Strip_ADCs[NUM_SI_MODULES*4*128]; //Remove this and declare in constructor(?) LMG

//PedModule_vf48::Strip_ADCs(NUM_SI_MODULES*4*128,TStripPed(1024,0.1));

class PedModuleFactory_vf48: public TAFactory
{
public:
   PedFlags fFlags;
   PedModuleFactory_vf48(int vf48)
   {
      fFlags.ProcessVF48=vf48;
   }
   void Help()
   {
      printf("PedModuleFactory::Help!\n");
      printf("\t--nounpack   Turn unpacking of TPC data (turn off reconstruction completely)\n");
      printf("\t--nPedBins xxx Set the range of the bins in ped calculation (default value %d)\n", fFlags.nPedBins);
      printf("\t--pedBinWidth xxx Set the bin width in ped calculation (default value %f)\n", fFlags.pedBinWidth);
      printf("\t--stripsSigma xxx Set the number of standard deviations from centre to filter out signal (default value %f)\n", fFlags.NSIGMATHRES);
   }
   void Usage()
   {
     Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("PedModuleFactory_vf48(%d)::Init!\n",fFlags.ProcessVF48);

      for (unsigned i=0; i<args.size(); i++)
      {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--nounpack")
            fFlags.fUnpackOff = true;
         if (args[i] == "--nPedBins")
         {
            fFlags.nPedBins = stoi(args[i+1]);
            i++;
            continue;
         }
         if (args[i] == "--pedBinWidth")
         {
            fFlags.pedBinWidth = stod(args[i+1]);
            i++;
            continue;
         }
         if (args[i] == "--stripSigma" || args[i] == "--stripsSigma")
         {
			 fFlags.NSIGMATHRES = stod(args[i+1]);
			 i++;
			 printf("Sigma has been set at %f \n", fFlags.NSIGMATHRES);
			 continue;
		   }
      }
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new PedModule_vf48(runinfo, &fFlags);
   }
};

static TARegister tar0(new PedModuleFactory_vf48(0));
static TARegister tar1(new PedModuleFactory_vf48(1));
static TARegister tar2(new PedModuleFactory_vf48(2));
static TARegister tar3(new PedModuleFactory_vf48(3));
static TARegister tar4(new PedModuleFactory_vf48(4));
static TARegister tar5(new PedModuleFactory_vf48(5));
static TARegister tar6(new PedModuleFactory_vf48(6));
static TARegister tar7(new PedModuleFactory_vf48(7));

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
