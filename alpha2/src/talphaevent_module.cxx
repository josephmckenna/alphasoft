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
#include "ALPHA2SettingsDatabase.h"

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

#include "TAlphaEventMap.h"

#define MAX_CHANNELS VF48_MAX_CHANNELS // defined in UnpackVF48.h
#define NUM_SI_MODULES nSil // defined in SiMod.h
#define NUM_VF48_MODULES nVF48 // defined in SiMod.h
#define NUM_SI_ALPHA1 nSiAlpha1 // defined in SiMod.h
#define NUM_SI_ALPHA2 nSiAlpha2 // defined in SiMod.h

#define VF48_COINCTIME 0.000010

// Not ready yet! It works but the TSiliconEvent and TAlphaEvent might need preparing for recycling
PointerRecycler<TSiliconEvent> SilEventFlow::gTSiliconEventRecycleBin(0,"TSiliconEvent");
PointerRecycler<TAlphaEvent> SilEventFlow::gTAlphaEventRecycleBin(0,"TAlphaEvent");

TVF48SiMap *gVF48SiMap = NULL;

class AlphaEventFlags
{
public:
   bool fPrint = false;
   bool SaveTAlphaEvent = false;
   bool SaveTSiliconEvent = false;
   
   int gNHitsCut = 200;
   double nClusterSigma = 3.5;//nVASigma;
   double pClusterSigma = 6;//pVASigma;
   double hitSigmaCut = 0.;//nVASigma;
   double hitThresholdCut = 99999;
   
   int ImproveVertexInteration=-1;
};

class AlphaEventModule: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   bool fTrace = false;
   TAlphaEventMap* fAlphaEventMap;
   AlphaEventModule(TARunInfo* runinfo, AlphaEventFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent Module";
#endif
      if (fTrace)
         printf("AlphaEventModule::ctor!\n");
      fAlphaEventMap = NULL;
      gVF48SiMap = NULL;
   }

   ~AlphaEventModule()
   {
      if (fTrace)
         printf("AlphaEventModule::dtor!\n");

   }

   void BeginRun(TARunInfo* runinfo)
   {

      if (fTrace)
         printf("HitModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

      // load the sqlite3 db
      TSettings *SettingsDB = ALPHA2SettingsDatabase::GetTSettings(runinfo->fRunNo);
      if (!runinfo->fRunNo)
         return;

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
       delete detectorXML;

       delete SettingsDB;


      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      fAlphaEventMap=new TAlphaEventMap();


      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void PreEndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("HitModule::PreEndRun, run %d\n", runinfo->fRunNo);
      //printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("HitModule::EndRun, run %d\n", runinfo->fRunNo);
      if (gVF48SiMap)
      {
         delete gVF48SiMap;
         gVF48SiMap = NULL;
      }
      if (fAlphaEventMap)
      {
         delete fAlphaEventMap;
         fAlphaEventMap = NULL;
      }
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
   
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TSiliconEvent* SiliconEvent=fe->silevent;
      TAlphaEvent* AlphaEvent=new TAlphaEvent(fAlphaEventMap);
      AlphaEvent->DeleteEvent();
      AlphaEvent->SetNHitsCut(fFlags->gNHitsCut);
      AlphaEvent->SetNClusterSigma(fFlags->nClusterSigma);
      AlphaEvent->SetPClusterSigma(fFlags->pClusterSigma);
      AlphaEvent->SetHitSignificance(fFlags->hitSigmaCut);
      AlphaEvent->SetHitThreshold(fFlags->hitThresholdCut);
      if( AlphaEvent )
      {
         int m, c, ttcchannel;
         for( int isil = 0; isil < NUM_SI_MODULES; isil++ )
         {
            TSiliconModule * module = SiliconEvent->GetSiliconModule( isil );
            if( !module ) continue;

            gVF48SiMap->GetVF48( isil,1, m, c, ttcchannel); // check that the mapping exists
            if( m == -1 ) continue; // if not, continue

            const std::string& name = gVF48SiMap->GetSilName(isil);
            TAlphaEventSil *sil = new TAlphaEventSil((Char_t*) name.c_str(),AlphaEvent,fAlphaEventMap);

            AlphaEvent->AddSil(sil);
            for( int iASIC = 1; iASIC <= 4; iASIC++ ) 
            {
               gVF48SiMap->GetVF48( isil,iASIC, m, c, ttcchannel);
               TSiliconVA * asic = module->GetASIC( iASIC );
               if( !asic ) continue;

               Int_t nASIC  = asic->GetASICNumber();

               for(uint s = 0; s<128; s++)
               {
                  
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
                  
                  if (asic->RawADC[s]<-1024) continue;
                  theASIC[s] = fabs(asic->PedSubADC[s]);
                  theRMS[s] = asic->stripRMS[s];
               }
            }
         }

         //AlphaEvent is prepared... put it into the flow
         fe->alphaevent=AlphaEvent;
      }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("HitModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class AlphaEventModule_cluster: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   AlphaEventModule_cluster(TARunInfo* runinfo, AlphaEventFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent Cluster";
#endif
      fFlags=flags;
   }
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TAlphaEvent* AlphaEvent=fe->alphaevent;
      AlphaEvent->RecClusters();
      return flow;
   }
};
class AlphaEventModule_hits: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   AlphaEventModule_hits(TARunInfo* runinfo, AlphaEventFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent hits";
#endif
      fFlags=flags;
   }
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TAlphaEvent* AlphaEvent=fe->alphaevent;
      AlphaEvent->RecHits();
      return flow;
   }
};

