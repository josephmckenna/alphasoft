//
// spill_log_module
//
// JTK McKenna
//


#include <list>
#include <stdio.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include "manalyzer.h"
#include "midasio.h"
#include "TSystem.h"
#include <TEnv.h>

#include "A2Flow.h"
#include "TTree.h"

#include <vector>


#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

#define HOT_DUMP_LOW_THR 500


#include "AnalysisTimer.h"

#ifdef HAVE_XMLSERVER
#include "xmlServer.h"
#include "TROOT.h"
#endif

#ifdef XHAVE_LIBNETDIRECTORY
#include "netDirectoryServer.h"
#endif

#include "TSISChannels.h"

class MatchSeqToDumpFlags
{
public:
   bool fPrint = false;
   bool fWriteElog = false;
   bool fWriteSpillDB = false;
   bool fWriteSpillTxt = false;
   bool fNoSpillSummary = false;
};


class MatchSeqToDump: public TARunObject
{
public: 
   MatchSeqToDumpFlags* fFlags;
   bool fTrace = false;
   int gIsOnline = 0;
   Int_t RunState =-1;
   Int_t gRunNumber =0;
   time_t run_start_time=0;
   time_t run_stop_time=0;
   int seqcount[NUMSEQ]; //Count sequences run

   std::vector<DumpMarker> DumpMarkers[NUMSEQ][2];
   int DumpPosition[NUMSEQ]={0};  
  
   
   std::vector<int> sis_channels;
   int n_sis_channels;

public:
   
   
   MatchSeqToDump(TARunInfo* runinfo, MatchSeqToDumpFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("MatchSeqToDump::ctor!\n");
    
   }

   ~MatchSeqToDump()
   {
      if (fTrace)
         printf("MatchSeqToDump::dtor!\n");
   }


   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("MatchSeqToDump::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
     
     
      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      //Is running, RunState==3
      //Is idle, RunState==1
      RunState=runinfo->fOdb->odbReadInt("/runinfo/State"); //The odb isn't in its 'final' state before run, so this is useless
      gRunNumber=runinfo->fRunNo;
      std::cout <<"RUN STATE!:"<<RunState<<std::endl;
      run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
      std::cout<<"START:"<< run_start_time<<std::endl;
      std::cout<<"STOP: "<< run_stop_time<<std::endl;

      if (run_start_time>0 && run_stop_time==0) //Start run
      {
         for (int i=0; i<USED_SEQ; i++)
         {
            gIsOnline=1;
         }
      }
      else
      {
         gIsOnline=0;
      }

      if (gIsOnline)
      {
      }

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

   }

   void PreEndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("MatchSeqToDump::EndRun, run %d\n", runinfo->fRunNo);
      //runinfo->State

