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
#include <iomanip> //set precision
#include <sstream>

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

   std::array<TSISChannel,USED_SEQ> DumpStartChannels;
   std::array<TSISChannel,USED_SEQ> DumpStopChannels;
   
   TSISChannel fADChannel;
   TSISChannel fPreTriggerChannel;
   int fADCounter;
   int fPreTriggerCounter;
   
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
      TSISChannels* SISChannels=new TSISChannels( runinfo->fRunNo );
      for (int j=0; j<USED_SEQ; j++) 
      {
         dumplist[j].SequencerID=j;
         dumplist[j].fRunNo=runinfo->fRunNo;
         DumpStartChannels[j] =SISChannels->GetChannel(StartNames[j],runinfo->fRunNo);
         DumpStopChannels[j]  =SISChannels->GetChannel(StopNames[j], runinfo->fRunNo);
      }
      fADChannel = SISChannels->GetChannel("SIS_AD", runinfo->fRunNo);
      fPreTriggerChannel = SISChannels->GetChannel("SIS_DIX_ALPHA", runinfo->fRunNo);
      delete SISChannels;

      fADCounter = 0;
      fPreTriggerCounter = 0;

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
      const uint ndumps=DumpsFlow->DumpMarkers.size();
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
      dumplist[iSeq].AddStates(DumpsFlow->states);
      //Inspect dumps and make sure the SIS will get triggered when expected... (study digital out)
      dumplist[iSeq].check(DumpsFlow->driver);
      
      }
      //dumplist[iSeq].Print();
      return flow;
   }

   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
      SISEventFlow* SISFlow = flow->Find<SISEventFlow>();
      SVDQODFlow* SVDFlow = flow->Find<SVDQODFlow>();
      GEMBANK_Flow* GEMFlow = flow->Find<GEMBANK_Flow>();
      GEMBANKARRAY_Flow* GEMArrayFlow = flow->Find<GEMBANKARRAY_Flow>();
      felabviewFlowEvent* LabVIEWFlow = flow->Find<felabviewFlowEvent>();

      if (SISFlow || SVDFlow || GEMFlow || GEMArrayFlow || LabVIEWFlow)
      {
        //We have some work to do on the flow
      }
      else
      {
         //Nothing to do here
         return flow;   
      }
      A2SpillFlow* f=new A2SpillFlow(flow);
      
      if (SISFlow)
      {
         //Add timestamps to dumps
         for (int j=0; j<NUM_SIS_MODULES; j++)
         {
            const std::vector<TSISEvent>& ce = SISFlow->sis_events[j];
            for (size_t i = 0; i < ce.size(); i++)
            {
               const TSISEvent& e = ce.at(i);
               for (int a = 0; a < USED_SEQ; a++)
               {
                  std::lock_guard<std::mutex> lock(SequencerLock[a]);
                  if (DumpStartChannels.at(a).IsValid())
                  {
                     const int counts = e.GetCountsInChannel(DumpStartChannels[a]);
                     //if (e->GetCountsInChannel(DumpStartChannels[a]))
                     for (int nstarts = 0; nstarts < counts; nstarts++)
                     {
                        dumplist[a].AddStartTime(e.GetMidasUnixTime(), e.GetRunTime());
                     }
                  }
                  if (DumpStopChannels.at(a).IsValid())
                  {
                     const int counts = e.GetCountsInChannel(DumpStopChannels[a]);
                     for (int nstops = 0; nstops < counts; nstops++)
                     {
                        dumplist[a].AddStopTime(e.GetMidasUnixTime(),e.GetRunTime());
                     }
                  }
               }
               if (e.GetCountsInChannel(fADChannel))
               {
                 TA2Spill* beam = new TA2Spill(runinfo->fRunNo,e.GetMidasUnixTime(),"Beam %d ------------------------------------------------------->	",fADCounter++);
                 f->spill_events.push_back(beam);
               }
               if (e.GetCountsInChannel(fPreTriggerChannel))
               {
                 TA2Spill* pretrigger = new TA2Spill(runinfo->fRunNo,e.GetMidasUnixTime(),"----- Pre Trigger %d------->",fPreTriggerCounter++);
                 f->spill_events.push_back(pretrigger);
               }
            }

         }
         //Add SIS counts to dumps
         for (int a=0; a<USED_SEQ; a++)
         {
            //if (SISFlow->sis_events[j].size())
            dumplist[a].AddAndSortScalerEvents(SISFlow->sis_events);
         }
         /*
         for (int a=0; a<USED_SEQ; a++)
         {
            std::lock_guard<std::mutex> lock(SequencerLock[a]);
            for (int j=0; j<NUM_SIS_MODULES; j++)
            {
               //if (SISFlow->sis_events[j].size())
                  dumplist[a].AddScalerEvents(SISFlow->sis_events[j]);
            }
         }*/
      }

      if (SVDFlow)
      {
         for (int a=0; a<USED_SEQ; a++)
         {
            std::lock_guard<std::mutex> lock(SequencerLock[a]);
            dumplist[a].AddSVDEvents(&SVDFlow->SVDQODEvents);
         }
      }




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
               LNE0 = *(bank->GetFirstDataEntry()->DATA(0));
            //LNE5
            if (bank->GetVariableName() == "LNEAPULB5030")
               LNE5 = *(bank->GetFirstDataEntry()->DATA(0));
         }
      }

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
  
  
      if (LabVIEWFlow)
      {
         if (LabVIEWFlow->GetBankName() == "PADT" )
         {
            std::ostringstream CsI;
            CsI << std::setprecision(2) << "\t" << LabVIEWFlow->GetBankName() << ": ";
            std::vector<std::string> names = { "CsI1*", "CsI1", "CsI2", "CsIA", "CsIB", "CsIC", "CsID", "CsE", "CsIF" };
            for (int i = 1; i < LabVIEWFlow->GetData()->size(); i++)
            {
               if ( i - 1 < names.size() )
                  CsI << names.at(i-1) << std::string(": ");
               CsI << LabVIEWFlow->GetData()->at(i) << "\t";
            }
            //TInfoSpill = new TInfoSpill(padt, 0 ,unixtime, padt.c_str());
            TA2Spill* lv = new TA2Spill(runinfo->fRunNo,unixtime,CsI.str().c_str());
            f->spill_events.push_back(lv);
         }
         else if( LabVIEWFlow->GetBankName() == "PADG")
         {
            std::ostringstream CsI;
            CsI << std::setprecision(2) << "\t" << LabVIEWFlow->GetBankName() << ": ";
            std::vector<std::string> names = { "PScreen", "AG1", "AG2", "BL A", "BL B", "BL C", "BL D", "BL E", "BL F" };
            for (int i = 1; i < LabVIEWFlow->GetData()->size(); i++)
            {
               if ( i - 1 < names.size() )
                  CsI  << names.at(i-1)  << std::string(": ");

               CsI << LabVIEWFlow->GetData()->at(i) << "\t";
            }
            //TInfoSpill = new TInfoSpill(padt, 0 ,unixtime, padt.c_str());
            TA2Spill* lv = new TA2Spill(runinfo->fRunNo,unixtime,CsI.str().c_str());
            f->spill_events.push_back(lv);
         }
         // Check pattern XRWX
         else if ( strncmp(LabVIEWFlow->GetBankName().c_str() + 1, "RW", 2) == 0)
         {
            //Catching trap
            if (LabVIEWFlow->GetBankName().at(0) == 'C')
            {
               std::ostringstream CTRW;
               CTRW << "[" << LabVIEWFlow->GetBankName() << "]: ";
               
               // This is a clunky handling of the frequency units
               double StartSweep = LabVIEWFlow->GetData()->at(2);
               double StopSweep = LabVIEWFlow->GetData()->at(3);
               //If units should be kHz
               if ( StartSweep > 1000 && StopSweep > 1000)
               {
                   StartSweep /= 1000;
                   StopSweep /= 1000;
                   //If units should be MHz
                   if ( StartSweep > 1000 && StopSweep > 1000)
                   {
                      StartSweep /= 1000;
                      StopSweep /= 1000;
                      CTRW << StartSweep << "-" << StopSweep <<"MHz\t";
                   }
                   else // Am kHz
                   {
                      CTRW << StartSweep << "-" << StopSweep <<"kHz\t";
                   }
               }
               else // Am Hz
               {
                  CTRW << StartSweep << "-" << StopSweep << "Hz\t";
               }
               CTRW << "Total "<< LabVIEWFlow->GetData()->at(4) <<"s (" << LabVIEWFlow->GetData()->at(5) <<" fade ms)\t";
               CTRW << LabVIEWFlow->GetData()->at(6) <<"-"<< LabVIEWFlow->GetData()->at(7) << "V";
               TA2Spill* lv = new TA2Spill(runinfo->fRunNo,unixtime,CTRW.str().c_str());
               f->spill_events.push_back(lv);
            }
            //Atom trap
            else if (LabVIEWFlow->GetBankName().at(0) == 'A')
            {
               
            }
         }
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
