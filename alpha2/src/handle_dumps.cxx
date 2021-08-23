//
// Slow down flow to real time (testing module)
//
// JTK McKENNA
//

#include <stdio.h>

#include "manalyzer.h"
#include "midasio.h"
#include "TSISEvent.h"
#include "TSpill.h"
#include "RecoFlow.h"
#include "A2Flow.h"
#include "TSISChannels.h"
#include "DumpHandling.h"
#include "GEM_BANK_flow.h"
#include <iostream>
class DumpMakerModuleFlags
{
public:
   bool fPrint = false;
};

TString StartNames[NUMSEQ]={"SIS_PBAR_DUMP_START","SIS_RECATCH_DUMP_START","SIS_ATOM_DUMP_START","SIS_POS_DUMP_START","NA","NA","NA","NA","NA"};
TString StopNames[NUMSEQ] ={"SIS_PBAR_DUMP_STOP", "SIS_RECATCH_DUMP_STOP", "SIS_ATOM_DUMP_STOP", "SIS_POS_DUMP_STOP","NA","NA","NA","NA","NA"};


class DumpMakerModule: public TARunObject
{
private:
   double LastSISTS=0;
   static const int MAXDET=10;
public:
   DumpMakerModuleFlags* fFlags;
   bool fTrace = false;
   std::deque<TA2Spill*> IncompleteDumps;

   int DumpStartChannels[USED_SEQ] ={-1};
   int DumpStopChannels[USED_SEQ]  ={-1};
   int detectorCh[MAXDET];
   TString detectorName[MAXDET];
   
   bool have_svd_events = false;
   
   DumpList<TA2Spill,TSVD_QOD,TSISEvent,NUM_SIS_MODULES> dumplist[USED_SEQ];
   std::mutex SequencerLock[USED_SEQ];
   
   DumpMakerModule(TARunInfo* runinfo, DumpMakerModuleFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="Handle Dumps";
#endif
      if (fTrace)
         printf("DumpMakerModule::ctor!\n");
      TSISChannels* SISChannels=new TSISChannels( runinfo->fRunNo );
      for (int j=0; j<USED_SEQ; j++) 
      {
         dumplist[j].SequencerID=j;
         DumpStartChannels[j] =SISChannels->GetChannel(StartNames[j],runinfo->fRunNo);
         DumpStopChannels[j]  =SISChannels->GetChannel(StopNames[j], runinfo->fRunNo);
      }
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
      for (int j=0; j<USED_SEQ; j++) 
         dumplist[j].fRunNo=runinfo->fRunNo;
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("DumpMakerModule::EndRun, run %d\n", runinfo->fRunNo);
      for (int a=0; a<USED_SEQ; a++)
      {
         dumplist[a].finish();
         while (dumplist[a].error_queue.size())
         {
            IncompleteDumps.push_back(dumplist[a].error_queue.front());
            dumplist[a].error_queue.pop_front();
         }
         dumplist[a].fRunNo=-2;
      }
         
      if (IncompleteDumps.size())
         printf("Error: Incomplete dumps!!!");
      for ( auto &spill :  IncompleteDumps )
      {
         std::cout<<"Deleting spill:"<<std::endl;
         spill->Print();
         delete spill;
      }
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

   //Catch sequencer flow in the main thread, so that we have expected dumps ASAP
   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      if( me->event_id != 8 ) // sequencer event id
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      DumpFlow* DumpsFlow=flow->Find<DumpFlow>();
      if (!DumpsFlow)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      uint ndumps=DumpsFlow->DumpMarkers.size();
      if (!ndumps)
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
         return flow;
      }
      int iSeq=DumpsFlow->SequencerNum;
      {
      //Lock scope
      std::lock_guard<std::mutex> lock(SequencerLock[iSeq]);
      
      dumplist[iSeq].setup(me->time_stamp);
      
      for(auto dump: DumpsFlow->DumpMarkers)
      {
         dumplist[iSeq].AddDump( &dump);
      }
      //Copy states into dumps
      dumplist[iSeq].AddStates(&DumpsFlow->states);
      //Inspect dumps and make sure the SIS will get triggered when expected... (study digital out)
      dumplist[iSeq].check(DumpsFlow->driver);
      
      }
      //dumplist[iSeq].Print();
      return flow;
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SISEventFlow* SISFlow = flow->Find<SISEventFlow>();
      if (SISFlow)
      {
         //Add timestamps to dumps
         for (int j=0; j<NUM_SIS_MODULES; j++)
         {
            std::vector<TSISEvent*>* ce=&SISFlow->sis_events[j];
            for (uint i=0; i<ce->size(); i++)
            {
              TSISEvent* e=ce->at(i);
              for (int a=0; a<USED_SEQ; a++)
              {
                 std::lock_guard<std::mutex> lock(SequencerLock[a]);
                 if (DumpStartChannels[a]>0)
                    //if (e->GetCountsInChannel(DumpStartChannels[a]))
                    for (int nstarts=0; nstarts<e->GetCountsInChannel(DumpStartChannels[a]); nstarts++)
                    {
                       dumplist[a].AddStartTime(e->GetMidasUnixTime(), e->GetRunTime());
                    }
                 if (DumpStopChannels[a]>0)
                    for (int nstops=0; nstops<e->GetCountsInChannel(DumpStopChannels[a]); nstops++)
                    {
                       dumplist[a].AddStopTime(e->GetMidasUnixTime(),e->GetRunTime());
                    }
               }
            }
         }
         //Add SIS counts to dumps
         for (int a=0; a<USED_SEQ; a++)
         {
            std::lock_guard<std::mutex> lock(SequencerLock[a]);
            for (int j=0; j<NUM_SIS_MODULES; j++)
            {
               //if (SISFlow->sis_events[j].size())
                  dumplist[a].AddScalerEvents(&SISFlow->sis_events[j]);
            }
         }
      }
      SVDQODFlow* SVDFlow = flow->Find<SVDQODFlow>();
      if (SVDFlow)
      {
         for (int a=0; a<USED_SEQ; a++)
         {
            std::lock_guard<std::mutex> lock(SequencerLock[a]);
            dumplist[a].AddSVDEvents(&SVDFlow->SVDQODEvents);
         }
      }

