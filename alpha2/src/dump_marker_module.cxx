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
#include "AnalysisTimer.h"
#include <iostream>
class DumpMakerModuleFlags
{
public:
   bool fPrint = false;
};
TString SeqNames[NUMSEQ]={"cat","rct","atm","pos","rct_botg","atm_botg","atm_topg","rct_topg","bml"};

TString StartNames[NUMSEQ]={"SIS_PBAR_DUMP_START","SIS_RECATCH_DUMP_START","SIS_ATOM_DUMP_START","SIS_POS_DUMP_START","NA","NA","NA","NA","NA"};
TString StopNames[NUMSEQ] ={"SIS_PBAR_DUMP_STOP", "SIS_RECATCH_DUMP_STOP", "SIS_ATOM_DUMP_STOP", "SIS_POS_DUMP_STOP","NA","NA","NA","NA","NA"};

struct SIS_Counts
{
   double t;
   int counts;
};
struct SVD_Counts
{
   double t;
   bool passed_cuts;
   bool online_mva;
};
bool compareRunTime(SIS_Counts* a, SIS_Counts* b) { return (a->t < b->t); }
class DumpMakerModule: public TARunObject
{
private:
   double LastSISTS=0;
public:
   DumpMakerModuleFlags* fFlags;
   bool fTrace = false;
   std::deque<A2Spill*> IncompleteDumps;

   std::deque<SIS_Counts*> SIS_Events[64];
   std::deque<SVD_Counts*> SVD_Events;

   int DumpStartChannels[USED_SEQ] ={-1};
   int DumpStopChannels[USED_SEQ]  ={-1};
   int detectorCh[MAXDET];
   TString detectorName[MAXDET];
   DumpMakerModule(TARunInfo* runinfo, DumpMakerModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("DumpMakerModule::ctor!\n");
      TSISChannels* SISChannels=new TSISChannels( runinfo->fRunNo );
      for (int j=0; j<USED_SEQ; j++) 
      {
         DumpStartChannels[j] =SISChannels->GetChannel(StartNames[j],runinfo->fRunNo);
         DumpStopChannels[j]  =SISChannels->GetChannel(StopNames[j], runinfo->fRunNo);
      }

      detectorCh[0] = SISChannels->GetChannel("SIS_PMT_CATCH_OR");
      detectorName[0] = "CATCH_OR";
      detectorCh[1] = SISChannels->GetChannel("SIS_PMT_CATCH_AND");
      detectorName[1] = "CATCH_AND";
      detectorCh[2] = SISChannels->GetChannel("SIS_PMT_ATOM_OR");
      detectorName[2] = "ATOM_OR";
      detectorCh[3] = SISChannels->GetChannel("SIS_PMT_ATOM_AND");
      detectorName[3] = "ATOM_AND";
      detectorCh[4] = SISChannels->GetChannel("PMT_12_AND_13");
      detectorName[4] = "CTSTICK";
      detectorCh[5] = SISChannels->GetChannel("IO32_TRIG_NOBUSY");
      detectorName[5] = "IO32_TRIG";
      detectorCh[6] = 42;//gSISChannels->GetChannel("SIS_PMT_10_AND_PMT_11");
      //detectorCh[6] = 40;//gSISChannels->GetChannel("PMT_10");
      detectorName[6] = "ATOMSTICK";
      detectorCh[7] = 47;
      detectorName[7] = "NewATOMSTICK";
      //detectorName[6] = "ATOMSTICK (PMT9)";

      delete SISChannels;
   }

