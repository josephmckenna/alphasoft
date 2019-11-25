//
// reco_module.cxx
//
// reconstruction of TPC data
//

#include <stdio.h>
#include <iostream>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"

#include <TTree.h>
#include "TSeqCollection.h"
// #include <TH1D.h>
// #include <TH2D.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TGraph.h>

#include "TPCconstants.hh"
#include "LookUpTable.hh"
#include "TSpacePoint.hh"
#include "TracksFinder.hh"
#include "AdaptiveFinder.hh"
#include "NeuralFinder.hh"
#include "TTrack.hh"
#include "TFitLine.hh"
#include "TFitHelix.hh"
#include "TFitVertex.hh"

#include "TStoreEvent.hh"

#include "AnalysisTimer.h"
#include "AnaSettings.h"
#include "json.hpp"

#include "Reco.hh"

class RecoRunFlags
{
public:
   bool fRecOff = false; //Turn reconstruction off
   bool fDiag=false;
   bool fTimeCut = false;
   double start_time = -1.;
   double stop_time = -1.;
   bool fEventRangeCut = false;
   int start_event = -1;
   int stop_event = -1;
   double fMagneticField=-1.;
   bool fFieldMap=true;

   double rfudge = 0.;
   double pfudge = 0.;

   bool ffiduc = false;
   //   enum finderChoice { base, adaptive, neural };
   finderChoice finder = adaptive;

   AnaSettings* ana_settings=0;

public:
   RecoRunFlags() // ctor
   { }

   ~RecoRunFlags() // dtor
   { }
};

class RecoRun: public TARunObject
{
public:
   bool do_plot = false;
   bool fTrace = false;
   //bool fTrace = true;

   RecoRunFlags* fFlags;

private:
   Reco r;
   unsigned fNhitsCut;
 
   double f_rfudge;
   double f_pfudge;

   bool diagnostics;

   bool fiducialization; // exclude points in the inhomogeneous field regions
   double z_fid; // region of inhomogeneous field
 
public:
   TStoreEvent *analyzed_event;
   TTree *EventTree;

   RecoRun(TARunInfo* runinfo, RecoRunFlags* f): TARunObject(runinfo),
                                                 fFlags(f),
                                                 r( f->ana_settings, f->fMagneticField)
   {
      printf("RecoRun::ctor!\n");
      //MagneticField = fFlags->fMagneticField;
      diagnostics=fFlags->fDiag; // dis/en-able histogramming
      fiducialization=fFlags->ffiduc;
      z_fid=650.; // mm
      
      assert( fFlags->ana_settings );
      fNhitsCut = fFlags->ana_settings->GetInt("RecoModule","NhitsCut");
        
      if( fabs(fFlags->rfudge) < 1 )
         f_rfudge = 1.+fFlags->rfudge;
      else
         std::cerr<<"RecoRun::RecoRun r fudge factor must be < 1"<<std::endl;

      if( fabs(fFlags->pfudge) < 1 )
         f_pfudge = 1.+fFlags->pfudge;  
      else
         std::cerr<<"RecoRun::RecoRun phi fudge factor must be < 1"<<std::endl;

      r.SetTrace( fTrace );
   }

