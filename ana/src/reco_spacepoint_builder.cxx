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

#include "TSpacePointBuilder.hh"

class SpacePointBuilderFlags
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
   
   AnaSettings* ana_settings=0;

   std::string fLocation="CERN";

public:
   SpacePointBuilderFlags() // ctor
   { }

   ~SpacePointBuilderFlags() // dtor
   { }
};

class SpacePointBuilder: public TARunObject
{
public:
   bool do_plot = false;
   const bool fTrace = false;
   // bool fTrace = true;
   bool fVerb=false;

   SpacePointBuilderFlags* fFlags;

private:
   TSpacePointBuilder r;
   unsigned fNhitsCut;
 
   double f_rfudge;
   double f_pfudge;

   bool diagnostics;

   bool fiducialization; // exclude points in the inhomogeneous field regions
   double z_fid; // region of inhomogeneous field
 
public:
   TStoreEvent *analyzed_event;
   TTree *EventTree;

   SpacePointBuilder(TARunInfo* runinfo, SpacePointBuilderFlags* f): TARunObject(runinfo),
                                                 fFlags(f),
                                                 r( f->ana_settings, f->fMagneticField,  
                                                    f->fLocation, fTrace)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="SpacePointBuilder";
#endif
      //MagneticField = fFlags->fMagneticField;
      diagnostics=fFlags->fDiag; // dis/en-able histogramming
      fiducialization=fFlags->ffiduc;
      z_fid=650.; // mm
      
      assert( fFlags->ana_settings );
      fNhitsCut = fFlags->ana_settings->GetInt("RecoModule","NhitsCut");
        
      if( fabs(fFlags->rfudge) < 1 )
         f_rfudge = 1.+fFlags->rfudge;
      else
         std::cerr<<"SpacePointBuilder::SpacePointBuilder r fudge factor must be < 1"<<std::endl;

      if( fabs(fFlags->pfudge) < 1 )
         f_pfudge = 1.+fFlags->pfudge;  
      else
         std::cerr<<"SpacePointBuilder::SpacePointBuilder phi fudge factor must be < 1"<<std::endl;

   }

   ~SpacePointBuilder()
   {
      printf("SpacePointBuilder::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      printf("SpacePointBuilder::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());

      if( !fFlags->fFieldMap ) r.UseSTRfromData(runinfo->fRunNo);

      std::cout<<"SpacePointBuilder::BeginRun() r fudge factor: "<<f_rfudge<<std::endl;
      std::cout<<"SpacePointBuilder::BeginRun() phi fudge factor: "<<f_pfudge<<std::endl;
      r.SetFudgeFactors(f_rfudge,f_pfudge);


      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

      analyzed_event = new TStoreEvent;
      EventTree = new TTree("StoreEventTree", "StoreEventTree");
      EventTree->Branch("StoredEvent", &analyzed_event, 32000, 0);
      delete analyzed_event;
      analyzed_event=NULL;
  
      std::cout<<"SpacePointBuilder::BeginRun Saving AnaSettings to rootfile... ";
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
      printf("SpacePointBuilder::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      printf("SpacePointBuilder::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      printf("SpacePointBuilder::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if( fTrace )
         printf("SpacePointBuilder::AnalyzeFlowEvent, run %d\n", runinfo->fRunNo);

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
      //     std::cout<<"SpacePointBuilder::Analyze Event # "<<age->counter<<std::endl;

      AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      if( !SigFlow ) 
      {
         delete analyzed_event;
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      
      bool skip_reco=false;
      if( fTrace )
         {
            int AW,PAD,SP=-1;
            AW=PAD=SP;
            if (SigFlow->awSig.size()) AW=int(SigFlow->awSig.size());
            printf("RecoModule::AnalyzeFlowEvent, AW # signals %d\n", AW);
            if (SigFlow->pdSig.size()) PAD=int(SigFlow->pdSig.size());
            printf("RecoModule::AnalyzeFlowEvent, PAD # signals %d\n", PAD);
            if (SigFlow->matchSig.size()) SP=int(SigFlow->matchSig.size());
            printf("RecoModule::AnalyzeFlowEvent, SP # %d\n", SP);
         }
     

      if (!skip_reco)
         {
            SigFlow->fSpacePoints.clear();
            if( !fiducialization )
               r.BuildSpacePointArray( SigFlow->matchSig , SigFlow->fSpacePoints);
            else
               r.BuildSpacePointArray( SigFlow->matchSig, SigFlow->fSpacePoints, z_fid );

            if( fTrace )
               printf("SpacePointBuilder::Analyze  Points: %ld\n",SigFlow->fSpacePoints.size());
         }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      printf("SpacePointBuilder::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class SpacePointBuilderFactory: public TAFactory
{
public:
   SpacePointBuilderFlags fFlags;
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


         if( args[i] == "--anasettings" ) json=args[i+1];

         if( args[i] == "--rfudge" ) fFlags.rfudge = atof(args[i+1].c_str());
         if( args[i] == "--pfudge" ) fFlags.pfudge = atof(args[i+1].c_str());
         
         if( args[i] == "--fiduc" ) fFlags.ffiduc = true;
         
         if( args[i] == "--location" ) fFlags.fLocation=args[i+1];
      }

      fFlags.ana_settings=new AnaSettings(json.Data());
      //  fFlags.ana_settings->Print();
   }

   void Finish()
   {
      printf("RecoModuleFactory::Finish!\n");
   }
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("RecoModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new SpacePointBuilder(runinfo,&fFlags);
   }
};


static TARegister tar(new SpacePointBuilderFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
