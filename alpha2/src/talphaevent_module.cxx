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

#include "TAlphaEvent.h"

#include "TVF48SiMap.h"
#include "TSystem.h"


#include <TGeoManager.h>
#include "TAlphaGeoDetectorXML.h"
#include "TAlphaGeoMaterialXML.h"
#include "TAlphaGeoEnvironmentXML.h"

#define MAX_CHANNELS VF48_MAX_CHANNELS // defined in UnpackVF48.h
#define NUM_SI_MODULES nSil // defined in SiMod.h
#define NUM_VF48_MODULES nVF48 // defined in SiMod.h
#define NUM_SI_ALPHA1 nSiAlpha1 // defined in SiMod.h
#define NUM_SI_ALPHA2 nSiAlpha2 // defined in SiMod.h

#define VF48_COINCTIME 0.000010

class AlphaEventFlags
{
public:
   bool fPrint = false;
   bool SaveTAlphaEvent = true;
   
   int gNHitsCut = 200;
   double nClusterSigma = 3.5;//nVASigma;
   double pClusterSigma = 6;//pVASigma;
   double hitSigmaCut = 0.;//nVASigma;
   double hitThresholdCut = 99999;
   
};

class AlphaEventModule: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   bool fTrace = false;

   TAlphaEvent* AlphaEvent = NULL;
   TTree* AlphaEventTree   = NULL;
   
   TVF48SiMap *gVF48SiMap = NULL;

   AlphaEventModule(TARunInfo* runinfo, AlphaEventFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("AlphaEventModule::ctor!\n");
      AlphaEvent = new TAlphaEvent(); 
      if (fFlags->SaveTAlphaEvent)
      {
         AlphaEventTree = new TTree("gAlphaEventTree","Alpha Event Tree");
         AlphaEventTree->Branch("AlphaEvent","TAlphaEvent",&AlphaEvent,16000,1);
      }
      
      // load the sqlite3 db
      char dbName[255]; 
      sprintf(dbName,"%s/a2lib/main.db",getenv("AGRELEASE"));
      TSettings *SettingsDB = new TSettings(dbName,runinfo->fRunNo);

      char name[200];
      sprintf(name,"%s%s%s",
         getenv("AGRELEASE"),
         SettingsDB->GetVF48MapDir().Data(),
         SettingsDB->GetVF48Map(runinfo->fRunNo).Data());
      printf("name: %s\n",name);
      gVF48SiMap = new TVF48SiMap(name);

       // Initialize geometry
       new TGeoManager("TGeo", "Root geometry manager");
       TString dir = SettingsDB->GetDetectorGeoDir();
       TString mat = SettingsDB->GetDetectorMat( runinfo->fRunNo );
       
    sprintf(name,"%s%s%s",getenv("AGRELEASE"),dir.Data(),mat.Data());
    TAlphaGeoMaterialXML * materialXML = new TAlphaGeoMaterialXML();
    materialXML->ParseFile(name);
    delete materialXML;
    
    TAlphaGeoEnvironmentXML * environmentXML = new TAlphaGeoEnvironmentXML();
    TString env = SettingsDB->GetDetectorEnv( runinfo->fRunNo );
    sprintf(name,"%s%s%s",getenv("AGRELEASE"),dir.Data(),env.Data());
    environmentXML->ParseFile(name);
    delete environmentXML;
    

       TString det = SettingsDB->GetDetectorGeo( runinfo->fRunNo );
       sprintf(name,"%s%s%s",getenv("AGRELEASE"),dir.Data(),det.Data() );
       printf(" det: %s\n",name);
       TAlphaGeoDetectorXML * detectorXML = new TAlphaGeoDetectorXML();
       detectorXML->ParseFile(name);

       delete SettingsDB;
   }

   ~AlphaEventModule()
   {
      if (fTrace)
         printf("AlphaEventModule::dtor!\n");
      delete AlphaEvent;
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
   

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* event, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);

      if (event->event_id != 11)
         return flow;

      SilEventsFlow* fe=flow->Find<SilEventsFlow>();
      if (!fe)
         return flow;
      int n_events=fe->silevents.size();
      if (!n_events)
      {
         return flow;
      }
      //std::cout<<"N Events: " <<n_events<<std::endl;
      for (int i=0; i<n_events; i++)
      {

         TSiliconEvent* SiliconEvent=fe->silevents.at(i);
         AlphaEvent->DeleteEvent();
         if( AlphaEvent )
         {
            int m, c, ttcchannel;
            for( int isil = 0; isil < NUM_SI_MODULES; isil++ )
            {
               TSiliconModule * module = SiliconEvent->GetSiliconModule( isil );
               if( !module ) continue;

               gVF48SiMap->GetVF48( isil,1, m, c, ttcchannel); // check that the mapping exists
               if( m == -1 ) continue; // if not, continue

               Char_t * name = (Char_t*)gVF48SiMap->GetSilName(isil).c_str();
               TAlphaEventSil *sil = new TAlphaEventSil(name);

               AlphaEvent->AddSil(sil);
               for( int iASIC = 1; iASIC <= 4; iASIC++ ) 
               {
                  gVF48SiMap->GetVF48( isil,iASIC, m, c, ttcchannel);
                  TSiliconVA * asic = module->GetASIC( iASIC );
                  if( !asic ) continue;

                  std::vector<TSiliconStrip*> strip_array = asic->GetStrips();
                  Int_t nASIC  = asic->GetASICNumber();

                  for(uint s = 0; s<strip_array.size(); s++)
                  {
                     TSiliconStrip * strip = (TSiliconStrip*) strip_array.at(s);
                     if (!strip) continue;
                     Int_t ss = strip->GetStripNumber();
                     double* theRMS=NULL;
                     double* theASIC=NULL;
                     if( nASIC == 1 )
                     {
                        theASIC = sil->GetASIC1();
                        theRMS= sil->GetRMS1();
                     }
                     else if( nASIC == 2)
                     {
                        theASIC = sil->GetASIC2();
                        theRMS= sil->GetRMS2();
                     }
                     else if( nASIC == 3)
                     {
                        theASIC = sil->GetASIC3();
                        theRMS= sil->GetRMS3();
                     }
                     else if( nASIC == 4)
                     {
                        theASIC = sil->GetASIC4();
                        theRMS= sil->GetRMS4();
                     }

                     theASIC[ss] = fabs(strip->GetPedSubADC());
                     theRMS[ss] = strip->GetStripRMS();
                  }
               }
            }
            AlphaEvent->SetNHitsCut(fFlags->gNHitsCut);
            AlphaEvent->SetNClusterSigma(fFlags->nClusterSigma);
            AlphaEvent->SetPClusterSigma(fFlags->pClusterSigma);
            AlphaEvent->SetHitSignificance(fFlags->hitSigmaCut);
            AlphaEvent->SetHitThreshold(fFlags->hitThresholdCut);
            
            AlphaEvent->RecEvent();
            if (fFlags->SaveTAlphaEvent)
               AlphaEventTree->Fill();
         }
      }
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"talphaevent_module");
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("HitModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class AlphaEventModuleFactory: public TAFactory
{
public:
   AlphaEventFlags fFlags;

public:
   void Help()
   {
      printf("AlphaEventModuleFactory::Help!\n");
      printf("\t--nounpack   Turn unpacking of TPC data (turn off reconstruction completely)\n");
   }
   void Usage()
   {
     Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++)
      {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      printf("AlphaEventModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule(runinfo, &fFlags);
   }
};

static TARegister tar(new AlphaEventModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
