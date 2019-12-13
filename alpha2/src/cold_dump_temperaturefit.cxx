//
// Slow down flow to real time (testing module)
//
// JTK McKENNA
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include "A2Flow.h"
#include "TSISChannels.h"
#include "TSISEvent.h"

#include "AnalysisTimer.h"
#include <iostream>
class ColdDumpTemperatureFitModuleFlags
{
public:
   bool fPrint = false;

};
class ColdDumpTemperatureFitModule: public TARunObject
{
private:
   std::deque<A2Spill*> ColdDumps;
   
   int possible_channels[5];

  std::vector<double> DumpTimes;
  std::vector<int>    DumpCounts;
  
  std::vector<double> BackgroundTimes;
  std::vector<int>    BackgroundCounts;

public:
   ColdDumpTemperatureFitModuleFlags* fFlags;
   bool fTrace = false;
   
   
   ColdDumpTemperatureFitModule(TARunInfo* runinfo, ColdDumpTemperatureFitModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("ColdDumpTemperatureFitModule::ctor!\n");
      // for fit!
//      loadPsiTable();
   }

   ~ColdDumpTemperatureFitModule()
   {
      if (fTrace)
         printf("ColdDumpTemperatureFitModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ColdDumpTemperatureFitModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      TSISChannels* SISChannels=new TSISChannels( runinfo->fRunNo );
      possible_channels[0]=42; 
      possible_channels[1]=SISChannels->GetChannel("SIS_PMT_ATOM_OR");
      possible_channels[2]=SISChannels->GetChannel("SIS_PMT_ATOM_AND");
      possible_channels[3]=SISChannels->GetChannel("SIS_PMT_CATCH_OR");
      possible_channels[4]=SISChannels->GetChannel("SIS_PMT_CATCH_AND");
      
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ColdDumpTemperatureFitModule::EndRun, run %d\n", runinfo->fRunNo);
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ColdDumpTemperatureFitModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
  {
      #ifdef _TIME_ANALYSIS_
      START_TIMER
      #endif
      A2SpillFlow* SpillFlow= flow->Find<A2SpillFlow>();
      if (SpillFlow)
      {
         for (size_t i=0; i<SpillFlow->spill_events.size(); i++)
         {
            A2Spill* s=SpillFlow->spill_events.at(i);
            //s->Print();
            if (!s->SeqData) continue;
            int thisSeq=s->SeqData->SequenceNum;
            if (thisSeq==0) //Catching trap
            //if (strcmp(s->SeqName.c_str(),"cat")==0)
            if (strcmp(s->Name.c_str(),"\"Cold Dump\"")==0)
            {
               A2Spill* ColdDump=new A2Spill(s);
               A2ScalerData* sc=ColdDump->ScalerData;
               //Find which channels had the most counts in... 
               
               int highest_channel = -1;
               int highest_counts = 0;
  
               for (int i = 0; i < 5; i++)
               {
                  int ch = possible_channels[i];
                  int ch_counts = sc->DetectorCounts[ch];
                  printf("channel %d has %d counts\n", ch, ch_counts);
                  if (ch_counts > highest_counts)
                  {
                     highest_counts = ch_counts;
                     highest_channel = ch;
                  }
               }
               if (highest_counts == 0)
                  printf("none of the channels seem to work!");
                  
               //Fill array of doubles#
               
               double tmin=sc->StartTime;
               double tmax=sc->StopTime;
               int size=SpillFlow->SIS_Events[highest_channel].size();
               for (int i=0; i<size; i++)
               {
                  double run_time=SpillFlow->SIS_Events[highest_channel][i].t;
                  if (run_time <= tmin) continue; //Data too early... skip
                  if (run_time <= tmax) { //Is inside the dump
                     // printf("%f,%d\n", det_event->GetRunTime(),det_event->GetCountsInChannel());         
                     DumpTimes.push_back(run_time);
                     DumpCounts.push_back(SpillFlow->SIS_Events[highest_channel][i].counts);
                  }
                  else //I am after the dump... so I am background data (run_time>tmax)
                  {
                     BackgroundTimes.push_back(run_time);
                     BackgroundCounts.push_back(SpillFlow->SIS_Events[highest_channel][i].counts);
                  }
               }
               std::cout<<"I have "<<DumpTimes.size()<<" sis bins for temperature fit, and "<<BackgroundTimes.size()<<" bins for background fitting"<<std::endl;
               ColdDumps.push_back(ColdDump);
            }
            
         }
      }
      if (ColdDumps.size())
      {
         
      }
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"catch_efficiency_module",timer_start);
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
         printf("ColdDumpTemperatureFitModule::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class ColdDumpTemperatureFitModuleFactory: public TAFactory
{
public:
   ColdDumpTemperatureFitModuleFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("ColdDumpTemperatureFitModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("ColdDumpTemperatureFitModuleFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("ColdDumpTemperatureFitModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new ColdDumpTemperatureFitModule(runinfo, &fFlags);
   }
};

static TARegister tar(new ColdDumpTemperatureFitModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
