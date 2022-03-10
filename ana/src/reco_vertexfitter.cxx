//
// reco_spacepoint_builder.cxx
//
// reconstruction of TPC data
//

#include <stdio.h>
#include <iostream>

#include "manalyzer.h"
#include "midasio.h"

#include "AgFlow.h"
#include "RecoFlow.h"

#include <TTree.h>

#include "TStoreEvent.hh"

#include "AnaSettings.hh"
#include "json.hpp"

#include "TRecoVertexFitter.hh"
#include "TTrackBuilder.hh"

class VertexFitterFlags
{
public:
   bool fRecOff = false; //Turn reconstruction off
   bool fDiag=false;
   bool fTimeCut = false;
   double start_time = -1.;
   double stop_time = -1.;
   double fMagneticField=-1.;
   bool fFieldMap=true;
   bool fEventRangeCut = false;
   int start_event = -1;
   int stop_event = -1;

   finderChoice finder = adaptive;

   bool ffiduc = false;
   
   AnaSettings* ana_settings=0;
public:
   VertexFitterFlags() // ctor
   { }

   ~VertexFitterFlags() // dtor
   { }
};

class VertexFitter: public TARunObject
{
public:
   bool do_plot = false;
   const bool fTrace = false;
   // bool fTrace = true;
   bool fVerb=false;

   VertexFitterFlags* fFlags;

private:
   TRecoVertexFitter vertexFitter;

   unsigned fNhitsCut;
   bool diagnostics;

public:
   TStoreEvent *analyzed_event;
   TTree *EventTree;

   VertexFitter(TARunInfo* runinfo, VertexFitterFlags* f): TARunObject(runinfo),
                                                 fFlags(f),
                                                 vertexFitter( 
                                                    fFlags->ana_settings->GetDouble("RecoModule","VtxChi2Cut"), 
                                                    fTrace
                                                 )

   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="VertexFitter";
#endif
      //MagneticField = fFlags->fMagneticField;
      diagnostics=fFlags->fDiag; // dis/en-able histogramming
      
      assert( fFlags->ana_settings );
      fNhitsCut = fFlags->ana_settings->GetInt("RecoModule","NhitsCut");
        

   }

   ~VertexFitter()
   {
      printf("VertexFitter::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("VertexFitter::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      analyzed_event = new TStoreEvent;
      EventTree = new TTree("StoreEventTree", "StoreEventTree");
      EventTree->Branch("StoredEvent", &analyzed_event, 32000, 0);
      delete analyzed_event;
      analyzed_event=NULL;
   }

   void EndRun(TARunInfo* runinfo)
   {
      printf("VertexFitter::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("VertexFitter::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("VertexFitter::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if( fTrace )
         printf("VertexFitter::AnalyzeFlowEvent, run %d\n", runinfo->fRunNo);

      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      AgEvent* age = ef->fEvent;

      // prepare event to store in TTree
      analyzed_event=new TStoreEvent();
      analyzed_event->Reset();
      analyzed_event->SetEventNumber( age->counter );
      analyzed_event->SetTimeOfEvent( age->time );

      if( fFlags->fRecOff )
         {
            {std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
            EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
            EventTree->Fill();}
            flow = new AgAnalysisFlow(flow, analyzed_event);
            return flow;
         }

      if (fFlags->fTimeCut)
         {
            if (age->time<fFlags->start_time)
               {
                  {std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
                  EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
                  EventTree->Fill();}
                  flow = new AgAnalysisFlow(flow, analyzed_event);
                  return flow;
               }
            if (age->time>fFlags->stop_time)
               {
                  std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);{
                  EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
                  EventTree->Fill();}
                  flow = new AgAnalysisFlow(flow, analyzed_event);
                  return flow;
               }
         }

      if (fFlags->fEventRangeCut)
         {
            if (age->counter<fFlags->start_event)
               {
                  {std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
                  EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
                  EventTree->Fill();}
                  flow = new AgAnalysisFlow(flow, analyzed_event);
                  return flow;
               }
            if (age->counter>fFlags->stop_event)
               {
                  {std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
                  EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
                  EventTree->Fill();}
                  flow = new AgAnalysisFlow(flow, analyzed_event);
                  return flow;
               }
         }
      //     std::cout<<"VertexFitter::Analyze Event # "<<age->counter<<std::endl;

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) 
      {
         delete analyzed_event;
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      if (!SigFlow->fSkipReco)
      {
            TFitVertex theVertex(age->counter);
            //theVertex.SetChi2Cut( fVtxChi2Cut );
            int status;
            {
            #if MINUIT2VERTEXFIT
               // Minuit2 is thread safe... no need to slow things down
            #else
            std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
            #endif
            status = vertexFitter.RecVertex(SigFlow->fHelixArray, &theVertex );
            } 
            if( fTrace )
               std::cout<<"RecoRun::AnalyzeFlowEvent Vertexing Status: "<<status<<std::endl;

            analyzed_event->SetEvent( &SigFlow->fSpacePoints, &SigFlow->fLinesArray, &SigFlow->fHelixArray);
            analyzed_event->SetVertexStatus( status );
            if( status > 0 )
               {
                  analyzed_event->SetVertex(*(theVertex.GetVertex()));
                  analyzed_event->SetUsedHelices(theVertex.GetHelixStack());
                  if( fTrace ) theVertex.Print("rphi");
               }
            else if( fTrace )
               std::cout<<"RecoRun::AnalyzeFlowEvent no vertex found"<<std::endl;
      }
 
      {
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         EventTree->SetBranchAddress("StoredEvent", &analyzed_event);
         EventTree->Fill();
      }
      //Put a copy in the flow for thread safety, now I can safely edit/ delete the local one
      flow = new AgAnalysisFlow(flow, analyzed_event); 
 

      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("VertexFitter::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class VertexFitterFactory: public TAFactory
{
public:
   VertexFitterFlags fFlags;
public:
   void Help()
   {
      printf("VertexFitterFactory::Help\n");
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
      printf("VertexFitterFactory::Init!\n");
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

         if( args[i] == "--anasettings" ) json=args[i+1];
         
         if( args[i] == "--fiduc" ) fFlags.ffiduc = true;
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
      }

      fFlags.ana_settings=new AnaSettings(json.Data());
      //  fFlags.ana_settings->Print();
   }

   void Finish()
   {
      printf("VertexFitterFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("VertexFitterFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new VertexFitter(runinfo,&fFlags);
   }
};


static TARegister tar(new VertexFitterFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
