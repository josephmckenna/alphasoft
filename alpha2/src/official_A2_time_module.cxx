//
// Module to generate friend trees with 'Offical' cross calibrated 
// time between all modules (EVB and chronoboxes)
// I.E Convert 'RunTime' to 'Official Time'
// JTK McKENNA
//




#include "manalyzer.h"
#include "midasio.h"
#include "A2Flow.h"

#include "TTree.h"
#include "TSISEvent.h"
#include <iostream>
#include "TSVD_QOD.h"

#include "AnalysisTimer.h"

class OfficialA2TimeFlags
{
public:
   bool fPrint = false;
   bool fNoSync= false;
};


class OfficialA2Time: public TARunObject
{
private:
   
   std::vector<double> VF48ts;
   //double VF48ZeroTime=0;
   
   int SVD_channel=-1;
  
   std::deque<double> SISEventRunTime;
   
   std::mutex SVDEventsLock;
   std::deque<ULong64_t> SISClock;
   std::deque<ULong64_t> VF48Clock;
   std::deque<SVDQOD*> SVDEvents;

public:
   OfficialA2TimeFlags* fFlags;

   double SVD_TimeStamp;
   TTree* SVDOfficial=NULL;

   bool fTrace = true;

   OfficialA2Time(TARunInfo* runinfo, OfficialA2TimeFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("OfficialA2Time::ctor!\n");
   }

