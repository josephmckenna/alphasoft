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

#define MAX_CHANNELS VF48_MAX_CHANNELS // defined in UnpackVF48.h
#define NUM_SI_MODULES nSil // defined in SiMod.h
#define NUM_VF48_MODULES nVF48 // defined in SiMod.h
#define NUM_SI_ALPHA1 nSiAlpha1 // defined in SiMod.h
#define NUM_SI_ALPHA2 nSiAlpha2 // defined in SiMod.h

#define VF48_COINCTIME 0.000010

class ZeroSuppFlags
{
public:
   bool fPrint = false;
   bool fUnpackOff = false;
   bool fZeroSuppOff = false;
   int VF48commonThreshold = false;
};

class ZeroSuppModule: public TARunObject
{
public:
   ZeroSuppFlags* fFlags = NULL;
   int xthresholds[8][6][8];
   bool fTrace = false;
   
   int fwmask[VF48_MAX_GROUPS];
  
   int ctotal;
   int cpresent;

   int xctotal[VF48_MAX_MODULES][VF48_MAX_GROUPS];
   int xcpresent[VF48_MAX_MODULES][VF48_MAX_GROUPS];
  
   int Navg;
   bool lastSampleBug;
 
   ZeroSuppModule(TARunInfo* runinfo, ZeroSuppFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("ZeroSuppModule::ctor!\n");
         

      const char* fname = getenv("THRESFILE");//"Threshold_Levels30.csv";

      printf("Reading VF48 thresholds file %s\n", fname);

      FILE *fp = fopen(fname, "r");
      assert(fp != NULL);
      for (int i=0; i<NUM_VF48_MODULES; i++) 
      {
         char s[1024];
	     fgets(s, sizeof(s), fp);
	     char* sptr = s;
	     for (int j=0; j<VF48_MAX_GROUPS; j++)
	     {
	        for (int k=0; k<8; k++) 
	        {
	           xthresholds[i][j][k] = strtoul(sptr, &sptr, 0);
	           sptr++;
	        }
         }
         fclose(fp);
         // values have to be doctored by hand
         xthresholds[2][0][0] = 10;
         for (int i=0; i<NUM_VF48_MODULES; i++)
         {
	        for (int j=0; j<VF48_MAX_GROUPS; j++)
	        {
	           for (int k=0; k<8; k++)
	           {
	              printf(" %3d", xthresholds[i][j][k]);
	              printf("\n");
			   }
		    }
	     }
      }
   }

