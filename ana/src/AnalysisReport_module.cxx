//
// AnalysisReport_module.cxx
//
// AnalysisReport of all modules
//
// Joseph McKenna
//

#include <stdio.h>
#include <unistd.h> // readlink()

#include "manalyzer.h"
#include "midasio.h"


#include "TH1D.h"
#include "TH2D.h"

#include "AgFlow.h"
#include "A2Flow.h"
#include "GitInfo.h"
#include "AnalysisTimer.h"

//I am intentionally global, external modules test this
bool TimeModules=true;


class A2DumpSummary
{
public:
   std::string DumpName;
   int PassedCuts;
   int Verticies;
   int VF48Events;
   double time;
   int TotalCount;
   A2DumpSummary(const char* name)
   {
      DumpName=name;
      PassedCuts=0;
      Verticies=0;
      VF48Events=0;
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
      PassedCuts+=s->ScalerData->PassCuts;
      Verticies+=s->ScalerData->Verticies;
      VF48Events+=s->ScalerData->VertexEvents;
      time+=s->ScalerData->StopTime-s->ScalerData->StartTime;
      TotalCount++;
   }
   void Print()
   {
      printf("DUMP SUMMARY:%s\t DumpCount:%d\t VF48Events:%d \tVerticies:%d\t PassedCuts:%d\t TotalTime:%f\t\n",
                   DumpName.c_str(),
                   TotalCount,
                   VF48Events,
                   Verticies,
                   PassedCuts,
                   time);
   }
};

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


class MeanMode
{
   private:
   const int mode_size;
   std::vector<int>* mode_hist;
   double running_mean;
   int entries;
   public:
   MeanMode(const int size): mode_size(size)
   {
      mode_hist=new std::vector<int>(size,0);
      running_mean=0;
      entries=0;
   }
   ~MeanMode()
   {
      mode_hist->clear();
      delete mode_hist;
   }
   void InsertValue(const int &x)
   {
      if (x<mode_size)
         (*mode_hist)[x]++;
      else
         mode_hist->back()++;
      running_mean+=(double)x;
      entries++;
      return;
   }
   double GetSum()
   {
      return running_mean;
   }
   int GetEntires()
   {
      return entries;
   }
   double GetMean()
   {
      return running_mean/(double)entries;
   }
   long unsigned GetMode()
   {
      int* mode_ptr=std::max_element(&mode_hist->front(),&mode_hist->back());
      return (long unsigned)(mode_ptr-&mode_hist->front());
   }
   int GetBin(const int x)
   {
      return mode_hist->at(x);
   }
   double GetRate(const int bin,const int t)
   {
      return (double) mode_hist->at(bin)/t;
   }
};


class AnalysisReportFlags
{
public:
   bool fPrint = false;
   std::string binary_name;
   std::string binary_path;

   bool fSaveHistograms = false;

   double mean_aw=0;       //Results from deconv module
   double mean_pad=0;      //Results from deconv module
   double mean_match=0;    //Results from match module
   double mean_tracks=0;   //Results from reco module
   double mean_r_sigma=0;  //Results from reco module
   double mean_z_sigma=0;  //Results from reco module
   double mean_verts=0;    //Results from reco module
   double mean_hits=0;     //Results from reco module
   double mean_bars=0;     //Results from reco module
   double last_event_ts=-1.; //Results from reco module

   int nStoreEvents=0;
   int nSigEvents=0;

   //ALPHA 2
   int nSVDEvents=0;
   double SVD_passrate=-1.;
   MeanMode SVD_N_RawHits{1000};
   MeanMode SVD_P_RawHits{1000};
   //MeanMode SVD_N_Clusters(1000);
   //MeanMode SVD_P_Clusters(1000);
   MeanMode SVD_RawHits{1000};
   MeanMode SVD_Hits{1000};
   MeanMode SVD_Tracks{100};
   MeanMode SVD_Verts{10};
   MeanMode SVD_Pass{2};

   A2DumpSummaryList DumpLogs;