   ~DumpMakerModule()
   {
      if (fTrace)
         printf("DumpMakerModule::dtor!\n");
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("DumpMakerModule::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
    }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("DumpMakerModule::EndRun, run %d\n", runinfo->fRunNo);
         
      if (IncompleteDumps.size())
         printf("Error: Incomplete dumps!!!");
   }
   
   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("DumpMakerModule::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }
   
   void FillActiveDumpsWithSVD()
   {
      int n=IncompleteDumps.size();
      for (size_t j=0; j<SVD_Events.size(); j++)
      {
         SVD_Counts* SV=SVD_Events.at(j);
         if (!SV) continue;
         //if (SV->t> LastSISTS) break;
         bool EventUsed=false;
         for (int i=0; i<n; i++)
         {
            A2Spill* s=IncompleteDumps.at(i);
            if (!s) continue;
            //if (!s->SISFilled) continue;
            if (s->SVDFilled) continue;
            //s->Print();
            //if ( s->StopTime>0)
            //std::cout<<"SIL EVENT:"<<SV->passed_cuts<< "\tt:"<< SV->t <<">"s->StopTime <<std::endl;
            
            if (SV->t>s->StopTime && s->StopTime>0)
            {
               s->SVDFilled=true;
               continue;
            }
            
            if (SV->t>s->StartTime)
            {
                s->PassCuts+=SV->passed_cuts;
                s->PassMVA+=SV->online_mva;
                EventUsed=true;
            }
         }
         if (EventUsed)
         {
            delete SV;
            SVD_Events.at(j)=NULL;
         }
      }
   }
   void FillCompleteDumpsWithSIS()
   {
      int n=IncompleteDumps.size();
      for (int k=0; k<64; k++)
      {
         //if (detectorCh[k]<=0) continue;
         for (size_t j=0; j<SIS_Events[k].size(); j++)
         {
            SIS_Counts* SC=SIS_Events[k].at(j);
            //SIS_Counts* SC=SIS_Events[k].front();
            if (!SC) continue;
            //bool EventUsed=false;
            for (int i=0; i<n; i++)
            {
               A2Spill* s=IncompleteDumps.at(i);
               if (!s) continue;
               if (s->SISFilled && k==0) continue;
               if (s->StopTime<=0) continue;
               if (SC->t>s->StopTime && s->StopTime>0)
               {
                  s->SISFilled=true;
                  continue;
               }
               if (SC->t>=s->StartTime)
               {
                  s->DetectorCounts[k]+=SC->counts;
                  //EventUsed=true;
               }
            }
         }
      }
   }

   
   TAFlowEvent* FindFinishedSpills(TAFlowEvent* flow)
   {
      std::vector<A2Spill*> finished;
      
      //Gather finished Spills
      int nIncomplete=IncompleteDumps.size();
      for (int i=0; i<nIncomplete;i++)
      {
         A2Spill* a=IncompleteDumps.at(i);
         if (!a) continue;
         if (a->Ready())
         {
           IncompleteDumps.at(i)=NULL;
           finished.push_back(a);
         }
      }

      //Clear the front of the Incomplete dumps deque (tidy memory)
      for (int i=0; i<nIncomplete;i++)
      {
         if (!IncompleteDumps.front())
            IncompleteDumps.pop_front();
         else
         break;
      }

      //Put finished spills into the flow
      int nFinished=finished.size();
      if (nFinished)
      {
         A2SpillFlow* f=new A2SpillFlow(flow);
         for (int i=0; i<nFinished; i++)
         {
            A2Spill* a=finished.at(i);
            f->spill_events.push_back(a);
         }
         flow=f;
         finished.clear();
      }
      return flow;
   }
   void PrintHeldEvents()
   {
      std::cout<<"Held SIS Events:"<<std::endl;
      for (int i=0; i<64; i++)
         std::cout<<i<<"\t"<<SIS_Events[i].size()<<std::endl;
      std::cout<<"Held SVD Events:"<< SVD_Events.size()<<std::endl;
   }
   void PrintActiveSpills()
   {
      int nIncomplete=IncompleteDumps.size();
      for (int i=0; i<nIncomplete;i++)
      {
         A2Spill* a=IncompleteDumps.at(i);
         if (a)
            a->Print();
      }
   }
   void FreeMemory()
   {
      int SIS_TOTAL=0;
      for (int i=0; i<64; i++)
         SIS_TOTAL+=SIS_Events[i].size();
      if (SIS_TOTAL<1000 && SVD_Events.size()<1000) return;

      double tmin=9999999.;
      bool min_time_found=false;
      int nIncomplete=IncompleteDumps.size();
      //std::cout<<"Incomplete dumps:"<<nIncomplete<<std::endl;
      for (int i=0; i<nIncomplete;i++)
      {
         A2Spill* a=IncompleteDumps.at(i);
         if (!a) continue;
         double start=a->StartTime;
         if (start<=0) continue;
         if (start<tmin)
         {
            tmin=start;
            min_time_found=true;
         }
      }
      //If there is a start in a dump, 
      //we dont need any data before the first one...
      double last_ts=0;
      if (min_time_found)
      {
         last_ts=tmin;
      }
      else
      //There are no start markers...
      //so we can delete all SIS events, 
      //and all SVD events up until the latest SIS event
      {
         for (int i=0; i<64; i++)
         {
            if (!SIS_Events[i].size()) continue;
            if (!SIS_Events[i].back()) continue;
            double time=SIS_Events[i].back()->t;
            //std::cout<<"TIME:"<<time<<std::endl;
            if (time > last_ts) last_ts=time;
         }
      }
      int sis_events_cleaned=0;
      for (int i=0; i<64; i++)
      {
         for(size_t j=0; j<SIS_Events[i].size(); j++)
         {
            //This is NULL pointer... its been used... 
            if (!SIS_Events[i].front())
            {
               SIS_Events[i].pop_front();
               continue;
            }
            //std::cout<<j<<"/"<<SIS_Events[i].size()<<"\t\t"<<SIS_Events[i].front()->t <<"<"<< last_ts <<std::endl;
            if (SIS_Events[i].front()->t < last_ts) 
            {
               delete SIS_Events[i].front();
               SIS_Events[i].pop_front();
               sis_events_cleaned++;
            }
            else break;
         }
      }
      int svd_events_cleaned=0;
      for (size_t i=0; SVD_Events.size(); i++)
      {
         if (!SVD_Events.front())
         {
            SVD_Events.pop_front();
            continue;
         }
         if (SVD_Events.front()->t < last_ts)
         {
            //std::cout<<i<<"/"<<SVD_Events.size()<<std::endl;
            delete SVD_Events.front();
            SVD_Events.pop_front();
            svd_events_cleaned++;
          }
          else break;
      }
      //if (sis_events_cleaned)
      //   std::cout<<sis_events_cleaned<<" SIS events freed"<<std::endl;
      //if (svd_events_cleaned)
      //   std::cout<<svd_events_cleaned<<" SVD events freed"<<std::endl;
      return;
   }
   void AddStartDumpMarkers(TSISEvent* e)
   {
      //Process start dump markers:
      for (int j=0; j<USED_SEQ; j++)
      {
         //Only use valid channel numbers
         if (DumpStartChannels[j]<=0) continue;
         int counts=e->GetCountsInChannel(DumpStartChannels[j]);
         if (!counts) continue;
         A2Spill* spill=new A2Spill();
         spill->RunNumber=e->GetRunNumber();
         spill->Unixtime=e->GetMidasUnixTime();
         spill->SequenceNum=j;
         spill->StartTime=e->GetRunTime();
         IncompleteDumps.push_back(spill);
      }
      return;
   }
   void AddStopDumpMarkers(TSISEvent* e)
   {
      //Process stop dump markers
      for (int j=0; j<USED_SEQ; j++)
      {
         //Only use valid channel numbers
         if (DumpStopChannels[j]<=0) continue;
         int counts=e->GetCountsInChannel(DumpStopChannels[j]);
         if (!counts) continue;
         for (size_t k=0; k<IncompleteDumps.size(); k++)
         {
            A2Spill* spill=IncompleteDumps.at(k);
            if (!spill) continue;
            if (spill->StopTime>0) continue;
            if (spill->SequenceNum==j)
            {
               spill->StopTime=e->GetRunTime();
            }
         }
      }
      return;
   }
   void QueueDetectorCounts(TSISEvent* e)
   {
      for (int j=0; j<64; j++)
      {
         //Only use valid channel numbers
         //if (detectorCh[j]<=0) continue;
         //int counts=e->GetCountsInChannel(detectorCh[j]);
         int counts=e->GetCountsInChannel(j);
         //No counts aren't interesting
         if (!counts) continue;
         //Save counts
         SIS_Counts* SC=new SIS_Counts();
         SC->t=e->GetRunTime();
         SC->counts=counts;
         SIS_Events[j].push_back(SC);
      }
   }

   void SortQueuedSISEvents()
   {
      for (int j=0; j<64; j++)
      {
         std::sort(SIS_Events[j].begin(),SIS_Events[j].end(),compareRunTime);
      }
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SISEventFlow* SISFlow = flow->Find<SISEventFlow>();
      if (SISFlow)
      {
         for (int j=0; j<NUM_SIS_MODULES; j++)
         {
            std::vector<TSISEvent*>* ce=&SISFlow->sis_events[j];
            for (uint i=0; i<ce->size(); i++)
            {
              TSISEvent* e=ce->at(i);
              AddStartDumpMarkers(e);
              AddStopDumpMarkers(e);
              QueueDetectorCounts(e);
            }
         }
         SortQueuedSISEvents();
         //Start and stop markers are prepared... Fill all dumps with counts
         //FillActiveDumpsWithSIS();
         FillCompleteDumpsWithSIS();
      }

      SVDQODFlow* QODFlow=flow->Find<SVDQODFlow>();
      if (QODFlow)
      {
         for (uint i=0; i<QODFlow->SVDQODEvents.size(); i++)
         {
            SVDQOD* q=QODFlow->SVDQODEvents.at(i);
            SVD_Counts* SV=new SVD_Counts();
            SV->t=q->t;
            SV->passed_cuts=q->NPassedCuts;
            SV->online_mva=q->MVA;
            SVD_Events.push_back(SV);
         }
      }

      FillActiveDumpsWithSVD();

      //PrintActiveSpills();
      FreeMemory();
      flow=FindFinishedSpills(flow);
      return flow; 
   }

};

class DumpMakerModuleFactory: public TAFactory
{
public:
   DumpMakerModuleFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("DumpMakerModuleFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
      }
   }

   void Finish()
   {
      printf("DumpMakerModuleFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("DumpMakerModuleFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new DumpMakerModule(runinfo, &fFlags);
   }
};

static TARegister tar(new DumpMakerModuleFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
