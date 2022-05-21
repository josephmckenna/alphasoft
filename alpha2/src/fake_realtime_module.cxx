//
// Slow down flow to real time (testing module)
//
// JTK McKENNA
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"

//usleep
#include "unistd.h"
#include <time.h>
#include <iostream>
#include <numeric>
#include <deque>
#include <functional>
class RealTimeModuleFlags
{
public:
   bool fPrint = false;
   bool fFakeRealtime = false;
};
class RealTimeModule: public TARunObject
{
private:
   time_t fStartAnalysisTime;
   time_t fODBStartTime;
   uint32_t FirstEvent = 0;
   uint32_t LastMidasTimeStamp = 0;
   uint32_t MidasEventsInLastSecond = 0;
   std::deque<int> MidasEventsPerSecond;
public:
   RealTimeModuleFlags* fFlags;
   bool fTrace = false;
   
   
   RealTimeModule(TARunInfo* runinfo, RealTimeModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      fModuleName="Fake Real Time Module";
      if (fTrace)
         printf("RealTimeModule::ctor!\n");
   }

   ~RealTimeModule()
   {
      if (fTrace)
         printf("RealTimeModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("RealTimeModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      fStartAnalysisTime = time(NULL);
#ifdef INCLUDE_VirtualOdb_H
      fODBStartTime = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
#endif
#ifdef INCLUDE_MVODB_H
      runinfo->fOdb->RU32("Runinfo/Start time binary",(uint32_t*) &fODBStartTime);
#endif
    }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("RealTimeModule::EndRun, run %d\n", runinfo->fRunNo);
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("RealTimeModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
  {
      return flow; 
  }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      if (fFlags->fFakeRealtime )
      {
         if(!FirstEvent)
            FirstEvent = me->time_stamp;
         
         if (me->time_stamp != LastMidasTimeStamp)
         {
            LastMidasTimeStamp = me->time_stamp ;
            MidasEventsPerSecond.emplace_back(MidasEventsInLastSecond);
            //std::cout <<MidasEventsInLastSecond << "\n";
            MidasEventsInLastSecond = 0;
            if (MidasEventsPerSecond.size() > 10 )
               MidasEventsPerSecond.pop_front();
         }
         else
         {
            MidasEventsInLastSecond++;
         }
        /* if (MidasEventsPerSecond.size() == 10 )
         {
            double average_events_per_second = std::accumulate(MidasEventsPerSecond.begin(), MidasEventsPerSecond.end(),0) / MidasEventsPerSecond.size();
            //std::cout <<"\t" << average_events_per_second <<"\n";
            //std::cout <<"sleep for " <<  1000 / average_events_per_second - 10 <<std::endl;
            usleep ( 0.9 * 1000 * 1000 / average_events_per_second );
         }*/
         while (true)
         {
#ifdef HAVE_THTTP_SERVER
      if ( runinfo->fRoot->fgHttpServer ) {
            runinfo->fRoot->fgHttpServer->ProcessRequests();
      }
#endif
            // Time difference between experiment start and analysis start
            int dt = fStartAnalysisTime - fODBStartTime;

            // Time difference between experiment start and now
            int dt_now = time(NULL) - fODBStartTime;
            
            // Run time in seconds
            int midas_time = me->time_stamp - fODBStartTime;

            // We've waited long enough
            if (midas_time < dt_now - dt)
               break;
            //if (midas_time >= 0)
            //  std::cout <<midas_time << " > " << dt_now << " - " << dt << std::endl;
            usleep(1000);
         }
      }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("RealTimeModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class RealTimeModuleFactory: public TAFactory
{
public:
   RealTimeModuleFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("RealTimeModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--fakerealtime")
            fFlags.fFakeRealtime = true;
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("RealTimeModuleFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("RealTimeModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new RealTimeModule(runinfo, &fFlags);
   }
};

static TARegister tar(new RealTimeModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
