//
// ALPHAg (Chronobox) spill_log_module
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

#include "AgFlow.h"

#include "chrono_module.h"
#include "TChrono_Event.h"
#include "TChronoChannelName.h"
#include "TTree.h"

#include <vector>
//MAX DET defined here:
#include "TSpill.h"

#define MAXDET 10

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

#define HOT_DUMP_LOW_THR 500


#include "AnalysisTimer.h"

time_t gTime; // system timestamp of the midasevent
time_t LastUpdate;
//struct tm LastUpdate = {0};


#ifdef HAVE_XMLSERVER
#include "xmlServer.h"
#include "TROOT.h"
#endif

#ifdef XHAVE_LIBNETDIRECTORY
#include "netDirectoryServer.h"
#endif


class SpillLogFlags
{
public:
   bool fPrint = false;
   bool fWriteElog = false;
   bool fWriteSpillDB = false;
   bool fWriteSpillTxt = false;
   bool fNoSpillSummary = false;
};


class SpillLog: public TARunObject
{
public: 
   SpillLogFlags* fFlags;
   bool fTrace = false;
   int gIsOnline = 0;
   Int_t RunState =-1;
   Int_t gRunNumber =0;
   time_t run_start_time=0;
   time_t run_stop_time=0;

   std::vector<std::string> InMemorySpillTable;
   TTree* SpillTree = NULL;

   //Live spill log body:
   std::ofstream LiveSpillLog;
   //Column headers
   std::ofstream SpillLogHeader;
   //List of active dumps
   std::ofstream LiveSequenceLog[NUMSEQ];

   std::vector<std::pair<int,int>> chrono_channels;
   int n_chrono_channels;

   //Chronobox channels
   Int_t ChronoClock[CHRONO_N_BOARDS];

   //Detector data to integrate (From ChronoFlow)
   Int_t DetectorChans[CHRONO_N_BOARDS][MAXDET];
   std::vector<Double_t> DetectorTS[MAXDET];
   std::vector<Int_t> DetectorCounts[MAXDET];
   TString detectorName[MAXDET];

   //Dump Marker counter (From ChronoFlow)
   std::vector<Double_t> StartTime[NUMSEQ];
   std::vector<Double_t> StopTime[NUMSEQ];


   //Channels for Dump markers
   ChronoChannel StartChannel[NUMSEQ];
   ChronoChannel StopChannel[NUMSEQ];
   ChronoChannel StartSeqChannel[NUMSEQ];

   std::vector<DumpMarker> DumpMarkers[NUMSEQ][2];
   uint DumpStarts;
   uint DumpStops;
   //Dump Markers to give timestamps (From DumpFlow)
   //std::vector<TString> Description[4];
   //std::vector<Int_t> DumpType[4]; //1=Start, 2=Stop
   //std::vector<Int_t> fonCount[4];
   // Int_t SequencerNum[4];
   Int_t gADSpillNumber;
   TSpill* gADSpill;
   ChronoChannel gADSpillChannel={-1,-1};
   
   Int_t gPOSSpillNumber;
   ChronoChannel gPOSSpillChannel={-1,-1};
private:
   sqlite3 *ppDb; //SpillLogDatabase handle
   sqlite3_stmt * stmt;
public:
   TString SpillLogTitle;

   SpillLog(TARunInfo* runinfo, SpillLogFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("SpillLog::ctor!\n");
   }

   ~SpillLog()
   {
      if (fTrace)
         printf("SpillLog::dtor!\n");
   }
   void SaveToTree(TARunInfo* runinfo,TAGSpill* s)
   {
         if (!s) return;
         #ifdef HAVE_CXX11_THREADS
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         #endif
         runinfo->fRoot->fOutputFile->cd();
         if (!SpillTree)
            SpillTree = new TTree("AGSpillTree","AGSpillTree");
         TBranch* b_variable = SpillTree->GetBranch("TAGSpill");
         if (!b_variable)
            SpillTree->Branch("TAGSpill","TAGSpill",&s,16000,1);
         else
            SpillTree->SetBranchAddress("TAGSpill",&s);
         SpillTree->Fill();
   }


   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
              
