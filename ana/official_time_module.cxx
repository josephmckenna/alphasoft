//
// Module to generate friend trees with 'Offical' cross calibrated 
// time between all modules (EVB and chronoboxes)
// I.E Convert 'RunTime' to 'Official Time'
// JTK McKENNA
//




#include "manalyzer.h"
#include "midasio.h"
#include "AgFlow.h"

#include "TTree.h"
#include "TChrono_Event.h"
#include <iostream>
#include "chrono_module.h"
#include "TChronoChannelName.h"
#include "AnalysisTimer.h"

class OfficialTimeFlags
{
public:
   bool fPrint = false;
};

class OfficialTime: public TARunObject
{
private:

  std::vector<double> TPCts;
  double TPCZeroTime;
  std::vector<double> Chrono_TPC;
  std::vector<double> ChronoSyncTS[CHRONO_N_BOARDS];
  
  std::vector<ChronoEvent*>* ChronoEventsFlow=NULL;
public:
  OfficialTimeFlags* fFlags;
  TTree* TPCOfficial;
  double TPC_TimeStamp;
  bool fTrace = true;
   
   OfficialTime(TARunInfo* runinfo, OfficialTimeFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("OfficialTime::ctor!\n");
   }

   ~OfficialTime()
   {
      if (fTrace)
         printf("OfficialTime::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OfficialTime::BeginRun, run %d\n", runinfo->fRunNo);
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
      
      TPCOfficial=new TTree("StoreEventOfficialTime","StoreEventOfficialTime");
      TPCOfficial->Branch("OfficalTime",&TPC_TimeStamp, 32000, 0);
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OfficialTime::EndRun, run %d\n", runinfo->fRunNo);
      //Flush out all un written timestamps
      FlushTPCTime();
      TPCOfficial->Write();
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OfficialTime::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   void TPCMatchTime()
   {
      uint ChronoEvents=Chrono_TPC.size();
      uint TPCEvents=TPCts.size();
      if (ChronoEvents==0 || TPCEvents==0)
         return;
      if (TPCEvents>1000)
      {
         FlushTPCTime(500);
         //Clean up other vector
         TPCts.erase(TPCts.begin(),TPCts.begin()+500);
         Chrono_TPC.erase (Chrono_TPC.begin(),Chrono_TPC.begin()+500);

      }
      //Find smaller arrays
      int Events;
      if (ChronoEvents>TPCEvents)
         Events=TPCEvents;
      else
         Events=ChronoEvents;
      if (fFlags->fPrint)
         {
            std::cout <<"CALIB SIZE"<<ChronoEvents<<"\t"<<TPCEvents<<"   "<<Events<<std::endl;
            for (int  i=0; i<Events-1; i++)
               {
                  //std::cout<<"CALIB   "<<i<<":"<<Chrono_TPC.at(i)<<" - "<<TPCts.at(i)<<" = "<< Chrono_TPC[i]-TPCts[i]<<std::endl;
                  std::cout<<"CALIB   "<<i<<":"<<Chrono_TPC[i]<<" - "<<TPCts[i]<<" = "<< Chrono_TPC[i]-TPCts[i]<<std::endl;
               }
         }
   }

   void FlushTPCTime(int nToFlush=-1)
   {
      if (nToFlush<0) nToFlush=TPCts.size();
      std::cout <<"Flushing TPC time ("<<nToFlush<<" events)"<<std::endl;
      for (int i=0; i<nToFlush; i++)
         {
            //Use spline of time here, not the vector:
            //TPC_TimeStamp=Chrono_TPC.at(i);
            TPC_TimeStamp=Chrono_TPC[i];
            TPCOfficial->Fill();
         }
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      //printf("Analyze, run %d, event serno %d, id 0x%04x, data size %d\n", runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      std::cout<<"OfficialTime::Analyze   Event # "<<me->serial_number<<std::endl;

      //if( me->event_id != 10 ) // sequencer event id
      //   return flow;
      //AgSignalsFlow* SigFlow = flow->Find<AgSignalsFlow>();
      //if( !SigFlow ) return flow;
      
      AgChronoFlow* ChronoFlow = flow->Find<AgChronoFlow>();
      if (ChronoFlow)
         {
            std::vector<ChronoEvent*>* ce=ChronoFlow->events;
            for (uint i=0; i<ce->size(); i++)
               {
                  ChronoEvent* e=ce->at(i);
                  if (e->Channel==CHRONO_SYNC_CHANNEL)
                     ChronoSyncTS[e->ChronoBoard].push_back(e->RunTime);
                  //if TPC trigger... add it too
                  if (e->Channel==CHRONO_N_TS_CHANNELS && e->ChronoBoard==0)
                     {
                        Chrono_TPC.push_back(e->RunTime);
                        //if (Chrono_TPC.size()==1) TPCZeroTime=e->RunTime();
                     }
               }
         }
      AgEventFlow *ef = flow->Find<AgEventFlow>();
      if (ef && ef->fEvent)
      {
         AgEvent* age = ef->fEvent;
         TPCts.push_back(age->time);
 
         TPCMatchTime();
      }
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"official_time_module");
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("OfficialTime::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class OfficialTimeFactory: public TAFactory
{
public:
   OfficialTimeFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("OfficialTimeFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--printcalib")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      printf("OfficialTimeFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("OfficialTimeFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new OfficialTime(runinfo, &fFlags);
   }
};

static TARegister tar(new OfficialTimeFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