class AlphaEventModule_gettracks: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   int stride;
   int offset;
   AlphaEventModule_gettracks(TARunInfo* runinfo, AlphaEventFlags* flags, int s, int o)
     : TARunObject(runinfo), fFlags(flags)
   {
      stride=s;
      offset=o;
      fFlags=flags;
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent GetTracks("+std::to_string(offset)+"/"+std::to_string(stride)+")";
#endif
   }
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TAlphaEvent* AlphaEvent=fe->alphaevent;
      AlphaEvent->GatherTrackCandidates(stride, offset);
      return flow;
   }
};


class AlphaEventModule_tracks: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   AlphaEventModule_tracks(TARunInfo* runinfo, AlphaEventFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent Make Helicies";
#endif
      fFlags=flags;
   }
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TAlphaEvent* AlphaEvent=fe->alphaevent;
      //std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      AlphaEvent->RecTrackCandidates();
      return flow;
   }
};
class AlphaEventModule_fittracks: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   int stride;
   int offset;
   
   AlphaEventModule_fittracks(TARunInfo* runinfo, AlphaEventFlags* flags, int s, int o)
     : TARunObject(runinfo), fFlags(flags)
   {
      stride=s;
      offset=o;
      fFlags=flags;
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent FitTracks("+std::to_string(offset)+"/"+std::to_string(stride)+")";
#endif
   }
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TAlphaEvent* AlphaEvent=fe->alphaevent;
      //std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      AlphaEvent->FitTrackCandidates(stride,offset);
      return flow;
   }
};


