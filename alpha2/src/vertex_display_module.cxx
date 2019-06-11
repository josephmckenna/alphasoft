//
// handle_sequencer
//
// A. Capra
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"

#include "A2Flow.h"

#include "TApplication.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"

#include "AnalysisTimer.h"

class VertexDisplayFlags
{
public:
   bool fPrint = false;
   bool fDraw = false;
};
struct Vertex
{
double x,y,z,t;
bool passed_cut,passed_online_mva;
};
struct Occupancy
{
int hits[72][4];
double t;
};
class VertexDisplay: public TARunObject
{
private:
  TApplication *VertApp;
  TCanvas* VertDisplay;
  TVirtualPad *VertexDisplay_sub1;
  TH2D* XY_pased_cuts;
  TH2D* XY_online_mva;
  TH1D* Z_pased_cuts;
  TH1D* Z_online_mva;
  
  TH2D* US_occ;
  TH2D* DS_occ;
  
  int NQueues;
  TH1D* AnalysisQueue;
  std::deque<Vertex> Events;
  std::deque<Occupancy> ModHits;
  double IntegrationWindow;
  double LastEventTime;
  double LastDrawTime;
  double DrawInterval;
public:
   VertexDisplayFlags* fFlags;
   bool fTrace = false;
   
   
   VertexDisplay(TARunInfo* runinfo, VertexDisplayFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("VertexDisplay::ctor!\n");
       if (!fFlags->fDraw) return;
       VertApp    =new TApplication("VertApp", 0, 0);
       VertDisplay=new TCanvas("VertexDisplay","VertexDisplay");
       VertDisplay->Divide(3,2);
       VertexDisplay_sub1 = VertDisplay->cd(6);
       gPad->Divide(2, 1);
       double x=5;
       double y=5;
       double z=30;
       int bins=25;
       XY_pased_cuts   =new TH2D("XY_passed_cuts","XY_passed_cuts",bins,-x,x,bins,-y,y);
       XY_online_mva   =new TH2D("XY_online_mva","XY_online_mva",bins,-x,x,bins,-y,y);
       //ZY         =new TH2D("ZY","ZY",bins,-z,z,bins,-y,y);
       Z_pased_cuts    =new TH1D("ZY_passed_cuts","ZY_passed_cuts",bins,-z,z);
       Z_online_mva    =new TH1D("ZY_online_mva","ZY_online_mva",bins,-z,z);
       //US_occ     =new TH2D("US_occ","US_occ",3,0,3,14,-TMath::Pi(),TMath::Pi());
       US_occ     =new TH2D("US_occ","US_occ",2,0,1.5,72,0,72);
      // US_occ     =new TH2D("DS_occ","DS_occ",3,0,3,14,-TMath::Pi(),TMath::Pi());
       DS_occ     =new TH2D("DS_occ","DS_occ",2,0,1.5,72,0,72);
       LastEventTime=0.;
       IntegrationWindow=10.;
       LastDrawTime=0.;
       DrawInterval=1.;
       
       NQueues=0;
   }

