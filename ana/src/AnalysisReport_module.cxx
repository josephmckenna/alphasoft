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

#include "AgFlow.h"
#include "A2Flow.h"
#include "GitInfo.h"
#include "AnalysisTimer.h"


//I am not happy these are global... but I want to use them in 'Finish' 
//section rather than end run...
bool TimeModules=true;
clock_t tStart_cpu;
time_t tStart_user;

double mean_aw;       //Results from deconv module
double mean_pad;      //Results from deconv module
double mean_match;    //Results from match module
double mean_tracks;   //Results from reco module
double mean_r_sigma;  //Results from reco module
double mean_z_sigma;  //Results from reco module
double mean_verts;    //Results from reco module
double mean_hits;     //Results from reco module
double mean_bars;     //Results from reco module
double last_event_ts; //Results from reco module


int nStoreEvents;
int nSigEvents;
   
//ALPHA2
int nSVDEvents;
double SVD_meanrawhits;
double SVD_meanhits;
double SVD_meantracks;
double SVD_meanverts;
double SVD_meanpass;


int RunNumber;
time_t midas_start_time;
time_t midas_stop_time;
   
class AnalysisReportModule: public TARunObject
{
public:

   bool fTrace = false;
   bool fSaveHistograms;


   
   clock_t last_flow_event;
   std::map<TString,int> FlowMap;
   std::vector<TH1D*> FlowHistograms;
   std::vector<double> MaxFlowTime;

   //clock_t last_module_time;
   std::map<TString,int> ModuleMap;
   std::vector<TH1D*> ModuleHistograms;
   std::map<TString,int> ModuleMap2D;
   std::vector<TH2D*> ModuleHistograms2D;
   std::vector<double> MaxModuleTime;
   std::vector<double> TotalModuleTime;
   