class AlphaEventModule_prunetracks: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   AlphaEventModule_prunetracks(TARunInfo* runinfo, AlphaEventFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent PruneTracks";
#endif
      fFlags=flags;
   }
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TAlphaEvent* AlphaEvent=fe->alphaevent;
      AlphaEvent->PruneTracks();
      return flow;
   }
};
class AlphaEventModule_vertex: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   AlphaEventModule_vertex(TARunInfo* runinfo, AlphaEventFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent Vertex";
#endif
      fFlags=flags;
   }
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TAlphaEvent* AlphaEvent=fe->alphaevent;
      //std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      AlphaEvent->RecVertex();
      return flow;
   }
};
class AlphaEventModule_improvevertexonce: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   const int stride;
   const int offset;
   AlphaEventModule_improvevertexonce(TARunInfo* runinfo, AlphaEventFlags* flags, int s, int o)
     : TARunObject(runinfo), fFlags(flags), stride(s), offset(o)
   {
      fFlags=flags;
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent Improve Vert"+std::to_string(fFlags->ImproveVertexInteration);
      if (stride)
      {
         fModuleName+="(";
         fModuleName+=std::to_string(offset);
         fModuleName+="/";
         fModuleName+=std::to_string(stride);
         fModuleName+=")";
      }
#endif
   }
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TAlphaEvent* AlphaEvent=fe->alphaevent;
      //std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      if (AlphaEvent->HasVertexStoppedImproving())
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      if (AlphaEvent->GetVertex()->GetNHelices()<3) 
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      if (offset!=stride || stride==0)
         AlphaEvent->ImproveVertexOnce(stride, offset);
      //Final loop
      if (offset==stride)
         AlphaEvent->ChooseImprovedVertex();
      return flow;
   }
};
class AlphaEventModule_improvevertex: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   AlphaEventModule_improvevertex(TARunInfo* runinfo, AlphaEventFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent ImproveVertex More";
#endif
      fFlags=flags;
   }
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TAlphaEvent* AlphaEvent=fe->alphaevent;
      //std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      if (AlphaEvent->HasVertexStoppedImproving())
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      if (AlphaEvent->GetVertex()->GetNHelices()<3)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      AlphaEvent->ImproveVertex();
      return flow;
   }
};
class AlphaEventModule_Rphi: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   AlphaEventModule_Rphi(TARunInfo* runinfo, AlphaEventFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent RPhi";
#endif
      fFlags=flags;
   }
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TAlphaEvent* AlphaEvent=fe->alphaevent;
      AlphaEvent->RecRPhi();
      return flow;
   }
};
class AlphaEventModule_GoodHel: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   AlphaEventModule_GoodHel(TARunInfo* runinfo, AlphaEventFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent GoodHel";
#endif
      fFlags=flags;
   }
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (!fe)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      TAlphaEvent* AlphaEvent=fe->alphaevent;
      AlphaEvent->CalcGoodHelices();
      return flow;
   }
};

class AlphaEventModule_save: public TARunObject
{
public:
   AlphaEventFlags* fFlags = NULL;
   TTree* AlphaEventTree   = NULL;
   TAlphaEvent* AlphaEvent = NULL;
   TTree* SiliconTree      = NULL;
   AlphaEventModule_save(TARunInfo* runinfo, AlphaEventFlags* flags)
     : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="TAlphaEvent Save";
#endif
      if (fFlags->SaveTAlphaEvent)
      {
         //AlphaEventTree = new TTree("gAlphaEventTree","Alpha Event Tree");
         
         //AlphaEventTree->Branch("AlphaEvent","TAlphaEvent",&AlphaEvent,16000,1);
      }
   }
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SilEventFlow* sf=flow->Find<SilEventFlow>();
      if (!sf)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      AlphaEvent=sf->alphaevent;
      if (fFlags->SaveTAlphaEvent)
      {
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         runinfo->fRoot->fOutputFile->cd();
         if (!AlphaEventTree)
            AlphaEventTree = new TTree("gAlphaEventTree","Alpha Event Tree");
         TBranch* b_variable = AlphaEventTree->GetBranch("AlphaEvent");
         if (!b_variable)
            AlphaEventTree->Branch("AlphaEvent","TAlphaEvent",&AlphaEvent,16000,1);
         else
            AlphaEventTree->SetBranchAddress("AlphaEvent",&AlphaEvent);
         AlphaEventTree->Fill();
      }

      TSiliconEvent* SiliconEvent=sf->silevent;
      
      //Record hits
      for( int isil = 0; isil < NUM_SI_MODULES; isil++ )
      {
         TAlphaEventSil * sil = (TAlphaEventSil*)AlphaEvent->GetSilByNumber( isil,true);
         if(!sil) continue;
         SiliconEvent->SetNHits( SiliconEvent->GetNHits() + sil->GetNHits() );
      }
      //Record tracks
      Int_t Ntracks=AlphaEvent->GetNGoodHelices() ;
      SiliconEvent->SetNTracks( Ntracks );
      //Record vertex
      TAlphaEventVertex * vertex = (TAlphaEventVertex*)AlphaEvent->GetVertex();
      if( vertex->IsGood() )
      {
         TVector3 v;
         v.SetXYZ( vertex->X(), vertex->Y(), vertex->Z() );
         SiliconEvent->SetVertex( &v );
         SiliconEvent->SetNVertices( 1 );
      }
      SiliconEvent->SetResidual( AlphaEvent->CosmicTest());
      SiliconEvent->ApplyCuts();
      
      if (fFlags->SaveTSiliconEvent)
      {
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         runinfo->fRoot->fOutputFile->cd();
         
         if (!SiliconTree)
            SiliconTree = new TTree("gSiliconTree","Silicon Tree");
         TBranch* b_variable =SiliconTree->GetBranch("SiliconEvent");
         if (!b_variable)
            SiliconTree->Branch("SiliconEvent","TSiliconEvent",&SiliconEvent,16000,1);
         else
            SiliconTree->SetBranchAddress("SiliconEvent",&SiliconEvent);
         SiliconTree->Fill();
      }
      //SiliconEvent->Print();
      return flow;
   }
};
class AlphaEventModuleFactory: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   void Help()
   {
      printf("AlphaEventModuleFactory::Help!\n");
      printf("\t--nounpack\t\tTurn unpacking of TPC data (turn off reconstruction completely)\n");
      printf("\t--nClusterSigma\t\tSet cluster sigma threshold (default:%f)\n",fFlags.nClusterSigma);
      printf("\t--pClusterSigma\t\tSet cluster sigma threshold (default:%f)\n",fFlags.pClusterSigma);
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
         if (args[i] == "--nClusterSigma")
            fFlags.nClusterSigma = atof(args[++i].c_str());
         if (args[i] == "--pClusterSigma")
            fFlags.pClusterSigma = atof(args[++i].c_str());
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("AlphaEventModuleFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule(runinfo, &fFlags);
   }
};