   ~ZeroSuppModule()
   {
      if (fTrace)
         printf("ZeroSuppModule::dtor!\n");

   }


int VF48_HaveHit(const VF48channel *chan, int16_t threshold, int Navg, bool lastSampleBug, int16_t* smin, int16_t* smax) {
  
  int num_samples = chan->numSamples;

  assert(num_samples > 2);

  *smin = 9999;
  *smax = -9999;
            
  if (lastSampleBug)
    num_samples -= 1;

  for (int i=1; i<num_samples-Navg; i++) {
    int da = 0; 
    for (int j=0; j<Navg; j++) {
      da += chan->samples[i+j];
    }
    da /= Navg;
    
    int16_t data9 = chan->samples[i-1];
    int16_t data0 = chan->samples[i+Navg];
    
    int16_t r = da;
     
    int16_t mon0 = data0 - r;
    int16_t mon9 = data9 - r;

    if (mon0 <  *smin)
      *smin = mon0;

    if (mon9 <  *smin)
      *smin = mon9;

    if (mon0 > *smax)
      *smax = mon0;

    if (mon9 > *smax)
      *smax = mon9;
  }
  
  int16_t t = threshold;
  int16_t tinv = ~t;
  
  int hitp = (*smax > t);
  int hitn = (*smin <= tinv);
  int h = (hitp||hitn);
 

  return h;
}
   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ZeroSuppModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void PreEndRun(TARunInfo* runinfo, std::deque<TAFlowEvent*>* flow_queue)
   {
      if (fTrace)
         printf("ZeroSuppModule::PreEndRun, run %d\n", runinfo->fRunNo);
      //time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      //printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));

   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ZeroSuppModule::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ZeroSuppModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      if (fFlags->fUnpackOff)
         return flow;

      if (event->event_id != 11)
         return flow;

      VF48EventFlow* fe=flow->Find<VF48EventFlow>();
      if (!fe)
         return flow;
      int n_events=fe->vf48events.size();
      for (int eventi=0; eventi<n_events; eventi++)
      {
         VF48event* e=fe->vf48events.at(eventi);
         for( int vf48modnum=0; vf48modnum<NUM_VF48_MODULES; vf48modnum++ )
         {
            if (!e->modules[vf48modnum])
            continue;

            //int fwmask[VF48_MAX_GROUPS];
            int swmask[VF48_MAX_GROUPS];
            int rdmask[VF48_MAX_GROUPS];

            for (int grp=0; grp<VF48_MAX_GROUPS; grp++)
            {
               fwmask[grp] = e->modules[vf48modnum]->suppressMask[grp];
               swmask[grp] = 0;
               rdmask[grp] = 0;

               for (int ch=0; ch<8; ch++ )
               {
                  int vf48chan = grp*8+ch;
                  int num_samples = e->modules[vf48modnum]->channels[vf48chan].numSamples;
                  //int fwhit = (fwmask[grp] & (1<<ch));
                  ctotal++;
                  xctotal[vf48modnum][grp]++;
                  if (num_samples > 0)
                  {
                     cpresent++;
                     xcpresent[vf48modnum][grp]++;
                     rdmask[grp] |= (1<<ch); // <<<<<<<------
                     if (1)
                     {
                        int threshold = fFlags->VF48commonThreshold;		
                        if (threshold==0) 
                        {
                           threshold = xthresholds[vf48modnum][grp][ch];
                        }
                        int16_t smin = 0;
                        int16_t smax = 0;
                        int swhit = VF48_HaveHit(&e->modules[vf48modnum]->channels[vf48chan], threshold, Navg, lastSampleBug, &smin, &smax); //...
                        if (swhit)
                        swmask[grp] |= (1<<ch);

#if 0
                        hmin[vf48modnum]->Fill(smin);
                        hmax[vf48modnum]->Fill(smax);

                        if (vf48chan==0) {
                           hmin_ch0[vf48modnum]->Fill(smin);
                           hmax_ch0[vf48modnum]->Fill(smax);
                        }

                        if (vf48chan==1) {
                           hmin_ch1[vf48modnum]->Fill(smin);
                           hmax_ch1[vf48modnum]->Fill(smax);
                        }

                        if (vf48chan==2) {
                           hmin_ch2[vf48modnum]->Fill(smin);
                           hmax_ch2[vf48modnum]->Fill(smax);
                        }

                        if (vf48chan==3) {
                           hmin_ch3[vf48modnum]->Fill(smin);
                           hmax_ch3[vf48modnum]->Fill(smax);
                        }
#endif

                        if (1) // emulate hardware channel suppression
                        {
                           if (!swhit)
                           {
                              // printf("%d",e->modules[vf48modnum]->channels[vf48chan].samples[0]); //Output before zeroing
                              e->modules[vf48modnum]->channels[vf48chan].numSamples = 0;
                              for (int s=0; s<num_samples; s++)
                              {
                                 e->modules[vf48modnum]->channels[vf48chan].samples[s] = 0;
                              }
                           }
                           // printf("-%d \n",e->modules[vf48modnum]->channels[vf48chan].samples[0]); //Zeroed outputs
                        }
                     }
                  }
               }
            }
         }
      }
     #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"ZeroSupp_vf48_module");
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("ZeroSuppModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class ZeroSuppModuleFactory: public TAFactory
{
public:
   ZeroSuppFlags fFlags;

public:
   void Help()
   {
      printf("ZeroSuppModuleFactory::Help!\n");
      printf("\t--nounpack   Turn unpacking of TPC data (turn off reconstruction completely)\n");
   }
   void Usage()
   {
     Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("ZeroSuppModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--nounpack")
            fFlags.fUnpackOff = true;
        }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("ZeroSuppModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("ZeroSuppModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new ZeroSuppModule(runinfo, &fFlags);
   }
};

static TARegister tar(new ZeroSuppModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
