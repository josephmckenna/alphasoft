//
// Slow down flow to real time (testing module)
//
// JTK McKENNA
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include "TSpill.h"
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


bool compareRunTime(SIS_Counts* a, SIS_Counts* b) { return (a->t < b->t); }
class DumpMakerModule: public TARunObject
{
private:
   double LastSISTS=0;
   static const int MAXDET=10;
public:
   DumpMakerModuleFlags* fFlags;
   bool fTrace = false;
   std::deque<TA2Spill*> IncompleteDumps;

   std::deque<SIS_Counts*> SIS_Events[64];
   std::deque<SVD_Counts*> SVD_Events;

   int DumpStartChannels[USED_SEQ] ={-1};
   int DumpStopChannels[USED_SEQ]  ={-1};
   int detectorCh[MAXDET];
   TString detectorName[MAXDET];
   
   bool have_svd_events = false;
   
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
      delete SISChannels;
   }

   ~DumpMakerModule()
   {
      if (fTrace)
         printf("DumpMakerModule::dtor!\n");
      for (int j=0; j<64; j++)
      {
        int n=SIS_Events[j].size();
        for (int i=0; i<n; i++)
           delete SIS_Events[j].at(i);
        SIS_Events[j].clear();
      }
      int n=SVD_Events.size();
      for (int i=0; i<n; i++)
         delete SVD_Events[i];
      SVD_Events.clear();
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
      if (!SVD_Events.size()) return;
      for (int i=0; i<n; i++)
      {
         TA2Spill* s=IncompleteDumps.at(i);
         if (!s) continue;
         TA2SpillScalerData* sc=s->ScalerData;
         if (!sc) continue;
         if (sc->SVDFilled) continue;
         //Only fill events after the dump is over
         if (sc->StopTime<=0) continue;
         //if (SVD_Events.size()<10) continue;
         //Check we have enough SVD events to fill the event in one pass
         SVD_Counts* back=SVD_Events.back();
         if (back->t <= sc->StopTime && back->t>0) continue;
         //Fill spill
         for (size_t j=0; j<SVD_Events.size(); j++)
         {
            SVD_Counts* SV=SVD_Events.at(j);
            if (!SV) continue;
            if (SV->t < sc->StartTime) continue;
            if (SV->t > sc->StopTime) break;
            sc->AddData(*SV);
         }
         sc->SVDFilled=true;
      }
   }
   void FillCompleteDumpsWithSIS()
   {
      int n=IncompleteDumps.size();
      for (int i=0; i<n; i++)
      {
         TA2Spill* s=IncompleteDumps.at(i);
         if (!s) continue;
         TA2SpillScalerData* sc=s->ScalerData;
         if (!sc) continue;
         //If all SIS channels set (all bits true in unsigned long )
         if (sc->SISFilled & (unsigned long) -1)/* && k==0)*/ continue;
         if (sc->StopTime<=0) continue;
         //Fill events with start and stop times
         for (int k=0; k<64; k++)
         {
            for (size_t j=0; j<SIS_Events[k].size(); j++)
            {
               SIS_Counts* SC=SIS_Events[k].at(j);
               //SIS_Counts* SC=SIS_Events[k].front();
               if (sc->SISFilled & 1UL<<k ) continue;
               if (!SC) continue;
               if (SC->t > sc->StopTime )
               {
                  sc->SISFilled+=1UL<<k;
                  break;
               }
               if (SC->t>=sc->StartTime)
               {
                  sc->AddData(*SC,k);
                  //EventUsed=true;
               }
            }
         }
      }
   }

   
   TAFlowEvent* FindFinishedSpills(TAFlowEvent* flow)
   {
      std::vector<TA2Spill*> finished;
      
      //Gather finished Spills
      int nIncomplete=IncompleteDumps.size();
      for (int i=0; i<nIncomplete;i++)
      {
         TA2Spill* a=IncompleteDumps.at(i);
         if (!a) continue;
         if (a->Ready(have_svd_events))
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
         //Calculate the time range of these dumps while putting them into the flow
         double tmin=999999999999999999999999999.;
         double tmax=0;
         for (int i=0; i<nFinished; i++)
         {
            TA2Spill* a=finished.at(i);
            TA2SpillScalerData* d=a->ScalerData;
            if (d->StartTime > 0 && 
                d->StartTime < tmin)
               tmin=d->StartTime;
            if (d->StopTime > tmax)
               tmax=d->StopTime;
            f->spill_events.push_back(a);
         }
         // Send copies of the SVD_Counts and SIS_Counts within
         // the dump range with the flow (so other modules can use it for
         // analysis, eg temperature fits)
         int size=SVD_Events.size();
         for (int i=0; i<size; i++)
         {
            if (SVD_Events[i]->t<tmin) continue;
            if (SVD_Events[i]->t>tmax+1) break;
            f->SVD_Events.push_back(SVD_Counts(*SVD_Events[i]));
         }
         for (int i=0; i<64; i++)
         {
            size=SIS_Events[i].size();
            for (int j=0; j<size; j++)
            {
               double t=SIS_Events[i][j]->t;
               if (t<tmin) continue;
               if (t>tmax+1) break;
               f->SIS_Events[i].push_back(SIS_Counts(*SIS_Events[i][j]));
            }
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
         TA2Spill* a=IncompleteDumps.at(i);
         if (a)
            a->Print();
      }
   }
   void FreeMemory(bool have_svd_events)
   {
      int SIS_TOTAL=0;
      for (int i=0; i<64; i++)
         SIS_TOTAL+=SIS_Events[i].size();
      if (SIS_TOTAL<1000) return;
      if ( have_svd_events && SVD_Events.size()<1000) return;
      double tmin=9999999.;
      bool min_time_found=false;
      int nIncomplete=IncompleteDumps.size();
      if (!nIncomplete) min_time_found=true; //There are no imcomplete dumps... tmin is found to be infinite

      //std::cout<<"Incomplete dumps:"<<nIncomplete<<std::endl;
      for (int i=0; i<nIncomplete;i++)
      {
         TA2Spill* a=IncompleteDumps.at(i);
         if (!a) continue;
         if (!a->ScalerData) continue;
         double start=a->ScalerData->StartTime;
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
               SISSortedUpToHere[i]--;
               continue;
            }
            //std::cout<<j<<"/"<<SIS_Events[i].size()<<"\t\t"<<SIS_Events[i].front()->t <<"<"<< last_ts <<std::endl;
            if (SIS_Events[i].front()->t < last_ts) 
            {
               delete SIS_Events[i].front();
               SIS_Events[i].pop_front();
               SISSortedUpToHere[i]--;
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
         if (SVD_Events.front()->t < last_ts-10.)
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
         TA2Spill* spill=new TA2Spill();
         spill->RunNumber=e->GetRunNumber();
         spill->Unixtime=e->GetMidasUnixTime();
         if (!spill->SeqData)
            spill->SeqData=new TSpillSequencerData();
         spill->SeqData->fSequenceNum=j;
         if (!spill->ScalerData)
            spill->ScalerData=new TA2SpillScalerData();
         spill->ScalerData->StartTime=e->GetRunTime();
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
            TA2Spill* spill=IncompleteDumps.at(k);
            if (!spill) continue;
            if (!spill->ScalerData) continue;
            if (spill->ScalerData->StopTime>0) continue;
            if (!spill->SeqData) continue;
            if (spill->SeqData->fSequenceNum==j)
            {
               spill->ScalerData->StopTime=e->GetRunTime();
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
         LastSISTS=e->GetRunTime();
      }
   }
   int SISSortedUpToHere[64]={0};
   void SortQueuedSISEvents()
   {
      for (int j=0; j<64; j++)
      {
         std::sort(SIS_Events[j].begin()+SISSortedUpToHere[j],SIS_Events[j].end(),compareRunTime);
         SISSortedUpToHere[j]=SIS_Events[j].size();
      }
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      #ifdef _TIME_ANALYSIS_
      START_TIMER
      #endif
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
         // Dont finish spill events until we fill it with SVD data... 
         // ...we have SVD data yay!
         have_svd_events=true;
         for (uint i=0; i<QODFlow->SVDQODEvents.size(); i++)
         {
            TSVD_QOD* q=QODFlow->SVDQODEvents.at(i);
            SVD_Counts* SV=new SVD_Counts();
            SV->VF48EventNo=q->VF48NEvent;
            SV->t=q->t;
            SV->has_vertex=q->NVertices;
            SV->passed_cuts=q->NPassedCuts;
            SV->online_mva=q->MVA;
            SVD_Events.push_back(SV);
         }
      }

      FillActiveDumpsWithSVD();

      // PrintActiveSpills();
      FreeMemory(have_svd_events);
      flow=FindFinishedSpills(flow);

      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"dumper_marker_module",timer_start);
      #endif

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
      if (fFlags.fPrint)
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