   ~VertexDisplay()
   {
      if (fTrace)
         printf("VertexDisplay::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("VertexDisplay::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      if (!fFlags->fDraw) return; 
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      NQueues=0;
       if (runinfo->fMtInfo)
       {
         NQueues=runinfo->fMtInfo->fMtThreads.size();
         AnalysisQueue=new TH1D("AnalysisQueue","AnalysisQueue",NQueues,0,NQueues);
       }  
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("VertexDisplay::EndRun, run %d\n", runinfo->fRunNo);
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("VertexDisplay::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
  {
      if (!fFlags->fDraw) return flow;
      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif      
      SilEventsFlow* fe=flow->Find<SilEventsFlow>();
      if (!fe)
         return flow;
      TSiliconEvent* SiliconEvent=fe->silevent;
      
      A2OnlineMVAFlow* of=flow->Find<A2OnlineMVAFlow>();
            
      //if (SiliconEvent->GetNVertices()==0)
      //   return flow;
      Vertex data;
      TVector3* vert=SiliconEvent->GetVertex();
      data.x=vert->X();
      data.y=vert->Y();
      data.z=vert->Z();
      //VF48 timestamp isn't the most accurate, but this modules doesn't care about calibrated accute time
      data.t=SiliconEvent->GetVF48Timestamp();
      data.passed_cut=SiliconEvent->GetPassedCuts();
      if (of)
        data.passed_online_mva=of->pass_online_mva;
      else
        data.passed_online_mva=false;
      
      //Add our new datapoint
      Events.push_back(data);
      
      //Remove events that are from too long ago
      while(Events.size()>0)
      {
         if (Events[0].t+IntegrationWindow<data.t)
            Events.pop_front();
         else
            break;
      }

      Occupancy Modules;
      for (int i=0; i<72; i++)
      {
        for (int j=0; j<4; j++)
           Modules.hits[i][j]=0;
        TSiliconModule* m=SiliconEvent->GetSiliconModule(i);
        if (!m) continue;
        for (int j=0; j<4; j++)
        {
           TSiliconVA* asic=m->GetASIC(j+1);
           if (!asic) continue;
           if (asic->IsAHitOR())
           Modules.hits[i][j]++;
        }
      }
      Modules.t=SiliconEvent->GetVF48Timestamp();
      ModHits.push_back(Modules);
      //Remove events that are from too long ago
      while(ModHits.size()>0)
      {
         if (ModHits[0].t+IntegrationWindow<data.t)
            ModHits.pop_front();
         else
            break;
      }

      if (LastDrawTime+DrawInterval<data.t)
      {
         XY_pased_cuts->Reset();
         XY_online_mva->Reset();
         Z_pased_cuts->Reset();
         Z_online_mva->Reset();
         if (NQueues)
            AnalysisQueue->Reset();
         US_occ->Reset();
         DS_occ->Reset();

         //Refill histogram
         for (size_t i=0; i<Events.size(); i++)
         {
            if (Events[i].passed_cut)
            {
               XY_pased_cuts->Fill(Events[i].x,Events[i].y);
               Z_pased_cuts->Fill(Events[i].z);
            }
            if (Events[i].passed_online_mva)
            {
               XY_online_mva->Fill(Events[i].x,Events[i].y);
               Z_online_mva->Fill(Events[i].z);
            }
         }
         for (size_t i=0; i<ModHits.size(); i++)
         {
            for (int j=0; j<72; j++)
            {
               for (int k=0; k<2; k++)
               {
                  //if (ModHits[i].hits[j][k])
                  //std::cout<<"Si:"<<j<< " ASIC:"<<k<<std::endl; 
                  US_occ->Fill(k,j,(double)ModHits[i].hits[j][k]/IntegrationWindow);
               }
               for (int k=2; k<4; k++)
               {
                  DS_occ->Fill(k-2,j,(double)ModHits[i].hits[j][k]/IntegrationWindow);
               }
            }
         }
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         for (int i=0; i<NQueues; i++)
         {
           int j=0;
           {
              std::lock_guard<std::mutex> lock(runinfo->fMtInfo->fMtFlowQueueMutex[i]);
              j=runinfo->fMtInfo->fMtFlagQueue[i].size();
           }
           //std::cout<<"Queue: "<<i<<" has "<<j<<std::endl;
           AnalysisQueue->Fill(i,j);
           
		 }
          
      //Draw histograms
      VertDisplay->cd(1);
      XY_online_mva->Draw("colz");
      VertDisplay->cd(4);
      Z_online_mva->Draw("colz");
      //Draw histograms
      VertDisplay->cd(2);
      XY_pased_cuts->Draw("colz");
      VertDisplay->cd(5);
      Z_pased_cuts->Draw("colz");
      if (NQueues)
      {
         VertDisplay->cd(3);
         AnalysisQueue->Draw("hist");
      }
      VertexDisplay_sub1->cd(1);
      //VertDisplay->cd(4);
      US_occ->Draw("colz");
      VertexDisplay_sub1->cd(2);
      //VertDisplay->cd(4);
      DS_occ->Draw("colz");
      //Update canvas
      VertDisplay->Update();
      LastDrawTime=data.t;
      }
      //Run App
 //     app->Run();
      #define SLOW_TO_REALTIME 0
      #if SLOW_TO_REALTIME
      double sleeptime=data.t-LastEventTime;
      std::cout<<"Sleeping for "<<sleeptime<<std::endl;
      usleep(sleeptime/1000);
      #endif
      
      LastEventTime=data.t;
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"vertex_display",timer_start);
      #endif
      return flow; 
  }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("VertexDisplay::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class VertexDisplayFactory: public TAFactory
{
public:
   VertexDisplayFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("VertexDisplayFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--live")
            fFlags.fDraw = true;
      }
   }

   void Finish()
   {
      printf("VertexDisplayFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("VertexDisplayFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new VertexDisplay(runinfo, &fFlags);
   }
};

static TARegister tar(new VertexDisplayFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