      if (fFlags->fWriteSpillDB)
      {
         if (sqlite3_open("SpillLog/AGSpillLog.db",&ppDb) == SQLITE_OK)
         {
            std::cout<<"Database opened ok"<<std::endl;
         }
         else
         {
            exit(555);
         }
      }
            if (!fFlags->fNoSpillSummary)
      {
         InMemorySpillTable.push_back("Begin run "+std::to_string(runinfo->fRunNo) );
         InMemorySpillTable.push_back(SpillLogTitle.Data());
         //InMemorySpillTable.push_back("             Dump Time            | CAT Event       WHAT DO WE WANT TO DO HERE!?! ");
         InMemorySpillTable.push_back("---------------------------------------------------------------------------------------------------------------------------------------------------------------------");
      }
      if (fFlags->fWriteSpillTxt)
      {
         std::cout<<"Writing to text file not support in ALPHAg.... go copy it from ALPHA 2 code in $AGRELEASE/alpha2/src/"<<std::endl;
      }

      //time_t run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      //printf("ODB Run start time: %d: %s", (int)run_start_time, ctime(&run_start_time));
      //Is running, RunState==3
      //Is idle, RunState==1
      
      gRunNumber=runinfo->fRunNo;
      //      std::cout <<"RUN STATE!:"<<RunState<<std::endl;

#ifdef INCLUDE_VirtualOdb_H
      RunState=runinfo->fOdb->odbReadInt("/runinfo/State"); //The odb isn't in its 'final' state before run, so this isFile Edit Options Buffers Tools C++ Help   
      run_start_time = runinfo->fOdb->odbReadUint32("/Runinfo/Start time binary", 0, 0);
      run_stop_time = runinfo->fOdb->odbReadUint32("/Runinfo/Stop time binary", 0, 0);
#endif
#ifdef INCLUDE_MVODB_H
      runinfo->fOdb->RU32("/Runinfo/Start time binary",(uint32_t*) &run_start_time);
      runinfo->fOdb->RU32("/Runinfo/Stop time binary",(uint32_t*) &run_stop_time);
      runinfo->fOdb->RI("/runinfo/State",&RunState);
#endif

      std::cout<<"START:"<< run_start_time<<std::endl;
      std::cout<<"STOP: "<< run_stop_time<<std::endl;
      
      for (int i=0; i<CHRONO_N_BOARDS; i++)
         ChronoClock[i]=CHRONO_CLOCK_CHANNEL;

      //Save chronobox channel names
      TChronoChannelName* name[CHRONO_N_BOARDS];
      TString ChannelName;

      for (int i=0; i<USED_SEQ; i++)
      {
         int iSeq=USED_SEQ_NUM[i];
         std::cout<<i<<" is " << iSeq <<std::endl;
         StartChannel[iSeq].Channel=-1;
         StartChannel[iSeq].Board=-1;
         StopChannel[iSeq].Channel=-1;
         StopChannel[iSeq].Board=-1;
         StartSeqChannel[iSeq].Channel=-1;
         StartSeqChannel[iSeq].Board=-1;
         //fListBoxSeq[i]->RemoveAll();
         //LayoutListBox(fListBoxSeq[i]);
      }
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
      for (int board=0; board<CHRONO_N_BOARDS; board++)
      {
         name[board]=new TChronoChannelName();
         name[board]->SetBoardIndex(board+1);
         for (int det=0; det<MAXDET; det++)
            DetectorChans[board][det]=-1;
         for (int chan=0; chan<CHRONO_N_CHANNELS; chan++)
         {
            TString OdbPath="/Equipment/cbms0";
            OdbPath+=board+1;
            OdbPath+="/Settings/ChannelNames";
            //std::cout<<runinfo->fOdb->odbReadString(OdbPath.Data(),chan)<<std::endl;
            #ifdef INCLUDE_VirtualOdb_H
            if (runinfo->fOdb->odbReadString(OdbPath.Data(),chan))
               name->SetChannelName(runinfo->fOdb->odbReadString(OdbPath.Data(),chan),chan);
            #endif
            #ifdef INCLUDE_MVODB_H
            std::string tmp;
            runinfo->fOdb->RSAI(OdbPath.Data(),chan,&tmp);
            name[board]->SetChannelName(tmp.c_str(),chan);
            #endif
         }
     }

      std::vector<std::pair<std::string,std::string>> channels=
      {
         //channel name, descriptive name
         {"CATCH_OR","CATCH_OR"},
         {"SiPM_B","TOP PM"},
         {"SiPM_E","BOT PM"},
         {"TPC_TRIG","TPC"},
         {"SiPM_A_AND_D","SiPM_A_AND_D"},
         {"SiPM_C_AND_F","SiPM_C_AND_F"},
         {"SiPM A_OR_C-AND-D_OR_F","SiPM A_OR_C-AND-D_OR_F"},
         {"SiPM_C","SiPM_C"},
         {"SiPM_D","SiPM_D"},
         {"SiPM_F","SiPM_F"}
      };
     