   ~OfficialA2Time()
   {
      if (fTrace)
         printf("OfficialA2Time::dtor!\n");
      int remaining_svd=SVDEvents.size();
      if (remaining_svd)
      {
         std::cout<<"Warning: "<<remaining_svd <<" SVD events with no official timestamp"<<std::endl;
         for (int i=0; i<remaining_svd; i++)
         {
            delete SVDEvents[i];
         }
         SVDEvents.clear();
      }
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OfficialA2Time::BeginRun, run %d\n", runinfo->fRunNo);
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }
   void PreEndRun(TARunInfo* runinfo)
   {
      runinfo->AddToFlowQueue(SVDMatchTime(runinfo,NULL));
   }
   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OfficialA2Time::EndRun, run %d\n", runinfo->fRunNo);
      //Flush out all un written timestamps
      //FlushSVDTime();
      SVDOfficial->Write();
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OfficialA2Time::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   void CleanSISEventsBefore(double t)
   {
       int n=SISEventRunTime.size();
       for ( int i=0; i<n ; i++)
       {
           
           if (t>SISEventRunTime.front())
           {
             //std::cout<<"Clean if "<< t <<" > " << SISEventRunTime.front() <<std::endl;
             SISEventRunTime.pop_front();
             SISClock.pop_front();
             VF48Clock.pop_front();
           }
           else
           {
              break;
           }
       }
   }
   void SaveQODEvent(TARunInfo* runinfo, SVDQOD* e)
   {
      #ifdef HAVE_CXX11_THREADS
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
      #endif
      runinfo->fRoot->fOutputFile->cd();
      if (!SVDOfficial)
         SVDOfficial=new TTree("SVDOfficialA2Time","SVDOfficialA2Time");
      TBranch* b_variable = SVDOfficial->GetBranch("OfficalTime");
      if (!b_variable)
         SVDOfficial->Branch("OfficalTime","SVDQOD",&e,32000,0);
      else
         SVDOfficial->SetBranchAddress("OfficalTime",&e);
      SVDOfficial->Fill();
      //std::cout<<"Saving at t:"<<e->t<<std::endl;
   }
   double ClockRatio(ULong64_t a, ULong64_t b)
   {
       //a is counts from 20Mhz VF48 Clock
       //b is counts from 10Mhz SIS (atomic) clock...
       //Double SIS counts
       ULong64_t diff=a-b;
       //std::cout<<"Diff"<<diff<<std::endl;
       double r1=((double)diff) / ((double)b);
       return r1+1.;
   }
   void CleanOldTimestamps(double TimeBufferSize)
   {
      if (SISEventRunTime.empty()) return;
      double LatestTime=SISEventRunTime.back();
      double tcut=LatestTime-TimeBufferSize;


      int nSIS=SISEventRunTime.size();
      for (int i=0; i<nSIS; i++)
      {
         if (SISEventRunTime.front()<tcut)
         {
            SISEventRunTime.pop_front();
            SISClock.pop_front();
            VF48Clock.pop_front();
         }
         else
            break;
      }
      /*int nSVD=SVDEvents.size();
      for (int i=0; i<nSIS; i++)
      {
         if (SVDEvents.front()<tcut)
         {
            
         }
      }*/
      return;
   }
   TAFlowEvent* SVDMatchTime(TARunInfo* runinfo,TAFlowEvent* flow)
   {
       std::lock_guard<std::mutex> lock(SVDEventsLock);
       int nSVD=SVDEvents.size();
       std::vector<SVDQOD*> finished_QOD_events;
       for ( int j=0; j<nSVD; j++)
       {
          int n=SISEventRunTime.size();
          SVDQOD* QOD=SVDEvents.front();
          for ( int i =0; i<n; i++)
          {
             double r=ClockRatio(VF48Clock[i],SISClock[i]);
             //std::cout<<"R:"<<r-2.<<std::endl;
             double t=2.*QOD->VF48Timestamp/r;
             //if (t > SISEventRunTime.back() ) 
             //{
             //    std::cout<<"Time saved"<<std::endl;
             //    return;
            // }
             //std::cout <<"SIL: "<<t <<" < " << SISEventRunTime[i] <<std::endl;
             if (t < SISEventRunTime.at(i) )
             {
                //std::cout <<"TEST: "<<t <<" < "<<SISEventRunTime[i]<<std::endl;
                QOD->t=t;
                SaveQODEvent(runinfo,QOD);
                CleanSISEventsBefore(t-0.1);
                finished_QOD_events.push_back(QOD);
                SVDEvents.pop_front();
                break;
             }
             else
             {
                continue;
             }
          }
       }
       //Free memory when timestamps are not aligning nicely (VF48 corruption?)
       CleanOldTimestamps(10.);
       
       int nFinished=finished_QOD_events.size();
       bool had_flow=(bool)flow;
       if (nFinished && flow)
       {
          SVDQODFlow* f=new SVDQODFlow(flow);
          for (int i=0; i<nFinished; i++)
             f->SVDQODEvents.push_back(finished_QOD_events.at(i));
          flow=f;
          if (had_flow) return flow;
          else runinfo->AddToFlowQueue(flow);
       }
       return flow;
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if (fFlags->fNoSync) return flow;
      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif   
      SISEventFlow* SISFlow = flow->Find<SISEventFlow>();
      if (SISFlow)
        {
            //VF48 clock is on SIS0
            std::vector<TSISEvent*>* ce=&SISFlow->sis_events[0];
            for (uint i=0; i<ce->size(); i++)
               {
                  TSISEvent* e=ce->at(i);
                  SISEventRunTime.push_back(e->GetRunTime());
                  SISClock.push_back(e->GetClock());
                  VF48Clock.push_back(e->GetVF48Clock());
                  //if (e->Channel==CHRONO_SYNC_CHANNEL)
               }
         }
      AlphaEventFlow* fe=flow->Find<AlphaEventFlow>();
      if (!fe)
         return flow;
      TAlphaEvent* AlphaEvent=fe->alphaevent;
      SilEventsFlow* sf=flow->Find<SilEventsFlow>();
      if (!sf)
         return flow;
      TSiliconEvent* SiliconEvent=sf->silevent;
      
      SVDQOD* SVD=new SVDQOD(AlphaEvent,SiliconEvent);
      A2OnlineMVAFlow* mva=flow->Find<A2OnlineMVAFlow>();
      if (mva)
         SVD->MVA=(int)mva->pass_online_mva;
      else
         SVD->MVA=-1;
      {
         std::lock_guard<std::mutex> lock(SVDEventsLock);
         SVDEvents.push_back(SVD);
      }
      flow=SVDMatchTime(runinfo,flow);

      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"official_A2_time_module",timer_start);
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("OfficialA2Time::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class OfficialA2TimeFactory: public TAFactory
{
public:
   OfficialA2TimeFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("OfficialA2TimeFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--printcalib")
            fFlags.fPrint = true;
         if (args[i] == "--nosync")
            fFlags.fNoSync = true;
      }
   }

   void Finish()
   {
      printf("OfficialA2TimeFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("OfficialA2TimeFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new OfficialA2Time(runinfo, &fFlags);
   }
};

static TARegister tar(new OfficialA2TimeFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