class AlphaEventModuleFactory_cluster: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory_cluster::Init!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_cluster::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule_cluster(runinfo, &fFlags);
   }
};
class AlphaEventModuleFactory_hits: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory_hits::Init!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_hits::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule_hits(runinfo, &fFlags);
   }
};
class AlphaEventModuleFactory_gettracks: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   //Multithread settings
   int stride=0;
   int offset=0;
   AlphaEventModuleFactory_gettracks(int s, int o)
   {
      stride=s;
      offset=o;
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory_gettracks(%d/%d)::Init!\n",offset,stride);
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_gettracks::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule_gettracks(runinfo, &fFlags, stride, offset);
   }
};
class AlphaEventModuleFactory_tracks: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory_tracks::Init!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_tracks::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule_tracks(runinfo, &fFlags);
   }
};
class AlphaEventModuleFactory_fittracks: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   //Multithread settings
   int stride=0;
   int offset=0;
   AlphaEventModuleFactory_fittracks(int s, int o)
   {
      stride=s;
      offset=o;
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory_fittracks(%d/%d)::Init!\n",offset,stride);
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_fittracks::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule_fittracks(runinfo, &fFlags, stride, offset);
   }
};
class AlphaEventModuleFactory_prunetracks: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory_prunetracks::Init!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_prunetracks::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule_prunetracks(runinfo, &fFlags);
   }
};
class AlphaEventModuleFactory_vertex: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory_vertex::Init!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_vertex::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule_vertex(runinfo, &fFlags);
   }
};
class AlphaEventModuleFactory_improvevertex: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory_improvevertex::Init!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_improvevertex::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule_improvevertex(runinfo, &fFlags);
   }
};
class AlphaEventModuleFactory_improvevertex_1: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   int stride=0;
   int offset=0;
   AlphaEventModuleFactory_improvevertex_1(int pass, int s=0, int o=0)
   {
      fFlags.ImproveVertexInteration=pass;
      stride=s;
      offset=o;
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory_improvevertex_1(%d)::Init!\n",fFlags.ImproveVertexInteration);
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_improvevertex::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule_improvevertexonce(runinfo, &fFlags, stride, offset);
   }
};

class AlphaEventModuleFactory_Rphi: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory_Rphi::Init!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_Rphi::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule_Rphi(runinfo, &fFlags);
   }
};
class AlphaEventModuleFactory_GoodHel: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory_GoodHel::Init!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_GoodHel::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule_GoodHel(runinfo, &fFlags);
   }
};

