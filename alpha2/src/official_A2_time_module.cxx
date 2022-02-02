//
// Module to generate friend trees with 'Official' cross calibrated 
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
   
   // int SVD_channel=-1; Unused

   std::mutex fTimeStampLock;

   std::deque<double> SISEventRunTime;

   std::deque<ULong64_t> SISClock;
   std::deque<ULong64_t> VF48Clock;
   std::deque<TSVD_QOD*> SVDEvents;

   int fVF48Events = 0;

public:
   OfficialA2TimeFlags* fFlags;

   double SVD_TimeStamp;
   TTree* SVDOfficial=NULL;

   bool fTrace = true;

   OfficialA2Time(TARunInfo* runinfo, OfficialA2TimeFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="OfficialA2Time";
#endif
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
      std::cout<<"Total VF48 events given an official time:"<<fVF48Events<<std::endl;
      if (SVDOfficial)
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
   void SaveQODEvent(TARunInfo* runinfo, TSVD_QOD* e)
   {
      if (!SVDOfficial)
      {
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         runinfo->fRoot->fOutputFile->cd();
         SVDOfficial=new TTree("SVDOfficialA2Time","SVDOfficialA2Time");
      }
      TBranch* b_variable = SVDOfficial->GetBranch("OfficialTime");
      if (!b_variable)
         SVDOfficial->Branch("OfficialTime","TSVD_QOD",&e,32000,0);
      else
         SVDOfficial->SetBranchAddress("OfficialTime",&e);
      std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
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
      std::lock_guard<std::mutex> lock(fTimeStampLock);
      if (SISEventRunTime.empty()) return;
      const double LatestTime = SISEventRunTime.back();
      const double tcut= LatestTime - TimeBufferSize;

      const int nSIS = SISEventRunTime.size();
      
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
   std::vector<TSVD_QOD*> GetFinishedEvents(TARunInfo* runinfo)
   {
       const int nSVD = SVDEvents.size();
       std::vector<TSVD_QOD*> finished_QOD_events;
       for ( int j=0; j<nSVD; j++)
       {
          const int n = SISEventRunTime.size();
          TSVD_QOD* QOD = SVDEvents.front();
          for ( int i =0; i < n; i++)
          {
             //There is no clock!!!
             //if (SISClock[i]==0) continue;

             double r=ClockRatio(VF48Clock[i],SISClock[i]);
             
             //std::cout<<"R:"<<r-2.<<std::endl;
             double t=2.*QOD->VF48Timestamp/r;

             if (t <= SISEventRunTime.at(i) )
             {
                //std::cout <<"TEST: "<<t <<" < "<<SISEventRunTime[i]<<std::endl;
                QOD->t=t;
                SaveQODEvent(runinfo,QOD);
                CleanSISEventsBefore(t-0.1);
                finished_QOD_events.push_back(QOD);
                SVDEvents.pop_front();
                fVF48Events++;
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
       return finished_QOD_events;
   }
   TAFlowEvent* SVDMatchTime(TARunInfo* runinfo,TAFlowEvent* flow)
   {
       std::vector<TSVD_QOD*>finished_QOD_events = GetFinishedEvents(runinfo);
       int nFinished=finished_QOD_events.size();
       if (nFinished)
       {
          SVDQODFlow* f=new SVDQODFlow(flow);
          for (int i=0; i<nFinished; i++)
             f->SVDQODEvents.push_back(finished_QOD_events.at(i));
          flow=f;
       }
       return flow;
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      if (fFlags->fNoSync)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      SISEventFlow* SISFlow = flow->Find<SISEventFlow>();
      if (SISFlow)
      {
         //VF48 clock is on SIS0
         const std::vector<TSISEvent> ce = SISFlow->sis_events.at(0);
         std::lock_guard<std::mutex> lock(fTimeStampLock);
         for (uint i = 0; i < ce.size(); i++)
         {
            const TSISEvent& e = ce.at(i);
            SISEventRunTime.push_back(e.GetRunTime());
            SISClock.push_back(e.GetClock());
            VF48Clock.push_back(e.GetVF48Clock());
            //if (e->Channel==CHRONO_SYNC_CHANNEL)
         }
      }

      SilEventFlow* fe=flow->Find<SilEventFlow>();
      if (fe)
      {
         TAlphaEvent* AlphaEvent=fe->alphaevent;
         TSiliconEvent* SiliconEvent=fe->silevent;

         TSVD_QOD* SVD=new TSVD_QOD(AlphaEvent,SiliconEvent);
         A2OnlineMVAFlow* mva=flow->Find<A2OnlineMVAFlow>();
         if (mva)
            SVD->MVA=(int)mva->pass_online_mva;
         else
            SVD->MVA=-1;

         SVDEvents.push_back(SVD);
      }
      //if (SiliconEvent->GetVF48NEvent()%10==0)
      flow=SVDMatchTime(runinfo,flow);
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
      if (fFlags.fPrint)
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

