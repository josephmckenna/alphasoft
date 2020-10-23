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

#include "AnalysisTimer.h"
#include <iostream>
class RealTimeModuleFlags
{
public:
   bool fPrint = false;
   bool fFakeRealtime = false;
};
class RealTimeModule: public TARunObject
{
private:
   clock_t start_time;
   uint32_t FirstEvent=0;
public:
   RealTimeModuleFlags* fFlags;
   bool fTrace = false;
   
   
   RealTimeModule(TARunInfo* runinfo, RealTimeModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      ModuleName="Fake Real Time Module";
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
      start_time=clock();
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
            FirstEvent=me->time_stamp;
         clock_t time_now=(clock()-start_time)/CLOCKS_PER_SEC;
         //std::cout << me->time_stamp-FirstEvent <<"<"<<time_now<<std::endl;
         while (me->time_stamp-FirstEvent>time_now)
         {
            //std::cout<<"Sleep"<<std::endl;
            //std::cout << me->time_stamp <<" > "<<time_now<<std::endl;
            time_now=(clock()-start_time)/CLOCKS_PER_SEC;
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