class AlphaEventModuleFactory_save: public TAFactory
{
public:
   AlphaEventFlags fFlags;
   void Init(const std::vector<std::string> &args)
   {
      printf("AlphaEventModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++)
      {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--alphaevent")
            fFlags.SaveTAlphaEvent = true;
         if (args[i] == "--silevent")
            fFlags.SaveTSiliconEvent = true;

      }
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AlphaEventModuleFactory_save::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AlphaEventModule_save(runinfo, &fFlags);
   }
};

static TARegister tar1(new AlphaEventModuleFactory);
static TARegister tar2(new AlphaEventModuleFactory_cluster);
static TARegister tar3(new AlphaEventModuleFactory_hits);
#ifdef NO_EXTRA_MT
//Gather tracks over 1 thread:
static TARegister tar4a(new AlphaEventModuleFactory_gettracks(0,0));
#else
//Gather tracks over 8 threads:
static TARegister tar4a(new AlphaEventModuleFactory_gettracks(8,0));
static TARegister tar4b(new AlphaEventModuleFactory_gettracks(8,1));
static TARegister tar4c(new AlphaEventModuleFactory_gettracks(8,2));
static TARegister tar4d(new AlphaEventModuleFactory_gettracks(8,3));
static TARegister tar4f(new AlphaEventModuleFactory_gettracks(8,4));
static TARegister tar4g(new AlphaEventModuleFactory_gettracks(8,5));
static TARegister tar4h(new AlphaEventModuleFactory_gettracks(8,6));
static TARegister tar4i(new AlphaEventModuleFactory_gettracks(8,7));
#endif
static TARegister tar5(new AlphaEventModuleFactory_tracks);
#ifdef NO_EXTRA_MT
//Fit tracks over 1 thread:
static TARegister tar6h(new AlphaEventModuleFactory_fittracks(0,0));
#else
//Fit tracks over 8 threads:
static TARegister tar6a(new AlphaEventModuleFactory_fittracks(8,0));
static TARegister tar6b(new AlphaEventModuleFactory_fittracks(8,1));
static TARegister tar6c(new AlphaEventModuleFactory_fittracks(8,2));
static TARegister tar6d(new AlphaEventModuleFactory_fittracks(8,3));
static TARegister tar6e(new AlphaEventModuleFactory_fittracks(8,4));
static TARegister tar6f(new AlphaEventModuleFactory_fittracks(8,5));
static TARegister tar6g(new AlphaEventModuleFactory_fittracks(8,6));
static TARegister tar6h(new AlphaEventModuleFactory_fittracks(8,7));
#endif
static TARegister tar7(new AlphaEventModuleFactory_prunetracks);
static TARegister tar8(new AlphaEventModuleFactory_vertex);
#ifdef NO_EXTRA_MT
//Improve vertex with 1 thread
static TARegister tar9aa(new AlphaEventModuleFactory_improvevertex_1(1,0,0));
#else
//Improve vertex with 8 threads
static TARegister tar9aa(new AlphaEventModuleFactory_improvevertex_1(1,7,0));
static TARegister tar9ab(new AlphaEventModuleFactory_improvevertex_1(1,7,1));
static TARegister tar9ac(new AlphaEventModuleFactory_improvevertex_1(1,7,2));
static TARegister tar9ad(new AlphaEventModuleFactory_improvevertex_1(1,7,3));
static TARegister tar9ae(new AlphaEventModuleFactory_improvevertex_1(1,7,4));
static TARegister tar9af(new AlphaEventModuleFactory_improvevertex_1(1,7,5));
static TARegister tar9ag(new AlphaEventModuleFactory_improvevertex_1(1,7,6));
static TARegister tar9ah(new AlphaEventModuleFactory_improvevertex_1(1,7,7));
#endif
static TARegister tar9b(new AlphaEventModuleFactory_improvevertex_1(2));
static TARegister tar9c(new AlphaEventModuleFactory_improvevertex_1(3));
static TARegister tar9d(new AlphaEventModuleFactory_improvevertex);
static TARegister tar10(new AlphaEventModuleFactory_Rphi);
static TARegister tar11(new AlphaEventModuleFactory_GoodHel);
static TARegister tar12(new AlphaEventModuleFactory_save);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