   int RunNumber=-1;
   time_t midas_start_time=0;
   time_t midas_stop_time=0;
   void FillTPC(TStoreEvent* e)
   {
      if (e->GetNumberOfTracks()>0)
      {
         mean_tracks+=e->GetNumberOfTracks();
         //if (std::isfinite(e->GetMeanZSigma())) 
         mean_z_sigma+=e->GetMeanZSigma();
         //if (std::isfinite(e->GetMeanRSigma())) 
         mean_r_sigma+=e->GetMeanRSigma();
      }
      if (e->GetVertexStatus()>0)
         mean_verts +=1;
      if (e->GetNumberOfPoints()>0)
         mean_hits  +=e->GetNumberOfPoints();
      if (e->GetBarMultiplicity()>0)
         mean_bars  +=e->GetBarMultiplicity();
      last_event_ts = e->GetTimeOfEvent();
      nStoreEvents++;
   }
   void FillTPCSigFlow(AgSignalsFlow* SigFlow)
   {
      if (SigFlow->awSig)
         mean_aw=(double)SigFlow->awSig->size();
      if (SigFlow->pdSig)
         mean_pad+=(double)SigFlow->pdSig->size();
      if (SigFlow->matchSig)
         mean_match+=(double)SigFlow->matchSig->size();
      nSigEvents++;
   }
   void CalculateTPCMeans()
   {
      if (nSigEvents>0)
      {
         mean_aw=mean_aw/(double)nSigEvents;
         mean_pad=mean_pad/(double)nSigEvents;
         mean_match=mean_match/(double)nSigEvents;
      }

      if (nStoreEvents>0)
      {
         mean_tracks =mean_tracks/(double)nStoreEvents;
         mean_verts  =mean_verts/(double)nStoreEvents;
         mean_r_sigma=mean_r_sigma/(double)nStoreEvents;
         mean_z_sigma=mean_z_sigma/(double)nStoreEvents;
         mean_hits   =mean_hits/(double)nStoreEvents;
         mean_bars   =mean_bars/(double)nStoreEvents;
      }
      return;
   }
   void PrintAG()
   {
      if (nStoreEvents>0)
      {
         std::cout <<"Mean #AW:   \t:"<<mean_aw<<std::endl;
         std::cout <<"Mean #PAD:   \t:"<<mean_pad<<std::endl;
         std::cout <<"Mean #MATCH:   \t:"<<mean_match<<std::endl;
         std::cout <<"Mean #Hits: \t"<<mean_hits<<std::endl;
         std::cout <<"Mean #Tracks:\t"<<mean_tracks<<"\t(Mean ChiR:"<<mean_z_sigma<<" ChiZ:"<<mean_r_sigma<<")"<<std::endl;
         std::cout <<"Mean #Verts:\t"<<mean_verts<<std::endl;
         std::cout <<"Mean #Bars:\t" <<mean_bars<<std::endl;
      }
      return;
   }
   void FillSVD(TSiliconEvent* se)
   {
      SVD_N_RawHits.InsertValue(se->GetNsideNRawHits());
      SVD_P_RawHits.InsertValue(se->GetPsideNRawHits());
      //SVD_N_Clusters.InsertValue(se->GetNNClusters());
      //SVD_P_Clusters.InsertValue(se->GetNPClusters());
      SVD_RawHits.InsertValue(se->GetNRawHits());
      SVD_Hits.InsertValue(se->GetNHits());
      SVD_Tracks.InsertValue(se->GetNTracks());
      SVD_Verts.InsertValue(se->GetNVertices());
      SVD_Pass.InsertValue((int)se->GetPassedCuts());
      nSVDEvents++;
      return;
   }
   void PrintA2(int rough_time=-1)
   {
      if(nSVDEvents>0)
      {
         std::cout <<"Number of SVD Events:\t"<<nSVDEvents<<std::endl;
         std::cout <<"               \tMode\tMean"<<std::endl;
         std::cout <<"SVD #RawNHits: \t"<<SVD_N_RawHits.GetMode()<<"\t"<<SVD_N_RawHits.GetMean()<<std::endl;
         std::cout <<"SVD #RawPHits: \t"<<SVD_P_RawHits.GetMode()<<"\t"<<SVD_P_RawHits.GetMean()<<std::endl;
         //std::cout <<"Mean SVD #RawHits: \t" <<SVD_RawHits.GetMode()  <<"\t"<<SVD_RawHits.GetMean()  <<std::endl;
         std::cout <<"SVD #Hits: \t"    <<SVD_Hits.GetMode()     <<"\t"<<SVD_Hits.GetMean()     <<std::endl;
         std::cout <<"SVD #Tracks:\t"   <<SVD_Tracks.GetMode()   <<"\t"<<SVD_Tracks.GetMean()   <<std::endl;

         std::cout<<"----------------Sum-----Mean---------"<<std::endl;
         //std::cout<<"SVD Events:\t"<< SVD_Verts
         std::cout <<"SVD #Events:\t"   <<SVD_Tracks.GetEntires()<<std::endl;
         std::cout <<"SVD #Verts:\t"    <<SVD_Verts.GetSum()     <<"\t"<<SVD_Verts.GetMean();
         if (rough_time>0)
         {
            double SVD_vertrate=SVD_Verts.GetRate(1,rough_time);
            if (SVD_vertrate<0.1)
               printf("\t~(%.1fmHz)",SVD_vertrate*1000.);
            else
               printf("\t~(%.1fHz)",SVD_vertrate);
         }
         std::cout<<std::endl;

         std::cout <<"SVD #Pass cuts:\t"<<SVD_Pass.GetSum()         <<"\t"<<SVD_Pass.GetMean();
         if (rough_time>0)
         {
            double SVD_passrate=SVD_Pass.GetRate(1,rough_time);
            if (SVD_passrate<0.1)
               printf("\t~(%.1fmHz)",SVD_passrate*1000.);
            else
               printf("\t~(%.1fHz)",SVD_passrate);
         }
         std::cout<<std::endl;
      }
   }
};

   
class AnalysisReportModule: public TARunObject
{
public:

   bool fTrace = false;
   std::string binary_path_full;
   
   AnalysisReportFlags* fFlags;

  

