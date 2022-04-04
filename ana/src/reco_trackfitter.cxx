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

#include "TRecoHelixFitter.hh"
#include "TRecoLineFitter.hh"
#include "TTrackBuilder.hh"

class TrackFitterFlags
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

   const int fThreadNo;
   const int fThreadCount;

public:
   TrackFitterFlags(const int threadNo, const int threadCount):
      fThreadNo(threadNo), fThreadCount(threadCount)
   { }

   ~TrackFitterFlags() // dtor
   { }
};

class TrackFitter: public TARunObject
{
public:
   bool do_plot = false;
   const bool fTrace = false;
   //bool fTrace = true;
   bool fVerb=false;

   TrackFitterFlags* fFlags;

private:
   TRecoHelixFitter helixFitter;
   TRecoLineFitter lineFitter;

   unsigned fNhitsCut;
 
   bool diagnostics;
 
public:

   TrackFitter(TARunInfo* runinfo, TrackFitterFlags* f): TARunObject(runinfo),
                                                 fFlags(f),
                                                 helixFitter( f->ana_settings, fTrace),
                                                 lineFitter(f->ana_settings, fTrace)
   {
#ifdef HAVE_MANALYZER_PROFILER
#ifdef __MINUIT2FIT__
      fModuleName = std::string("TrackFitter (M2) (") +
                    std::to_string(fFlags->fThreadNo) + 
                    std::string("/") + 
                    std::to_string(fFlags->fThreadCount) +
                    std::string(")");
#else
      fModuleName="TrackFitter (M1)";
      //Minuit simply cannot be used during multithreading
      if (fFlags->fThreadNo > 1)
         fModuleName="TrackFitter (M1) [OFF]";
#endif
#endif
      //MagneticField = fFlags->fMagneticField;
      diagnostics=fFlags->fDiag; // dis/en-able histogramming
      
      assert( fFlags->ana_settings );
      fNhitsCut = fFlags->ana_settings->GetInt("RecoModule","NhitsCut");
        

   }

   ~TrackFitter()
   {
      if (fTrace)
         printf("TrackFitter::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("TrackFitter::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("TrackFitter::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("TrackFitter::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("TrackFitter::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if( fTrace )
         printf("TrackFitter::AnalyzeFlowEvent, run %d\n", runinfo->fRunNo);

      AgEventFlow *ef = flow->Find<AgEventFlow>();

      if (!ef || !ef->fEvent)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      AgEvent* age = ef->fEvent;
      if( fFlags->fRecOff )
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      if (fFlags->fTimeCut)
      {
         if (age->time<fFlags->start_time)
         {
          *flags|=TAFlag_SKIP_PROFILE;
            return flow;
         }
         if (age->time>fFlags->stop_time)
         {
             *flags|=TAFlag_SKIP_PROFILE;
            return flow;
         }
      }

      if (fFlags->fEventRangeCut)
      {
         if (age->counter<fFlags->start_event)
         {
            *flags|=TAFlag_SKIP_PROFILE;
            return flow;
         }
         if (age->counter>fFlags->stop_event)
         {
            *flags|=TAFlag_SKIP_PROFILE;
            return flow;
         }
      }
      //     std::cout<<"TrackFitter::Analyze Event # "<<age->counter<<std::endl;

#ifdef __MINUIT2FIT__
//Minuit2 fit can multi thread :)
#else
//Minuit1 cant... so only thread 1 does work
      if ( fFlags->fThreadNo != 1)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
#endif

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) 
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      if ( SigFlow->fSpacePoints.empty() || SigFlow->fSkipReco)
      {
         *flags|=TAFlag_SKIP_PROFILE;
         return flow;
      }
      
#ifdef __MINUIT2FIT__
      //Minuit2 fit
      if ( fFlags->fThreadNo == 1)
      {
         if( fFlags->fMagneticField == 0. )
            SigFlow->fLinesArray.clear();
         SigFlow->fHelixArray.clear();
         
      }
      if( fFlags->fMagneticField == 0. )
      {
         int nlin = lineFitter.FitLine(SigFlow->fTracksArray, SigFlow->fLinesArray,fFlags->fThreadNo,fFlags->fThreadCount);
         std::cout<<"RecoRun Analyze lines count: "<<nlin<<std::endl;
      }
      int nhel = helixFitter.FitHelix(SigFlow->fTracksArray, SigFlow->fHelixArray,fFlags->fThreadNo,fFlags->fThreadCount);
      if( fTrace )
         std::cout<<"RecoRun Analyze helices count: "<<nhel<<std::endl;

      

#else
      //Minuit 1 can't multithread... so we have to hold a global lock
      {
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         if( fFlags->fMagneticField == 0. )
         {
            SigFlow->fLinesArray.clear();
            int nlin = lineFitter.FitLine(SigFlow->fTracksArray, SigFlow->fLinesArray);
            std::cout<<"RecoRun Analyze lines count: "<<nlin<<std::endl;
         }
         SigFlow->fHelixArray.clear();
         int nhel = helixFitter.FitHelix(SigFlow->fTracksArray, SigFlow->fHelixArray);
      }
#endif

      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("TrackFitter::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class TrackFitterFactory: public TAFactory
{
public:
   TrackFitterFlags fFlags;
public:

   void Help()
   {
      printf("TrackFitterFactory::Help\n");
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

   TrackFitterFactory(int threadNo, int totalThreadCount):
      fFlags(threadNo,totalThreadCount)
   {

   }

   void Init(const std::vector<std::string> &args)
   {
      TString json="default";
      printf("TrackFitterFactory::Init!\n");
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
      printf("TrackFitterFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("TrackFitterFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new TrackFitter(runinfo,&fFlags);
   }
};

#if N_TRACK_FIT_THREADS==1
static TARegister tar(new TrackFitterFactory(1,1));
#elif N_TRACK_FIT_THREADS==2
static TARegister tar1(new TrackFitterFactory(1,2));
static TARegister tar2(new TrackFitterFactory(2,2));
#elif N_TRACK_FIT_THREADS==3
static TARegister tar1(new TrackFitterFactory(1,3));
static TARegister tar2(new TrackFitterFactory(2,3));
static TARegister tar3(new TrackFitterFactory(3,3));
#elif N_TRACK_FIT_THREADS==4
static TARegister tar1(new TrackFitterFactory(1,4));
static TARegister tar2(new TrackFitterFactory(2,4));
static TARegister tar3(new TrackFitterFactory(3,4));
static TARegister tar4(new TrackFitterFactory(4,4));
#elif N_TRACK_FIT_THREADS==8
static TARegister tar1(new TrackFitterFactory(1,8));
static TARegister tar2(new TrackFitterFactory(2,8));
static TARegister tar3(new TrackFitterFactory(3,8));
static TARegister tar4(new TrackFitterFactory(4,8));
static TARegister tar5(new TrackFitterFactory(5,8));
static TARegister tar6(new TrackFitterFactory(6,8));
static TARegister tar7(new TrackFitterFactory(7,8));
static TARegister tar8(new TrackFitterFactory(8,8));
#else
#error *Invalid number of threads for TrackFitter*
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
