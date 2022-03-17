//
// AnalysisReport_module.cxx
//
// AnalysisReport of all modules
//
// Joseph McKenna
//

#include <stdio.h>


#include "manalyzer.h"
#include "midasio.h"


#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"

#ifdef BUILD_AG
#include "RecoFlow.h"
#endif

#include "TAGAnalysisReport.h"

//I am intentionally global, external modules test this
bool TimeModules=true;


class AnalysisReportFlags
{
public:
   bool fPrint = false;

   TAGAnalysisReport* AnalysisReport = NULL;

   bool fSaveHistograms = false;

   double sum_aw=0;       //Results from deconv module
   double sum_pad=0;      //Results from deconv module
   double sum_match=0;    //Results from match module
   double sum_tracks=0;   //Results from reco module
   double sum_r_sigma=0;  //Results from reco module
   double sum_z_sigma=0;  //Results from reco module
   double sum_verts=0;    //Results from reco module
   double sum_hits=0;     //Results from reco module
   double sum_bars=0;     //Results from reco module
   
#ifdef BUILD_AG
   void FillTPC(TStoreEvent* e)
   {
      if (e->GetNumberOfTracks()>0)
      {
         sum_tracks  += e->GetNumberOfTracks();
         //if (std::isfinite(e->GetMeanZSigma())) 
         sum_z_sigma += e->GetMeanZSigma();
         //if (std::isfinite(e->GetMeanRSigma())) 
         sum_r_sigma += e->GetMeanRSigma();
      }
      if (e->GetVertexStatus()>0)
         sum_verts += 1;
      if (e->GetNumberOfPoints()>0)
         sum_hits  += e->GetNumberOfPoints();
      if (e->GetBarMultiplicity()>0)
         sum_bars  += e->GetBarMultiplicity();
      AnalysisReport->SetLastTPCTime( e->GetTimeOfEvent() );
      AnalysisReport->IncrementStoreEvents();
   }
#endif
#ifdef BUILD_AG
   void FillTPCSigFlow(AgSignalsFlow* SigFlow)
   {
      if (SigFlow->awSig.size())
         sum_aw    += (double)SigFlow->awSig.size();
      if (SigFlow->pdSig.size())
         sum_pad   += (double)SigFlow->pdSig.size();
      if (SigFlow->matchSig.size())
         sum_match += (double)SigFlow->matchSig.size();
      AnalysisReport->IncrementSigEvents();
   }
#endif
};

   
class AnalysisReportModule: public TARunObject
{
public:

   bool fTrace = false;

   AnalysisReportFlags* fFlags;

   AnalysisReportModule(TARunInfo* runinfo, AnalysisReportFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="AnalysisReport";
#endif
      if (fTrace)
         printf("AnalysisReportModule::ctor!\n");
         
      if (!getenv("AGRELEASE"))
      {
         std::cerr<<"AGRELEASE not set! Did you mean to 'source agconfig.sh'?"<<std::endl;
         exit(1);
      }
   }

   ~AnalysisReportModule()
   {
      if (fTrace)
         printf("AnalysisReportModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      if (fFlags->fPrint)
         printf("AnalysisReportModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("AnalysisReport")->cd();
      fFlags->AnalysisReport=new TAGAnalysisReport(runinfo->fRunNo);
      uint32_t midas_start_time = -1 ;
      #ifdef INCLUDE_VirtualOdb_H
      midas_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      #endif
      #ifdef INCLUDE_MVODB_H
      runinfo->fOdb->RU32("Runinfo/Start time binary",(uint32_t*) &midas_start_time);
      #endif
      fFlags->AnalysisReport->SetStartTime(midas_start_time);
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("AnalysisReportModule::EndRun, run %d\n", runinfo->fRunNo);
      uint32_t midas_stop_time = -1;
      #ifdef INCLUDE_VirtualOdb_H
      midas_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      #endif
      #ifdef INCLUDE_MVODB_H
      runinfo->fOdb->RU32("Runinfo/Stop time binary",(uint32_t*) &midas_stop_time);
      #endif
      runinfo->fRoot->fOutputFile->cd("AnalysisReport");
      fFlags->AnalysisReport->SetStopTime(midas_stop_time);
      //Fill internal containers with histogram data
      fFlags->AnalysisReport->Flush(
         fFlags->sum_aw,       //Results from deconv module
         fFlags->sum_pad,      //Results from deconv module
         fFlags->sum_match,    //Results from match module
         fFlags->sum_tracks,   //Results from reco module
         fFlags->sum_r_sigma,  //Results from reco module
         fFlags->sum_z_sigma,  //Results from reco module
         fFlags->sum_verts,    //Results from reco module
         fFlags->sum_hits,     //Results from reco module
         fFlags->sum_bars
      );

      //Do the tree writing... I only have one report... so 
      TTree* t=new TTree("AnalysisReport","AnalysisReport");
      t->Branch("TAGAnalysisReport","TAGAnalysisReport",&fFlags->AnalysisReport,32000,0);
      t->Fill();
      t->Write();
      delete t;
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("AnalysisReportModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("AnalysisReportModule::ResumeRun, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent( __attribute__((unused)) TARunInfo* runinfo, __attribute__((unused)) TAFlags* flags, TAFlowEvent* flow)
   {
      //Clocks unfold backwards... 
      std::vector<TAFlowEvent*> flowArray;
      int FlowEvents=0;
      TAFlowEvent* f = flow;
      while (f) 
      {
         flowArray.push_back(f);
         f=f->fNext;
         FlowEvents++;
      }
      for (int ii=FlowEvents-1; ii>=0; ii--)
      {
         f=flowArray[ii];
#ifdef BUILD_AG
         AgAnalysisFlow* analyzed_event=dynamic_cast<AgAnalysisFlow*>(f);
         if (analyzed_event)
         {
            TStoreEvent* e=analyzed_event->fEvent;
            if (e)
            {
               fFlags->FillTPC(e);
            }
            continue;
         }
         AgSignalsFlow* SigFlow = dynamic_cast<AgSignalsFlow*>(f);
         if (SigFlow)
         {
            fFlags->FillTPCSigFlow(SigFlow);
            continue;
         }
#endif
      }
      return flow;
   }
};

class AnalysisReportModuleFactory: public TAFactory
{
public:
   AnalysisReportFlags fFlags;
   void Usage()
   {
      printf("AnalysisReportModuleFactory::Help!\n");
      printf("\t--notime \tTurn off AnalysisReport module processing time calculation and summary\n");
      printf("\t--summarise NNN MMM OOO \t\tPrint SVD summary for NNN, MMM and OOO dumps (no limit on dumps to track)\n");
  
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("AnalysisReportModuleFactory::Init!\n");
      

      for (unsigned i=0; i<args.size(); i++) {
         //Ok, lets support both proper and american spellings
         if (args[i] == "--summarise" || args[i] == "--summarize")
         {
#ifdef BUILD_A2
#else
             std::cerr<<"--summarise feature only available for ALPHA2"<<std::endl;
#endif

         }
      }
   }

   void Finish()
   {
      if (fFlags.AnalysisReport)
         fFlags.AnalysisReport->Print();
      //delete fFlags.AnalysisReport;
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("AnalysisReportModule::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new AnalysisReportModule(runinfo,&fFlags);
   }
};

static TARegister tar(new AnalysisReportModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