   AnalysisReportModule(TARunInfo* runinfo)
      : TARunObject(runinfo)
   {
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
      //if (fTrace)
         printf("AnalysisReportModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      
      RunNumber= runinfo->fRunNo;
      
      midas_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      
      tStart_cpu = clock();
      tStart_user = time(NULL);
      
      last_flow_event= clock();
      //last_module_time= clock();
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory


      nSigEvents=0;
      mean_aw=0.;
      mean_pad=0.;
      mean_match=0.;
      
      nStoreEvents=0;
      mean_tracks=0.;
      mean_r_sigma=0.;
      mean_z_sigma=0.;
      mean_verts=0.;
      mean_hits=0.;
      mean_bars=0.;
      
      nSVDEvents=0;
      SVD_meanrawhits=0.;
      SVD_meanhits=0.;
      SVD_meantracks=0.;
      SVD_meanverts=0.;
      SVD_meanpass=0.;

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      gDirectory->mkdir("AnalysisReport")->cd();
      //if (fSaveHistograms)
       //  RecoTime = new TH1D("RecoTime", "Analysis time per event; time, s", 101, -50, 50);
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("AnalysisReportModule::EndRun, run %d\n", runinfo->fRunNo);
      midas_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      //time_t run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      //printf("ODB Run stop time: %d: %s", (int)run_stop_time, ctime(&run_stop_time));
      std::cout<<"Flow event average processing time (approximate)"<<std::endl;
      std::cout<<"FlowType\t\tEntries\tMean T\tRMS\tMax T"<<std::endl;
      for (uint i=0; i<FlowHistograms.size(); i++)
      {
        printf("%-20s\t%d\t%.3f\t%.3f\t",FlowHistograms.at(i)->GetTitle(),
                                         (int)FlowHistograms.at(i)->GetEntries(),
                                         FlowHistograms.at(i)->GetMean(),
                                         FlowHistograms.at(i)->GetRMS());
                                         std::cout<<MaxFlowTime.at(i)<<std::endl;
      }
      if (ModuleHistograms.size()>0)
      {
         double AllModuleTime=0;
         for (auto& n : TotalModuleTime)
            AllModuleTime += n;
         std::cout<<"Module average processing time"<<std::endl;
         std::cout<<"Module\t\t\t\tEntries\tMean T\tRMS\tMax T\tTotal T\t Fraction of Total"<<std::endl;
         std::cout<<"------------------------------------------------------------------------------------------"<<std::endl;
         for (uint i=0; i<ModuleHistograms.size(); i++)
         {
           //std::cout<<ModuleHistograms.at(i)->GetTitle()<<"\t\t";
           printf("%-25s\t%d\t%.3f\t%.3f\t",ModuleHistograms.at(i)->GetTitle(),
                                   (int)ModuleHistograms.at(i)->GetEntries(),
                                   ModuleHistograms.at(i)->GetMean(),
                                   ModuleHistograms.at(i)->GetRMS());
           std::cout<<MaxModuleTime.at(i)<<"\t";
           std::cout<<TotalModuleTime.at(i)<<"\t";
           printf("%.1f%%\n",100.*TotalModuleTime.at(i)/AllModuleTime);
           
         }
         std::cout<<"------------------------------------------------------------------------------------------"<<std::endl;
      }
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

      if(nSVDEvents>0)
      {
         SVD_meanrawhits=SVD_meanrawhits/(double)nSVDEvents;
         SVD_meanhits   =SVD_meanhits/(double)nSVDEvents;
         SVD_meantracks =SVD_meantracks/(double)nSVDEvents;
         SVD_meanverts  =SVD_meanverts/(double)nSVDEvents;
         SVD_meanpass   =SVD_meanpass/(double)nSVDEvents;
      }
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
   void AddFlowMap( const char* FlowName)
   {
      #ifdef HAVE_CXX11_THREADS
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      #endif
      gDirectory->cd("/AnalysisReport");
      FlowMap[FlowName]= FlowHistograms.size();
      Int_t Nbins=100;
      Double_t bins[Nbins+1];
      Double_t TimeRange=100; //seconds
      for (int i=0; i<Nbins+1; i++)
      {
         bins[i]=TimeRange*pow(1.1,i)/pow(1.1,Nbins);
         //std::cout <<"BIN:"<<bins[i]<<std::endl;
      }
      TH1D* Histo=new TH1D(FlowName,FlowName,Nbins,bins);
      FlowHistograms.push_back(Histo);
      MaxFlowTime.push_back(0.);
      return;
   }
   void AddModuleMap( const char* ModuleName)
   {
      #ifdef HAVE_CXX11_THREADS
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      #endif
      gDirectory->cd("/AnalysisReport");
      ModuleMap[ModuleName]= ModuleHistograms.size();
      Int_t Nbins=100;
      Double_t bins[Nbins+1];
      Double_t TimeRange=100; //seconds
      for (int i=0; i<Nbins+1; i++)
      {
         bins[i]=TimeRange*pow(1.1,i)/pow(1.1,Nbins);
         //std::cout <<"BIN:"<<bins[i]<<std::endl;
      }
      TH1D* Histo=new TH1D(ModuleName,ModuleName,Nbins,bins);
      ModuleHistograms.push_back(Histo);
      TotalModuleTime.push_back(0.);
      MaxModuleTime.push_back(0.);
      return;
   }
   void AddModuleMap2D( const char* ModuleName )
   {
      #ifdef HAVE_CXX11_THREADS
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      #endif
      gDirectory->cd("/AnalysisReport");
      ModuleMap2D[ModuleName]= ModuleHistograms2D.size();
      Int_t Nbins=100;
      Double_t bins[Nbins+1];
      Double_t TimeRange=100; //seconds
      for (int i=0; i<Nbins+1; i++)
      {
         bins[i]=TimeRange*pow(1.1,i)/pow(1.1,Nbins);
         //std::cout <<"BIN:"<<bins[i]<<std::endl;
      }
      TString name=ModuleName;
      double ymin=0.;
      double ymax=100.;
      //Estimate Y range by key words:
      if (name.Contains("Points")) {
         ymax=2000.;
      } else if (name.Contains("Tracks")) {
         ymax=10.;
      }
      TH2D* Histo=new TH2D(ModuleName,ModuleName,Nbins,bins,Nbins,ymin,ymax);
      ModuleHistograms2D.push_back(Histo);
      
   }
   Double_t DeltaModuleTime( clock_t start, clock_t stop)
   {
      double cputime = (double)(stop - start)/CLOCKS_PER_SEC;
      return cputime;
      
   }
   Double_t DeltaTime()
   {
      double cputime = (double)(clock() - last_flow_event)/CLOCKS_PER_SEC;
      last_flow_event = clock();
      return cputime;
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
            AgAnalysisReportFlow* timer=dynamic_cast<AgAnalysisReportFlow*>(f);
            if (timer)
            {
               const char* name=timer->ModuleName[0];
               if (!ModuleMap.count(name))
                  AddModuleMap(name);
               double dt=999.;
               if (!timer->start)
                  std::cout<<"Module:"<<name<<" gave no start time"<<std::endl;
               else
                  dt=DeltaModuleTime(timer->start,timer->stop);
               int i=ModuleMap[name];
               TotalModuleTime[i]+=dt;
               if (dt>MaxModuleTime[i]) MaxModuleTime.at(i)=dt;
               ModuleHistograms.at(i)->Fill(dt);
               
               if (timer->SecondAxis.size()>0)
               {
                  for (uint sec=0; sec<timer->SecondAxis.size(); sec++)
                  {
                     TString FullTitle(name);
                     FullTitle+=timer->ModuleName[sec+1];
                     if (!ModuleMap2D.count(FullTitle))
                        AddModuleMap2D(FullTitle);
                     int i=ModuleMap2D[FullTitle];
                     //std::cout <<"Filling at "<<i<<"\t"<<ModuleHistograms2D.at(i)->GetTitle()<<"with:"<<dt<<"\t"<<timer->SecondAxis.at(sec)<<std::endl;
                     ModuleHistograms2D.at(i)->Fill(dt,timer->SecondAxis.at(sec));
                  }
               }
            }
            else
            {
               const char*  name=typeid(*f).name(); 
               if (!FlowMap.count(name))
                  AddFlowMap(name);
               double dt=DeltaTime();
               int i=FlowMap[name];
               if (dt>MaxFlowTime[i]) MaxFlowTime.at(i)=dt;
               FlowHistograms.at(i)->Fill(dt);
            }
            
            AgAnalysisFlow* analyzed_event=dynamic_cast<AgAnalysisFlow*>(f);
            if (analyzed_event)
            {
               TStoreEvent* e=analyzed_event->fEvent;
               if (e)
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
            }
            AgSignalsFlow* SigFlow = dynamic_cast<AgSignalsFlow*>(f);
            if (SigFlow)
            {
               if (SigFlow->awSig)
                  mean_aw=(double)SigFlow->awSig->size();
               if (SigFlow->pdSig)
                  mean_pad+=(double)SigFlow->pdSig->size();
               if (SigFlow->matchSig)
                  mean_match+=(double)SigFlow->matchSig->size();
               nSigEvents++;
            }
            SilEventsFlow* SilFlow = dynamic_cast<SilEventsFlow*>(f);
            if(SilFlow)
            {
               TSiliconEvent* se=SilFlow->silevent;
               SVD_meanrawhits+=se->GetNRawHits();
               SVD_meanhits+=se->GetNHits();
               SVD_meantracks+=se->GetNTracks();
               SVD_meanverts+=se->GetNVertices();
               SVD_meanpass+=(int)se->GetPassedCuts();
               nSVDEvents++;
            }
         }
  
      return flow;
   }

};

class AnalysisReportModuleFactory: public TAFactory
{
public:
   void Usage()
   {
      printf("\t--notime");
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
         //if (args[i] == "--AnalysisReport")
            //fSaveHistograms = true;
         
      }
   }

   void Finish()
   {
      //Git revision date:
      time_t t = GIT_DATE;
      struct tm *tm = localtime(&t);
      char date[20];
      strftime(date, sizeof(date), "%Y-%m-%d\t%X", tm);
      t = COMPILATION_DATE;
      tm = localtime(&t);
      char comp_date[20];
      strftime(comp_date, sizeof(comp_date), "%Y-%m-%d\t%X", tm);
      //CPU and Wall clock time:
      double cputime = (double)(clock() - tStart_cpu)/CLOCKS_PER_SEC;
      double usertime = difftime(time(NULL),tStart_user);

      char now[20];
      strftime(now, sizeof(now), "%Y-%m-%d\t%X", tm);
      printf("AnalysisReportModuleFactory::Finish!\n");
      printf("===========================================================\n");
      printf("Analysis Report for run %d\n",RunNumber);
      printf("===========================================================\n");
      std::cout <<"Start Run: "<<asctime(localtime(&midas_start_time));
      std::cout <<"Stop Run: "<<asctime(localtime(&midas_stop_time));
      if( midas_stop_time > midas_start_time )
         std::cout <<"Duration: "<<difftime(midas_stop_time,midas_start_time)<<" s"<<std::endl;
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
      if(nSVDEvents>0)
      {
         std::cout <<"Number of SVD Events:\t"<<nSVDEvents<<std::endl;
         std::cout <<"Mean SVD #RawHits: \t"<<SVD_meanrawhits<<std::endl;
         std::cout <<"Mean SVD #Hits: \t"<<SVD_meanhits<<std::endl;
         std::cout <<"Mean SVD #Tracks:\t"<<SVD_meantracks<<std::endl;
         std::cout <<"Mean SVD #Verts:\t"<<SVD_meanverts<<std::endl;
         std::cout <<"Mean SVD #Pass cuts:\t"<<SVD_meanpass<<std::endl;
      }
      std::cout <<"Time of Last Event: "<<last_event_ts<<" s"<<std::endl;
      printf("Compilation date:%s\n",comp_date);
      std::cout <<"Analysis run on host: "<<getenv("HOSTNAME")<<std::endl;
      std::cout << getenv("_") << " exec time:\tCPU: "<< cputime <<"s\tUser: " << usertime << "s"<<std::endl;
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
      return new AnalysisReportModule(runinfo);
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