      A2SpillFlow* f=new A2SpillFlow(flow);

      GEMBANK_Flow* GEMFlow = flow->Find<GEMBANK_Flow>();
      double LNE0 = -1.;
      double LNE5 = -1.;
      uint32_t unixtime = -1;
      if (GEMFlow)
      {
         GEMBANK<float> *bank = (GEMBANK<float>*) GEMFlow->data;
         unixtime = GEMFlow->MIDAS_TIME;
         if (bank->GetCategoryName() == "ADandELENA" && bank->GetSizeOfDataArray())
         {
            //LNE0
            if (bank->GetVariableName() == "LNEAPULB0030")
               LNE0 = *bank->GetFirstDataEntry()->DATA(0);
            //LNE5
            if (bank->GetVariableName() == "LNEAPULB5030")
               LNE5 = *bank->GetFirstDataEntry()->DATA(0);
         }
      }
      GEMBANKARRAY_Flow* GEMArrayFlow = flow->Find<GEMBANKARRAY_Flow>();
      if (GEMArrayFlow)
      {
         GEMBANKARRAY *array = GEMArrayFlow->data;
         unixtime = GEMArrayFlow->MIDAS_TIME;
         int bank_no = 0;
         GEMBANK<float>* bank = (GEMBANK<float>*) array->GetGEMBANK(bank_no);
         while(bank)
         {

            if (bank->GetCategoryName() == "ADandELENA")// && bank->GetSizeOfDataArray())
            {
               //LNE0
               if (bank->GetVariableName() == "LNEAPULB0030")
               {
                  LNE0 = *(bank->GetFirstDataEntry()->DATA(0));
               }
               //LNE5
               if (bank->GetVariableName() == "LNEAPULB5030")
               {
                  LNE5 = *(bank->GetFirstDataEntry()->DATA(0));
               }
            }
            bank = (GEMBANK<float>*) array->GetGEMBANK(++bank_no);
         }
      }
      if (LNE0 > 0 || LNE5 > 0)
      {
         
         std::string ELENA_STRING = std::string("LNE0: ") + std::to_string(LNE0) + std::string("\tLNE5: ") + std::to_string(LNE5);
         TA2Spill* elena = NULL;
         if (LNE0 > 0 && LNE5 > 0)
            elena = new TA2Spill(runinfo->fRunNo,unixtime,"---------------------> LNE0: %.2E \t LNE5: %.2E	",LNE0 * 1E6,LNE5 * 1E6);
         else if (LNE0 > 0)
            elena = new TA2Spill(runinfo->fRunNo,unixtime,"---------------------> LNE0: %.2E ",LNE0 * 1E6);
         else if (LNE5 > 0)
            elena = new TA2Spill(runinfo->fRunNo,unixtime,"---------------------> \t\t\tLNE5: %.2E ",LNE5 * 1E6);
         if (elena)
            f->spill_events.push_back(elena);
         //std::cout << "DATA" << LNE0 << "\t" << LNE5 << std::endl;
      }


      //Flush errors
      for (int a=0; a<USED_SEQ; a++)
      {
         std::lock_guard<std::mutex> lock(SequencerLock[a]);
         while (dumplist[a].error_queue.size())
         {
            IncompleteDumps.push_back(dumplist[a].error_queue.front());
            dumplist[a].error_queue.pop_front();
         }
      }

      for (size_t i=0; i<IncompleteDumps.size(); i++)
      {
         //if IncompleteDumps.front()INFO TYPE...
          f->spill_events.push_back(IncompleteDumps.front());
          IncompleteDumps.pop_front();
      }

      //Flush completed dumps as TA2Spill objects and put into flow
      for (int a=0; a<USED_SEQ; a++)
      {
         std::lock_guard<std::mutex> lock(SequencerLock[a]);
         std::vector<TA2Spill*> finished=dumplist[a].flushComplete();
         for (TA2Spill* spill: finished)
         {
            //spill->Print();
            f->spill_events.push_back(spill);
         }
      }

      flow=f;
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