      for (size_t i=0; i<channels.size(); i++)
      {
         for (int board=0; board<CHRONO_N_BOARDS; board++)
         {
            int channel=name[board]->GetChannel(channels.at(i).first);
            if (channel>0)
               chrono_channels.push_back({board,channel});
         }
      }
      n_chrono_channels=chrono_channels.size();

      for (int board=0; board<CHRONO_N_BOARDS; board++)
      {
         int channel=-1;
         for (int i=0; i<USED_SEQ; i++)
         {
            int iSeq=USED_SEQ_NUM[i];
            channel=name[board]->GetChannel(StartDumpName[i]);
            if (channel>0)
            {
               std::cout<<"Sequencer["<<iSeq<<"]:"<<StartDumpName[iSeq]<<" on channel:"<<channel<< " board:"<<board<<std::endl;
               StartChannel[iSeq].Channel=channel;
               StartChannel[iSeq].Board=board;
               std::cout<<"Start Channel:"<<channel<<std::endl;
            }
            
            channel=name[board]->GetChannel(StopDumpName[i]);
            if (channel>0)
            {
               std::cout<<"Sequencer["<<iSeq<<"]:"<<StopDumpName[iSeq]<<" on channel:"<<channel<< " board:"<<board<<std::endl;
               StopChannel[iSeq].Channel=channel;
               StopChannel[iSeq].Board=board;
               std::cout<<"Stop Channel:"<<channel<<std::endl;
            }

            channel=name[board]->GetChannel(StartSeqName[i]);
            if (channel>0)
            {
               std::cout<<"Sequencer["<<iSeq<<"]"<<StartSeqName[iSeq]<<" on channel:"<<channel<< " board:"<<board<<std::endl;
               StartSeqChannel[iSeq].Channel=channel;
               StartSeqChannel[iSeq].Board=board;
            }
         }

         channel=name[board]->GetChannel("AD_TRIG");
         if (channel>0)
         {
            gADSpillChannel.Channel=channel;
            gADSpillChannel.Board=board;
            std::cout<<"AD_TRIG:"<<channel<<" board:"<<board<<std::endl;
         }
         channel=name[board]->GetChannel("POS_TRIG");
         if (channel>0 )
         {
            gPOSSpillChannel.Channel=channel;
            gPOSSpillChannel.Board=board;
         }
      }

