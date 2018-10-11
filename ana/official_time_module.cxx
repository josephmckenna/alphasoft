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
      Chrono_TPC.clear();
      TPCts.clear();
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("OfficialTime::EndRun, run %d\n", runinfo->fRunNo);

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
         
      //Find smaller arrays
      int Events;
      if (ChronoEvents>TPCEvents)
         Events=TPCEvents;
      else
         Events=ChronoEvents;
      std::cout <<"CALIB SIZE"<<ChronoEvents<<"\t"<<TPCEvents<<"   "<<Events<<std::endl;

      for (int  i=0; i<Events-1; i++)
      {
         //std::cout<<"CALIB   "<<i<<":"<<Chrono_TPC.at(i)<<" - "<<TPCts.at(i)<<" = "<< Chrono_TPC[i]-TPCts[i]<<std::endl;
         std::cout<<"CALIB   "<<i<<":"<<Chrono_TPC[i]<<" - "<<TPCts[i]<<" = "<< Chrono_TPC[i]-TPCts[i]<<std::endl;
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
                     Chrono_TPC.push_back(e->RunTime);
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
         if (args[i] == "--print")
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