   ~RecoRun()
   {
      printf("RecoRun::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("RecoRun::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

      if( !fFlags->fFieldMap ) r.UseSTRfromData(runinfo->fRunNo);

      std::cout<<"RecoRun::BeginRun() r fudge factor: "<<f_rfudge<<std::endl;
      std::cout<<"RecoRun::BeginRun() phi fudge factor: "<<f_pfudge<<std::endl;
      r.SetFudgeFactors(f_rfudge,f_pfudge);


      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      analyzed_event = new TStoreEvent;
      EventTree = new TTree("StoreEventTree", "StoreEventTree");
      EventTree->Branch("StoredEvent", &analyzed_event, 32000, 0);
      delete analyzed_event;
      analyzed_event=NULL;

      if( diagnostics ) r.Setup( runinfo->fRoot->fOutputFile );
  
      std::cout<<"RecoRun::BeginRun Saving AnaSettings to rootfile... ";
      runinfo->fRoot->fOutputFile->cd();
      int error_level_save = gErrorIgnoreLevel;
      gErrorIgnoreLevel = kFatal;
      TObjString sett = fFlags->ana_settings->GetSettingsString();
      int bytes_written = gDirectory->WriteTObject(&sett,"ana_settings");
      if( bytes_written > 0 )
         std::cout<<" DONE ("<<bytes_written<<")"<<std::endl;
      else
         std::cout<<" FAILED"<<std::endl;
      gErrorIgnoreLevel = error_level_save;
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("RecoRun::EndRun, run %d\n", runinfo->fRunNo);
      r.PrintPattRec();
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("RecoRun::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("RecoRun::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if( fTrace )
         printf("RecoRun::Analyze, run %d\n", runinfo->fRunNo);

      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
         return flow;

      AgEvent* age = ef->fEvent;

      // prepare event to store in TTree
      analyzed_event=new TStoreEvent();
      analyzed_event->Reset();
      analyzed_event->SetEventNumber( age->counter );
      analyzed_event->SetTimeOfEvent( age->time );

      if( fFlags->fRecOff )
         {
            EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
            EventTree->Fill();
            flow = new AgAnalysisFlow(flow, analyzed_event);
            return flow;
         }

      if (fFlags->fTimeCut)
         {
            if (age->time<fFlags->start_time)
               {
                  EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
                  EventTree->Fill();
                  flow = new AgAnalysisFlow(flow, analyzed_event);
                  return flow;
               }
            if (age->time>fFlags->stop_time)
               {
                  EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
                  EventTree->Fill();
                  flow = new AgAnalysisFlow(flow, analyzed_event);
                  return flow;
               }
         }

      if (fFlags->fEventRangeCut)
         {
            if (age->counter<fFlags->start_event)
               {
                  EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
                  EventTree->Fill();
                  flow = new AgAnalysisFlow(flow, analyzed_event);
                  return flow;
               }
            if (age->counter>fFlags->stop_event)
               {
                  EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
                  EventTree->Fill();
                  flow = new AgAnalysisFlow(flow, analyzed_event);
                  return flow;
               }
         }
      #ifdef _TIME_ANALYSIS_
      START_TIMER
      #endif   

      std::cout<<"RecoRun::Analyze Event # "<<age->counter<<std::endl;

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) 
      {
         delete analyzed_event;
         return flow;
      }
      
      bool skip_reco=false;
      if( fTrace )
         {
            int AW,PAD,SP=-1;
            AW=PAD=SP;
            if (SigFlow->awSig) AW=int(SigFlow->awSig->size());
            printf("RecoModule::Analyze, AW # signals %d\n", AW);
            if (SigFlow->pdSig) PAD=int(SigFlow->pdSig->size());
            printf("RecoModule::Analyze, PAD # signals %d\n", PAD);
            if (SigFlow->matchSig) SP=int(SigFlow->matchSig->size());
            printf("RecoModule::Analyze, SP # %d\n", SP);
         }
      if (!SigFlow->matchSig)
      {
          std::cout<<"RecoRun::No matched hits"<<std::endl;
          skip_reco=true;
#ifdef _TIME_ANALYSIS_
            if (TimeModules) flow=new AgAnalysisReportFlow(flow,"reco_module(no matched hits)",timer_start);
#endif
      }
      else if( SigFlow->matchSig->size() > fNhitsCut )
         {
            std::cout<<"RecoRun::Analyze Too Many Points... quitting"<<std::endl;
            skip_reco=true;
#ifdef _TIME_ANALYSIS_
            if (TimeModules) flow=new AgAnalysisReportFlow(flow,"reco_module(too many hits)",timer_start);
#endif
         }

      if (!skip_reco)
         {
            //Root's fitting routines are often not thread safe
#ifdef MODULE_MULTITHREAD
            std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
#endif
      
            if( !fiducialization )
               r.AddSpacePoint( SigFlow->matchSig );
            else
               r.AddSpacePoint( SigFlow->matchSig, z_fid );
      
            printf("RecoRun::Analyze  Points: %d\n",r.GetNumberOfPoints());

            r.FindTracks(fFlags->finder);
            printf("RecoRun::Analyze  Tracks: %d\n",r.GetNumberOfTracks());
#ifdef _TIME_ANALYSIS_
            if (TimeModules) flow=new Ag2DAnalysisReportFlow(flow,
                                                           {"reco_module(AdaptiveFinder)","Points in track"," # Tracks"},
                                                           {(double)r.GetNumberOfPoints(),(double)r.GetNumberOfTracks()},timer_start);
            timer_start=CLOCK_NOW
#endif
            if( fFlags->fMagneticField == 0. )
               {
                  int nlin = r.FitLines();
                  std::cout<<"RecoRun Analyze lines count: "<<nlin<<std::endl;
               }

            int nhel = r.FitHelix();
            std::cout<<"RecoRun Analyze helices count: "<<nhel<<std::endl;

            TFitVertex theVertex(age->counter);
            //theVertex.SetChi2Cut( fVtxChi2Cut );
            int status = r.RecVertex( &theVertex );
            std::cout<<"RecoRun Analyze Vertexing Status: "<<status<<std::endl;

            analyzed_event->SetEvent(r.GetPoints(),r.GetLines(),r.GetHelices());
            analyzed_event->SetVertexStatus( status );
            if( status > 0 )
               {
                  analyzed_event->SetVertex(*(theVertex.GetVertex()));
                  analyzed_event->SetUsedHelices(theVertex.GetHelixStack());
                  theVertex.Print("rphi");
               }
            else
               std::cout<<"RecoRun Analyze no vertex found"<<std::endl;
         }
      
      AgBarEventFlow *bf = flow->Find<AgBarEventFlow>();
      //If have barrel scintilator, add to TStoreEvent
      if (bf)
         {
            //bf->BarEvent->Print();
            analyzed_event->AddBarrelHits(bf->BarEvent);
         }

      EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
      EventTree->Fill();
      //Put a copy in the flow for thread safety, now I can safely edit/ delete the local one
      flow = new AgAnalysisFlow(flow, analyzed_event); 
 
      std::cout<<"\tRecoRun Analyze EVENT "<<age->counter<<" ANALYZED"<<std::endl;
#ifdef _TIME_ANALYSIS_
      if (TimeModules) flow=new AgAnalysisReportFlow(flow,"reco_module",timer_start);
#endif
      if (!skip_reco) r.Reset();
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("RecoRun::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class RecoModuleFactory: public TAFactory
{
public:
   RecoRunFlags fFlags;
public:
   void Help()
   {
      printf("RecoModuleFactory::Help\n");
      printf("\t--usetimerange 123.4 567.8\t\tLimit reconstruction to a time range\n");
      printf("\t--useeventrange 123 456\t\tLimit reconstruction to an event range\n");
      //      printf("\t--Bmap xx\t\tSet STR using Babcock Map OBSOLETE!!! This is now default\n");
      printf("\t--Bfield 0.1234\t\t set magnetic field value in Tesla\n");
      printf("\t--loadcalib\t\t Load calibration STR file made by this analysis\n");
      printf("\t--recoff\t\t disable reconstruction\n");
      printf("\t--diag\t\t enable histogramming\n");
      printf("\t--anasettings /path/to/settings.json\t\t load the specified analysis settings\n");
      printf("\t--rfudge 0.12\t\t Fudge or alter the STR radius by a fraction\n");
      printf("\t--pfudge 0.12\t\t Fudge or alter the STR azimuth by a fraction\n");
      printf("\t--fiduc\t\t skip over points in the inhomogenous field region\n");
   }
   void Usage()
   {
      Help();
   }
   void Init(const std::vector<std::string> &args)
   {
      TString json="default";
      printf("RecoModuleFactory::Init!\n");
      for (unsigned i=0; i<args.size(); i++) {
         if( args[i]=="-h" || args[i]=="--help" )
            Help();
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
         if( args[i] == "--Bfield" )
            {
               fFlags.fMagneticField = atof(args[++i].c_str());
               printf("Magnetic Field (incompatible with --loadcalib)\n");
            }
         if (args[i] == "--loadcalib")
            {
               fFlags.fFieldMap = false;
               fFlags.fMagneticField = 0.; // data driven STR valid only for B=0T
               printf("Attempting to use calibrated timing for reconstruction\n");
            }
         if (args[i] == "--recoff")
            fFlags.fRecOff = true;
         if( args[i] == "--diag" )
            fFlags.fDiag = true;
         if( args[i] == "--finder" ){
            std::string findString = args[++i];
            // if(findString == "base") fFlags.finder = RecoRunFlags::base;
            // else if(findString == "neural") fFlags.finder = RecoRunFlags::neural;
            // else if(findString == "adaptive") fFlags.finder = RecoRunFlags::adaptive;
            if(findString == "base") fFlags.finder = base;
            else if(findString == "neural") fFlags.finder = neural;
            else if(findString == "adaptive") fFlags.finder = adaptive;
            else {
               std::cerr << "Unknown track finder mode \"" << findString << "\", using adaptive" << std::endl;
            }
         }

         if( args[i] == "--anasettings" ) json=args[i+1];

         if( args[i] == "--rfudge" ) fFlags.rfudge = atof(args[i+1].c_str());
         if( args[i] == "--pfudge" ) fFlags.pfudge = atof(args[i+1].c_str());
         
         if( args[i] == "--fiduc" ) fFlags.ffiduc = true;
         
         if( args[i] == "--anasettings" ) json=args[++i];
      }

      fFlags.ana_settings=new AnaSettings(json.Data());
      fFlags.ana_settings->Print();
   }

   void Finish()
   {
      printf("RecoModuleFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("RecoModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new RecoRun(runinfo,&fFlags);
   }
};


static TARegister tar(new RecoModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