      for (int i=0; i<MAXDET; i++)
      {
         DetectorTS[i].clear();
         DetectorCounts[i].clear();
      }
      DumpStarts=0;
      DumpStops=0;
      for (int i = 0; i < USED_SEQ; i++)
      {
         int iSeqType=USED_SEQ_NUM[i];
         StartTime[iSeqType].clear();
         StopTime[iSeqType].clear();
         DumpMarkers[iSeqType][0].clear();
         DumpMarkers[iSeqType][1].clear();
      }
      if (gIsOnline)
      {

      }
      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }


   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::EndRun, run %d\n", runinfo->fRunNo);
      //runinfo->State

      if (!fFlags->fNoSpillSummary)
      {


         InMemorySpillTable.push_back("End run");
         InMemorySpillTable.push_back("---------------------------------------------------------------------------------------------------------------------------------------------------------------------");
         size_t lines=InMemorySpillTable.size();
         unsigned long byte_size=0;
         for (size_t i=0; i<lines; i++)
            byte_size+=InMemorySpillTable[i].size()*sizeof(char);
         std::string unit;
         double factor=1;
         if (byte_size>(unsigned long)factor*1024)
         {
            unit="kb";
            factor*=1024.;
         }
         if (byte_size>(unsigned long)factor*1024)
         {
            unit="mb";
            factor*=1024.;
         }
         if (byte_size>(unsigned long)factor*1024)
         {
            unit="gb";
            factor*=1024.;
         }
         std::cout<<"Spill log in memory size: "<<(double)byte_size/factor<<unit.c_str()<<std::endl;
         for (size_t i=0; i<lines; i++)
            std::cout<<InMemorySpillTable[i].c_str()<<std::endl;

      }

      if (fFlags->fWriteSpillDB)
         sqlite3_close(ppDb);

      if (fFlags->fWriteSpillTxt)
      {
         //Live spill log body:
         LiveSpillLog<<"End run " <<gRunNumber<<std::endl;
         LiveSpillLog.close();

         for (int i=0; i<NUMSEQ; i++)
         {
            LiveSequenceLog[i].close();
         }
      }
      
      if (!gIsOnline) return;

     #if 0

      char cmd[1024000];
      //if (fileCache)
      {
         //TString DataLoaderPath=outfileName(0,outfileName.Length()-5);
         TString spillLogName="R";
         spillLogName+=gRunNumber;
         spillLogName+=".log";
         std::cout <<"Log file: "<<spillLogName<<std::endl;
         std::ofstream spillLog (spillLogName);
         spillLog<<"[code]"<<log.Data()<<"[/code]"<<std::endl;
         spillLog.close();
         sprintf(cmd,"cat %s | ssh -x alpha@alphadaq /home/alpha/packages/elog/elog -h localhost -p 8080 -l SpillLog -a Run=%d -a Author=ALPHAgdumps &",spillLogName.Data(),gRunNumber);
         printf("--- Command: \n%s\n", cmd);
         if ( fFlags->fWriteElog )
            system(cmd);
      }

      for (int i=0; i<MAXDET; i++)
      {
         for (int j=0; j<CHRONO_N_BOARDS; j++)
         {
            if (DetectorChans[j][i]<0) continue;
            printf("Detector: %s\n",detectorName[i].Data());
            printf("Channel: %d\n",DetectorChans[j][i]);
            printf("DETSIZE:%zu\n",DetectorTS[i].size());
            printf("COUNTSIZE:%zu\n",DetectorCounts[i].size());
         }
      }
      for (int i = 0; i < USED_SEQ; i++)
      {
         int iSeqType=USED_SEQ_NUM[i];
         std::cout<<"Seq dumps (starts and stops)"<<iSeqType<<" - " << DumpMarkers[iSeqType][0].size()<<"+"<< DumpMarkers[iSeqType][1].size()<<std::endl;
         std::cout<<"Start triggers: "<< StartTime[iSeqType].size();
         std::cout<< " - Stop Triggers: " << StopTime[iSeqType].size()<<std::endl;
      }

      LogSpills();
      Spill_List.clear();

      #endif
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::PauseRun, run %d\n", runinfo->fRunNo);
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

      const AGSpillFlow* SpillFlow= flow->Find<AGSpillFlow>();
      if (SpillFlow)
      {
         for (size_t i=0; i<SpillFlow->spill_events.size(); i++)
         {
            TAGSpill* s=SpillFlow->spill_events.at(i);

            //Add spills that just have text data
            if (!s->IsDumpType && !s->IsInfoType)
            {
                InMemorySpillTable.push_back(s->Name.c_str());
                //continue;
            }
            //Add spills that have analysis data in (eg Catching efficiency: Cold Dump / Hot Dump)
            if (s->IsInfoType)
            {
                //s->Print();
                InMemorySpillTable.push_back(s->Content(&chrono_channels,n_chrono_channels).Data());
                continue;
            }
            if (!s->SeqData) continue;

            if (fFlags->fWriteSpillTxt)
               LiveSpillLog<<s->Content(&chrono_channels,n_chrono_channels);
            if (fFlags->fWriteSpillDB)
               s->AddToDatabase(ppDb,stmt);
            if (!fFlags->fNoSpillSummary)
               InMemorySpillTable.push_back(s->Content(&chrono_channels,n_chrono_channels).Data());
            SaveToTree(runinfo,s);
         }
      }

      #ifdef _TIME_ANALYSIS_
         if (TimeModules) flow=new AgAnalysisReportFlow(flow,"spill_log_module",timer_start);
      #endif
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("SpillLog::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
   }
};

class SpillLogFactory: public TAFactory
{
public:
   SpillLogFactory(): TAFactory()
   {
      std::cout<<"Please run me as: ./ag_events.exe -g -Halphagdaq.cern.ch "<<std::endl;
      gEnv->SetValue("Gui.DefaultFont","-*-courier-medium-r-*-*-12-*-*-*-*-*-iso8859-1");  
   }
   SpillLogFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("SpillLogFactory::Init!\n");
      

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--elog")
            fFlags.fWriteElog = true;
      }
  if(gROOT->IsBatch()) {
    printf("Cannot run in batch mode\n");
    exit (1);
  }
 
   }

   void Finish()
   {
      printf("SpillLogFactory::Finish!\n");
   }
   
   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("SpillLogFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new SpillLog(runinfo, &fFlags);
   }
};

static TARegister tar(new SpillLogFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */