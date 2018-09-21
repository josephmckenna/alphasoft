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
#include "GitInfo.h"
#include "AnalysisTimer.h"
bool TimeModules=false;
clock_t tStart_cpu;
time_t tStart_user;
class AnalysisReportModule: public TARunObject
{
public:

   bool fTrace = false;
   bool fSaveHistograms;

   clock_t last_flow_event;
   clock_t last_module_time;
   std::map<TString,int> FlowMap;
   std::map<TString,int> ModuleMap;
   std::vector<TH1D*> FlowHistograms;
   std::vector<double> MaxFlowTime;
   std::vector<TH1D*> ModuleHistograms;
   std::vector<double> MaxModuleTime;

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
      //if (fTrace)
         printf("AnalysisReportModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      
      tStart_cpu = clock();
      tStart_user = time(NULL);
      
      last_flow_event= time(NULL);
      last_module_time= time(NULL);
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

     // gDirectory->mkdir("AnalysisReport")->cd();
      //if (fSaveHistograms)
       //  RecoTime = new TH1D("RecoTime", "Analysis time per event; time, s", 101, -50, 50);
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("AnalysisReportModule::EndRun, run %d\n", runinfo->fRunNo);
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
         std::cout<<"Module average processing time (approximate)"<<std::endl;
         std::cout<<"Module\t\t\tEntries\tMean T\tRMS\tMax T"<<std::endl;
         for (uint i=0; i<ModuleHistograms.size(); i++)
         {
           //std::cout<<ModuleHistograms.at(i)->GetTitle()<<"\t\t";
           printf("%-20s\t%d\t%.3f\t%.3f\t",ModuleHistograms.at(i)->GetTitle(),
                                   (int)ModuleHistograms.at(i)->GetEntries(),
                                   ModuleHistograms.at(i)->GetMean(),
                                   ModuleHistograms.at(i)->GetRMS());
           std::cout<<MaxModuleTime.at(i)<<std::endl;
         }
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
      MaxModuleTime.push_back(0.);
      return;
   }
   Double_t DeltaModuleTime(clock_t* time)
   {
      double cputime = (double)(*time - last_module_time)/CLOCKS_PER_SEC;
      last_module_time = *time;
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
      TAFlowEvent* f = flow;
         while (f) 
         {
            AgAnalysisReportFlow* timer=dynamic_cast<AgAnalysisReportFlow*>(f);
            if (timer)
            {
               const char* name=timer->ModuleName;
               if (!ModuleMap.count(name))
                  AddModuleMap(name);
               double dt=DeltaModuleTime(timer->time);
               int i=ModuleMap[name];
               if (dt>MaxModuleTime[i]) MaxModuleTime.at(i)=dt;
               ModuleHistograms.at(i)->Fill(dt);
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
            f = f->fNext;
         }

      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("AnalysisReportModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class AnalysisReportModuleFactory: public TAFactory
{
public:
   void Usage()
   {
      printf("\t--time");
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("AnalysisReportModuleFactory::Init!\n");
      
      //fSaveHistograms=false;
      //fDoPads = true;
      //fPlotPad = -1;
      //fPlotPadCanvas = NULL;
      
      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--time")
             TimeModules=true;
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
      strftime(date, sizeof(date), "%Y-%m-%d", tm);

      //CPU and Wall clock time:
      double cputime = (double)(clock() - tStart_cpu)/CLOCKS_PER_SEC;
      double usertime = difftime(time(NULL),tStart_user);

      printf("===========================================================\n");
      printf("AnalysisReportModuleFactory::Finish!\n");
      printf("===========================================================\n");
      std::cout << getenv("_") << " exec time:\tCPU: "<< cputime <<"s\tUser: " << usertime << "s"<<std::endl;
      printf("Git branch:      %s\n",GIT_BRANCH);
      printf("Git date:         %s\n",date);
      //printf("Git date:        %d\n",GIT_DATE);
      printf("Git hash:        %s\n",GIT_REVISION);
      printf("Git hash (long): %s\n",GIT_REVISION_FULL);
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