      if (!fFlags->fNoSpillSummary)
      {
         for (int i = 0; i < USED_SEQ; i++)
         {
            int iSeq=USED_SEQ_NUM[i];
            //Check if there are unfinished dumps... throw warning!
            int incomplete_starts=DumpMarkers[iSeq][0].size() - DumpPosition[iSeq];
            int incomplete_stops=DumpMarkers[iSeq][1].size() - DumpPosition[iSeq];

            TString new_seq_msg;
            if (incomplete_starts)
            {
               std::cerr<<"End of run... Seqencer "<<iSeq<<" has "<<incomplete_starts<<" dump starts haven't happened!"<<std::endl;
               new_seq_msg+="\nEnd of run... Seqencer ";
               new_seq_msg+=incomplete_starts;
               new_seq_msg+=" start markers haven't happened:\t";
               for (int j=DumpPosition[iSeq]; j<(int)DumpMarkers[iSeq][0].size(); j++)
               {
                  new_seq_msg+=DumpMarkers[iSeq][0].at(j).Description.Data();
                  if (j!=incomplete_starts-1)
                     new_seq_msg+=", ";
               }
               //DumpMarkers[iSeq][0].clear();
            }
            if (incomplete_stops)
            {
               std::cerr<<"End of run... Seqencer "<<iSeq<<" has "<<incomplete_stops<<" dump starts haven't happened!"<<std::endl;
               new_seq_msg+="\nEnd of run... Seqencer ";
               new_seq_msg+=incomplete_stops;
               new_seq_msg+=" stop markers haven't happened:\t";
               for (int j=DumpPosition[iSeq]; j<(int)DumpMarkers[iSeq][1].size(); j++)
               {
                  new_seq_msg+=DumpMarkers[iSeq][1].at(j).Description.Data();
                  if (j!=incomplete_stops-1)
                     new_seq_msg+=", ";
               }
               //DumpMarkers[iSeq][1].clear();
            }
            if (incomplete_starts || incomplete_stops)
            {
               TA2Spill* err=new TA2Spill(new_seq_msg.Data());
               A2SpillFlow* f=new A2SpillFlow(NULL);
               f->spill_events.push_back(err);
               runinfo->AddToFlowQueue(f);
            }
            if (incomplete_starts && incomplete_stops)
            {
               int j=DumpPosition[iSeq];
               A2SpillFlow* f=NULL;
               while(1)
               {
                  if (j>=(int)DumpMarkers[iSeq][0].size()) break;
                  if (j>=(int)DumpMarkers[iSeq][1].size()) break;
                  if (strcmp(DumpMarkers[iSeq][0].at(j).Description.Data(),DumpMarkers[iSeq][1].at(j).Description.Data())!=0)
                  {
                     TString miss_match="MISS MATCHING (UNUSED) START AND STOP DUMP NAMES:";
                     miss_match+=DumpMarkers[iSeq][0].at(j).Description.Data();
                     miss_match+=" and ";
                     miss_match+=DumpMarkers[iSeq][1].at(j).Description.Data();
                     if (!f)
                        f=new A2SpillFlow(NULL);
                     TA2Spill* err=new TA2Spill(miss_match.Data());
                     f->spill_events.push_back(err);
                  }
                  j++;
               }
               runinfo->AddToFlowQueue(f);
            }
         }

      }

   }
   

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("MatchSeqToDump::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }
   
   TAFlowEvent* AnalyzeFlowEvent(TARunInfo* runinfo, TAFlags* flags, TAFlowEvent* flow)
   {
   //   if (!gIsOnline) return flow;
      #ifdef _TIME_ANALYSIS_
      START_TIMER
      #endif 
      std::vector<TA2Spill*> messages;
      const AgDumpFlow* DumpFlow = flow->Find<AgDumpFlow>();
      if (DumpFlow)
      { // I am a Dump Flow
         for (int i = 0; i < USED_SEQ; i++)
         {
            int iSeq=USED_SEQ_NUM[i];
            //Fix this to insert new vector at back (not this dumb loop)
            uint ndumps=DumpFlow->DumpMarkers[iSeq].size();
            
            if (!ndumps) continue;
            TString new_seq_msg="---- Sequence start (";
            new_seq_msg+=iSeq;
            new_seq_msg+=")---- ";
            //Check if there are unfinished dumps... throw warning!
            std::cout<<"SEQ:"<<iSeq<<"("<<i<<")"<<"starts: "<<DumpMarkers[iSeq][0].size()<<" current position:"<<DumpPosition[iSeq]<<std::endl;
            int incomplete_starts=DumpPosition[iSeq] - DumpMarkers[iSeq][0].size();
            int incomplete_stops=DumpPosition[iSeq] - DumpMarkers[iSeq][1].size();
            if (ndumps && incomplete_starts>0)
            {
               std::cerr<<"New sequence for Seqencer:"<<iSeq<<"\t"<<incomplete_starts<<" starts haven't happened!"<<std::endl;
               new_seq_msg+="\n";
               new_seq_msg+=incomplete_starts;
               new_seq_msg+=" start markers haven't happened:\t";
               for (int j=DumpPosition[iSeq]; j<DumpPosition[iSeq]+incomplete_starts; j++)
               {
                  if (j<(int)DumpMarkers[iSeq][0].size())
                     new_seq_msg+=DumpMarkers[iSeq][0].at(j).Description.Data();
                  else
                     new_seq_msg+="UNKNOWN";
                  if (j!=DumpPosition[iSeq]+incomplete_starts-1)
                     new_seq_msg+=", ";
               }
               //DumpPosition[iSeq]=DumpMarkers[iSeq][0].size()-1;
            }
            if (ndumps && incomplete_stops>0)
            {
               std::cerr<<"New sequence for Seqencer:"<<iSeq<<"\t"<<incomplete_stops<<" stops haven't happened!"<<std::endl;
               new_seq_msg+="\n";
               new_seq_msg+=incomplete_stops;
               new_seq_msg+=" stop markers haven't happened:\t";
               for (int j=DumpPosition[iSeq]; j<DumpPosition[iSeq]+incomplete_stops; j++)
               {
                  if (j<(int)DumpMarkers[iSeq][1].size())
                     new_seq_msg+=DumpMarkers[iSeq][1].at(j).Description.Data();
                  else
                     new_seq_msg+="UNKNOWN";
                  if (j!=DumpPosition[iSeq]+incomplete_stops-1)
                     new_seq_msg+=", ";
               }
               //DumpPosition[iSeq]=DumpMarkers[iSeq][1].size()-1;
            }
            messages.push_back(new TA2Spill(new_seq_msg.Data()));
            
            for (uint j=0; j<ndumps; j++)
            {
               //Show list of up-comming start dumps
               char StartStop='#';
               int type_pos=-1;
               if (DumpFlow->DumpMarkers[iSeq].at(j).DumpType==1)
               {
                  StartStop='(';
                  //DumpStarts++;
                  type_pos=0;
               }
               if (DumpFlow->DumpMarkers[iSeq].at(j).DumpType==2)
               {
                  StartStop=')';
                  //DumpStops++;
                  type_pos=1;
               }
               TString msg = TString::Format("%c  %s", StartStop, DumpFlow->DumpMarkers[iSeq].at(j).Description.Data());
               //std::cout<<msg<<std::endl;
               //Add the markers to a queue for timestamps later
               if (type_pos==0 || type_pos==1)
                  DumpMarkers[iSeq][type_pos].push_back(DumpFlow->DumpMarkers[iSeq].at(j));
            }
         }
      }

      A2SpillFlow* SpillFlow= flow->Find<A2SpillFlow>();
      if (SpillFlow)
      {
         for (size_t i=0; i<SpillFlow->spill_events.size(); i++)
         {
            TA2Spill* s=SpillFlow->spill_events.at(i);
            //s->Print();
            if (!s->SeqData) continue;
            int thisSeq=s->SeqData->fSequenceNum;
            s->SeqData->fDumpID=DumpPosition[thisSeq];

            const char* DumpStartName;
            int start_dump_state=-1;
            if (DumpPosition[thisSeq]>=(int)DumpMarkers[thisSeq][0].size())
            {
               DumpStartName=NULL;
            }
            else
            {
               DumpStartName=DumpMarkers[thisSeq][0].at(DumpPosition[thisSeq]).Description.Data();
               start_dump_state=DumpMarkers[thisSeq][0].at(DumpPosition[thisSeq]).fonState;
            }
            const char* DumpStopName;
            int stop_dump_state=-1;
            if (DumpPosition[thisSeq]>=(int)DumpMarkers[thisSeq][1].size())
            {
               DumpStopName=NULL;
            }
            else
            {
               DumpStopName=DumpMarkers[thisSeq][1].at(DumpPosition[thisSeq]).Description.Data();
               stop_dump_state=DumpMarkers[thisSeq][1].at(DumpPosition[thisSeq]).fonState;
            }
            if (DumpStartName)
            {
               s->Name=DumpStartName;
               s->SeqData->fStartState=start_dump_state;
               DumpPosition[thisSeq]++;
            }
            if (!DumpStartName)
            {
               //If a name hasn't been already assigned to the dump (maybe from catch_efficiency_module)
               if (strcmp(s->Name.c_str(),"")==0)
                  s->Name="MISSING_DUMP_NAME";
               DumpStartName="MISSING_DUMP_NAME";
            }
            if (DumpStopName)
            {
               s->SeqData->fStopState=stop_dump_state;
            }
            if (!DumpStopName)
               DumpStopName="MISSING_DUMP_NAME";

            if (strcmp(DumpStartName,DumpStopName)!=0)
            {
               char error[100];
               sprintf(error,"Miss matching dump names! %s AND %s",DumpStartName,DumpStopName);
               std::cerr<<error<<std::endl;
               messages.push_back(new TA2Spill(error));
            }
         }
      }
      if (messages.size())
      {
         A2SpillFlow* sf;
         if (!SpillFlow)
         {
            sf=new A2SpillFlow(flow);
            flow=sf;
         }
         else
         {
            sf=SpillFlow;
         }
         for (size_t i=0; i<messages.size(); i++)
         {
             sf->spill_events.push_back(messages.at(i));
         }
         messages.clear();
      }
      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"spill_log_module",timer_start);
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("MatchSeqToDump::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class MatchSeqToDumpFactory: public TAFactory
{
public:
   MatchSeqToDumpFactory(): TAFactory()
   {
      std::cout<<"Please run me as: ./ag_events.exe -g -Halphagdaq.cern.ch "<<std::endl;
      gEnv->SetValue("Gui.DefaultFont","-*-courier-medium-r-*-*-12-*-*-*-*-*-iso8859-1");  
   }
   MatchSeqToDumpFlags fFlags;

public:
   void Usage()
   {
      std::cout<<"\t--elog\t\tWrite elog (not implemented)"<<std::endl;
      std::cout<<"\t--spilldb\t\tSwrite to Spill log sqlite database (local)"<<std::endl;
      std::cout<<"\t--spilltxt\t\tWrite Spill log to SpillLog/reload.txt"<<std::endl;
      std::cout<<"\t--nospillsummary\t\tTurn off spill log table printed at end of run"<<std::endl;
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("SpillLogFactory::Init!\n");
      

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--nospillsummary")
            fFlags.fNoSpillSummary = true;
      }
 
   }

   void Finish()
   {
      if (fFlags.fPrint)
         printf("MatchSeqToDumpFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("MatchSeqToDumpFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new MatchSeqToDump(runinfo, &fFlags);
   }
};

static TARegister tar(new MatchSeqToDumpFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
