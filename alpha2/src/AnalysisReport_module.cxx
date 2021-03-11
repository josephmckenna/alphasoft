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

#ifdef BUILD_A2
#include "A2Flow.h"
#endif

#include "TAnalysisReport.h"

//I am intentionally global, external modules test this
bool TimeModules=true;
#ifdef BUILD_A2
class A2DumpSummary
{
public:
   std::string DumpName;
   int PassedCuts;   double PassedCuts_sqsum;
   int Verticies;    double Verticies_sqsum;
   int VF48Events;   double VF48Events_sqsum;
   double time;
   int TotalCount;
   A2DumpSummary(const char* name)
   {
      DumpName=name;
      PassedCuts      =0;
      PassedCuts_sqsum=0;
      Verticies       =0;
      Verticies_sqsum =0;
      VF48Events      =0;
      VF48Events_sqsum=0;
      time=0.;
      TotalCount=0;
   }
   void Fill(TA2Spill* s)
   {
      if (!s->ScalerData)
      {
         std::cout<<"Error: Spill has no scaler data to fill!"<<std::endl;
         return;
      }
      //std::cout<<"Adding spill to list"<<std::endl;
      PassedCuts       += s->ScalerData->PassCuts;
      PassedCuts_sqsum += s->ScalerData->PassCuts * s->ScalerData->PassCuts;

      Verticies        += s->ScalerData->Verticies;
      Verticies_sqsum  += s->ScalerData->Verticies * s->ScalerData->Verticies;

      VF48Events       += s->ScalerData->VertexEvents;
      VF48Events_sqsum += s->ScalerData->VertexEvents * s->ScalerData->VertexEvents;

      time             += s->ScalerData->StopTime-s->ScalerData->StartTime;
      TotalCount++;
   }
   double calc_stdev(double sq_sum, int scaler)
   {
      double mean= (double) scaler / (double) TotalCount;
      double variance = sq_sum / TotalCount - mean * mean;
      return sqrt(variance);
   }
   /*void Print()
   {
      printf("DUMP SUMMARY:%s\t DumpCount: %d  \t VF48Events: %d ( %f )\tVerticies: %d ( %f )\t PassedCuts: %d ( %f )\t TotalTime: %f\t\n",
                   DumpName.c_str(),
                   TotalCount,
                   VF48Events, calc_stdev(VF48Events_sqsum, VF48Events),
                   Verticies, calc_stdev(Verticies_sqsum, Verticies), 
                   PassedCuts, calc_stdev(PassedCuts_sqsum, PassedCuts),
                   time);
   }*/
   void Print()
    {
      std::streamsize ss = std::cout.precision();
       std::cout<<"DUMP SUMMARY: "<< DumpName.c_str() << "\t";
       std::cout<<"DumpCount: "   << TotalCount << "\t";
       std::cout<<"VF48Events: "  << VF48Events << "\t";
       std::cout<<"Verticies: "   << Verticies << " (" << std::setprecision(3) << 100.*Verticies/VF48Events << "% / "  << Verticies/time <<"Hz)\t";
       std::cout<<"PassedCuts: "  << PassedCuts << " (" << std::setprecision(3) << 100.*PassedCuts/VF48Events << "% / " << PassedCuts/time <<"Hz)\t";
      std::cout<<"TotalTime: "   << std::setprecision(ss) << time << std::endl;
    }
};
#endif
#ifdef BUILD_A2
class A2DumpSummaryList
{
public:
   std::vector<A2DumpSummary*> list;
   A2DumpSummaryList() {}
   ~A2DumpSummaryList()
   {
      const int size=list.size();
      for (int i=0; i<size; i++)
         delete list[i];
      list.clear();
   }
   
   void TrackDump(const char* d)
   {
      list.push_back(new A2DumpSummary(d));
      return;
   }
   void Fill(TA2Spill* s)
   {
      //std::cout<<"Filling list "<<s->Name.c_str()<<std::endl;
      const int size=list.size();
      if (!size) return;
      for (int i=0; i<size; i++)
      {
         //std::cout<<list[i]->DumpName.c_str()<<"vs"<<s->Name.c_str()<<std::endl;
         if (strcmp(list[i]->DumpName.c_str(),s->Name.c_str())==0)
            list[i]->Fill(s);
      }
      return;
   }
   void Print()
   {
      const int size=list.size();
      for (int i=0; i<size; i++)
         list[i]->Print();
   }
};
#endif



class AnalysisReportFlags
{
public:
   bool fPrint = false;
   TA2AnalysisReport* AnalysisReport = NULL;
   double last_event_ts=-1.; //Results from reco module
#ifdef BUILD_A2
   A2DumpSummaryList DumpLogs;
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
#ifdef MANALYZER_PROFILER
      ModuleName="AnalysisReport";
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
      #ifdef HAVE_CXX11_THREADS
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      #endif
      if (fFlags->fPrint)
         printf("AnalysisReportModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("AnalysisReport")->cd();
      fFlags->AnalysisReport=new TA2AnalysisReport(runinfo->fRunNo);
      uint32_t midas_start_time = -1 ;
      #ifdef INCLUDE_VirtualOdb_H
      midas_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      #endif
      #ifdef INCLUDE_MVODB_H
      runinfo->fOdb->RU32("/Runinfo/Start time binary",(uint32_t*) &midas_start_time);
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
      runinfo->fOdb->RU32("/Runinfo/Stop time binary",(uint32_t*) &midas_stop_time);
      #endif
      runinfo->fRoot->fOutputFile->cd("AnalysisReport");
      fFlags->AnalysisReport->SetStopTime(midas_stop_time);
      //Fill internal containers with histogram data
      fFlags->AnalysisReport->Flush();

      //Do the tree writing... I only have one report... so 
      TTree* t=new TTree("AnalysisReport","AnalysisReport");
      t->Branch("TA2AnalysisReport","TA2AnalysisReport",&fFlags->AnalysisReport,32000,0);
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

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
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
#ifdef BUILD_A2
         SilEventFlow* SilFlow = dynamic_cast<SilEventFlow*>(f);
         if(SilFlow)
         {
            TSiliconEvent* se=SilFlow->silevent;
            fFlags->AnalysisReport->FillSVD(
                se->GetNsideNRawHits(),
                se->GetPsideNRawHits(),
                //SVD_N_Clusters->Fill(se->GetNNClusters());
                //SVD_P_Clusters->Fill(se->GetNPClusters());
                se->GetNRawHits(),
                se->GetNHits(),
                se->GetNTracks(),
                se->GetNVertices(),
                se->GetPassedCuts(),
                se->GetVF48Timestamp()
                );
    
         }
         const A2SpillFlow* SpillFlow= dynamic_cast<A2SpillFlow*>(f);
         if (SpillFlow)
         {
            for (size_t i=0; i<SpillFlow->spill_events.size(); i++)
            {
               TA2Spill* s=SpillFlow->spill_events.at(i);
               //s->Print();
               fFlags->DumpLogs.Fill(s);
            }
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
             while (args[i+1].c_str()[0]!='-')
             {
                char dump[80];
                sprintf(dump,"\"%s\"",args[++i].c_str());
                fFlags.DumpLogs.TrackDump(dump);
                if (i+1==args.size()) break;
             }
#else
             std::cerr<<"--summarise feature only available for ALPHA2"<<std::endl;
#endif

         }
      }
   }

   void Finish()
   {
#ifdef BUILD_A2
      fFlags.DumpLogs.Print();
#endif
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