   AnalysisReportModule(TARunInfo* runinfo, AnalysisReportFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      ModuleName="AnalysisReport";
      if (fTrace)
         printf("AnalysisReportModule::ctor!\n");
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

      fFlags->RunNumber= runinfo->fRunNo;

      #ifdef INCLUDE_VirtualOdb_H
      fFlags->midas_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      #endif
      #ifdef INCLUDE_MVODB_H
      runinfo->fOdb->RU32("/Runinfo/Start time binary",(uint32_t*) &fFlags->midas_start_time);
      #endif

      //last_module_time= clock();
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory



      
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("AnalysisReport")->cd();
     
      char result[ 200 ]={0};
      readlink( "/proc/self/exe", result, 200 );
      binary_path_full=result;
      std::size_t found = binary_path_full.find_last_of("/\\");
      //std::cout << " path: " << binary_path_full.substr(0,found).c_str() << '\n';
      //std::cout << " file: " << binary_path_full.substr(found+1).c_str() << '\n';
   
      fFlags->binary_path=binary_path_full.substr(0,found);
      fFlags->binary_name=binary_path_full.substr(found+1);
      
      //      return std::string( result, (count > 0) ? count : 0 );
      //if (fSaveHistograms)
       //  RecoTime = new TH1D("RecoTime", "Analysis time per event; time, s", 101, -50, 50);
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("AnalysisReportModule::EndRun, run %d\n", runinfo->fRunNo);
      #ifdef INCLUDE_VirtualOdb_H
      fFlags->midas_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      #endif
      #ifdef INCLUDE_MVODB_H
      runinfo->fOdb->RU32("/Runinfo/Stop time binary",(uint32_t*) &fFlags->midas_stop_time);
      #endif

      //time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      //printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));
      std::cout<<"Flow event average processing time (approximate)"<<std::endl;

      fFlags->CalculateTPCMeans();

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
         SilEventsFlow* SilFlow = dynamic_cast<SilEventsFlow*>(f);
         if(SilFlow)
         {
            TSiliconEvent* se=SilFlow->silevent;
            fFlags->FillSVD(se);
            continue;
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
      
      //fSaveHistograms=false;
      //fDoPads = true;
      //fPlotPad = -1;
      //fPlotPadCanvas = NULL;
      
      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--notime")
             TimeModules=false;
         //Ok, lets support both proper and american spellings
         if (args[i] == "--summarise" || args[i] == "--summarize")
         {
             while (args[i+1].c_str()[0]!='-')
             {
                char dump[80];
                sprintf(dump,"\"%s\"",args[++i].c_str());
                fFlags.DumpLogs.TrackDump(dump);
                if (i+1==args.size()) break;
             }
         }
         if (args[i] == "--AnalysisReport")
            fFlags.fSaveHistograms = true;
         
      }
   }

   void Finish()
   {
      fFlags.DumpLogs.Print();
      //Git revision date:
      time_t t = GIT_DATE;
      struct tm *tm = localtime(&t);
      char date[20];
      strftime(date, sizeof(date), "%Y-%m-%d\t%X", tm);
      t = COMPILATION_DATE;
      tm = localtime(&t);
      char comp_date[20];
      strftime(comp_date, sizeof(comp_date), "%Y-%m-%d\t%X", tm);

      char now[20];
      strftime(now, sizeof(now), "%Y-%m-%d\t%X", tm);
      printf("===========================================================\n");
      printf("%s Report for run %d\n",fFlags.binary_name.c_str(),fFlags.RunNumber);
      printf("===========================================================\n");
      std::cout <<"Start Run: "<<asctime(localtime(&fFlags.midas_start_time));
      std::cout <<"Stop Run: ";
      if (fFlags.midas_stop_time==0)
         std::cout <<" UNKNOWN\t(end-of-run ODB entry not processed)"<<std::endl;
      else
         std::cout<<asctime(localtime(&fFlags.midas_stop_time));
      int rough_time=-1;
      if( fFlags.midas_stop_time > fFlags.midas_start_time )
      {
         rough_time=difftime(fFlags.midas_stop_time,fFlags.midas_start_time);
         std::cout <<"Duration: "<<rough_time<<" s"<<std::endl;
      }
      fFlags.PrintAG();
      fFlags.PrintA2(rough_time);
      std::cout <<"Time of Last Event: "<<fFlags.last_event_ts<<" s"<<std::endl;
      printf("Compilation date:%s\n",comp_date);
      std::cout <<"Analysis run on host: ";
      if(getenv("HOSTNAME")!=nullptr) {std::cout << getenv("HOSTNAME") << std::endl;} else { std::cout << "UNKNOWN" << std::endl;}
      printf("Git branch:      %s\n",GIT_BRANCH);
      printf("Git date:         %s\n",date);
      //printf("Git date:        %d\n",GIT_DATE);
      printf("Git hash:        %s\n",GIT_REVISION);
      printf("Git hash (long): %s\n",GIT_REVISION_FULL);
      printf("Git diff (shortstat):%s\n",GIT_DIFF_SHORT_STAT);
      printf("===========================================================\n");
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
