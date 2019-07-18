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

class UnpackFlags
{
public:
   bool fPrint = false;
   bool fUnpackOff = false; //Turn off unpacking (reconstruction totally off)
   bool fTimeCut = false;
   double start_time = -1.;
   double stop_time = -1.;
   bool fEventRangeCut = false;
   int start_event = -1;
   int stop_event = -1;

   bool fStopUnpackAfterTime = false;
   double fStopUnpackAfter = -1.;

};

class UnpackModule: public TARunObject
{
public:
   UnpackFlags* fFlags = NULL;
   UnpackVF48* vfu; 
   bool fTrace = false;
      
   // VF48 sampling parameters from sqlite db?

   // Sub-sampling settings
   // int soffset(-1);
   // double subsample(-1.);
   //  int offset(-1);
   double gSubSample[NUM_VF48_MODULES];
   int gOffset[NUM_VF48_MODULES];
   int gSOffset[NUM_VF48_MODULES];
   double gSettingsFrequencies[VF48_MAX_MODULES];


   UnpackModule(TARunInfo* runinfo, UnpackFlags* flags)
      : TARunObject(runinfo)
   {
      if (fTrace)
         printf("UnpackModule::ctor!\n");
      vfu = new UnpackVF48();
      // load the sqlite3 db
      char dbName[255]; 
      sprintf(dbName,"%s/a2lib/main.db",getenv("AGRELEASE"));
      TSettings *SettingsDB = new TSettings(dbName,runinfo->fRunNo);      
      for (int m=0; m<NUM_VF48_MODULES; m++)
      {
         gSettingsFrequencies[m]= SettingsDB->GetVF48Frequency( runinfo->fRunNo, m);
         vfu->SetTsFreq(m,gSettingsFrequencies[m]);
         // extract VF48 sampling parameters from sqlite db
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
      delete SettingsDB;
      fFlags   = flags;
   }

   ~UnpackModule()
   {
      if (fTrace)
         printf("UnpackModule::dtor!\n");
      delete vfu;
   }


   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("UnpackModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      int module = 0;
      int samples    = runinfo->fOdb->odbReadInt("/equipment/VF48/Settings/VF48_NumSamples",module,0);
      int grpEnabled = runinfo->fOdb->odbReadInt("/equipment/VF48/Settings/VF48_GroupEnable",module,0);

      printf("Module %d, samples: %d, grpEnable: 0x%x\n", module, samples, grpEnabled);
      vfu->SetFlushIncompleteThreshold(40);
      vfu->SetNumModules(NUM_VF48_MODULES);
      vfu->SetGroupEnableMask(-1, grpEnabled);
      vfu->SetNumSamples(-1, samples);
      vfu->SetCoincTime(VF48_COINCTIME);
      vfu->Reset();


   }

   void PreEndRun(TARunInfo* runinfo, std::deque<TAFlowEvent*>* flow_queue)
   {
      if (fTrace)
         printf("UnpackModule::PreEndRun, run %d\n", runinfo->fRunNo);
      //time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      //printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("UnpackModule::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("UnpackModule::PauseRun, run %d\n", runinfo->fRunNo);
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
      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif
      event->FindAllBanks();
      for (int i=0; i<NUM_VF48_MODULES; i++) 
      {
         char bankname[5];
         bankname[0] = 'V';
         bankname[1] = 'F';
         bankname[2] = 'A';
         bankname[3] = '0' + i;
         bankname[4] = 0;
         //int size = event->LocateBank(NULL,bankname,&ptr);
         TMBank* vf48_bank = event->FindBank(bankname); 
         if (!vf48_bank) continue;
         int size=vf48_bank->data_size/4;
         if ( size > 0 )//&& (!gRecOff||gADCspecs))
         {
            // printf("VF48 bank %s size %d\n", bankname, size);
            //          UnpackVF48(i, size, ptr, gVf48disasm,false); //new Konstantin function
            //          vfu->fBadDataCount = 0;
            vfu->UnpackStream(i, event->GetBankData(vf48_bank), size);
            while (1)
            {
               VF48event* e = vfu->GetEvent();
               if (e)
               {
                  // check for errors
                  int trigs = 0;
                  for( int imod = 0; imod < NUM_VF48_MODULES; imod++)
                  {
                     VF48module* the_Module = e->modules[imod];
                     // All modules should be present
                     // there is probably a problem with the event
                     // <<<< -----
                     if(  !the_Module )
                     {
                        printf("Event %d: Error VF48 module %d not present\n", (int)e->eventNo, imod);
                        //if(gVerbose)
                        //   e->PrintSummary();
                        //gBadVF48Events++;
                        delete e;
                        e=NULL;
                        break;
                     }

                     if( the_Module->error != 0 )
                     {
                        printf("Event %d: Found VF48 error, not using event\n", (int)e->eventNo);
                        //if(gVerbose) e->PrintSummary();
                        //   gBadVF48Events++;
                        delete e;
                        e=NULL;
                        break;
                     }
                  }
                  // end check for errors

                  TAFlowEvent* timer=NULL;
                  #ifdef _TIME_ANALYSIS
                     if (TimeModules)
                        timer=new AgAnalysisReportFlow(NULL,"unpack_vf48_module",timer_start)
                  #endif
                  runinfo->AddToFlowQueue(new VF48EventFlow(timer,e));
               }
               else
               {
                  break;
               }
            }
         }
      }
      return flow;
   }

};

class UnpackModuleFactory: public TAFactory
{
public:
   UnpackFlags fFlags;

public:
   void Help()
   {
      printf("UnpackModuleFactory::Help!\n");
      printf("\t--print      Turn printing on\n");
      printf("\t--noadc      Turn adc off\n");
      printf("\t--nopwb      Turn pwd off\n");
      printf("\t--nounpack   Turn unpacking of TPC data (turn off reconstruction completely)\n");
      printf("\t--usetimerange 123.4 567.8\t\tLimit reconstruction to a time range\n");
      printf("\t--useeventrange 123 567\t\tLimit reconstruction to an event range\n");
      printf("\t--stopunpackafter 567.8\t\tStop unpacking after time\n");
   }
   void Usage()
   {
     Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("UnpackModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--nounpack")
            fFlags.fUnpackOff = true;
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
         if( args[i] == "--stopunpackafter" )
            {
               fFlags.fStopUnpackAfterTime=true;
               i++;
               fFlags.fStopUnpackAfter=atof(args[i].c_str());
            }
      }
   }

   void Finish()
   {
      printf("UnpackModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("UnpackModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new UnpackModule(runinfo, &fFlags);
   }
};

static TARegister tar(new UnpackModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
