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

time_t gTime; // system timestamp of the midasevent
time_t LastUpdate;
//struct tm LastUpdate = {0};

std::list<A2Spill*> Spill_List;


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
   bool fPrintMixingSummary = false;
   bool fPrintBackgroundSummary = false;
};

class DumpSummary
{
public:
   std::string DumpName;
   int PassedCuts;
   int Verticies;
   int VF48Events;
   double time;
   int TotalCount;
   DumpSummary(const char* name)
   {
      DumpName=name;
      PassedCuts=0;
      Verticies=0;
      VF48Events=0;
      time=0.;
      TotalCount=0;
   }
   void Fill(A2Spill* s)
   {
      //std::cout<<"Adding spill to list"<<std::endl;
      PassedCuts+=s->PassCuts;
      Verticies+=s->Verticies;
      VF48Events+=s->VF48Events;
      time+=s->StopTime-s->StartTime;
      TotalCount++;
   }
   void Print()
   {
      printf("DUMP SUMMARY:%s\t DumpCount:%d\t VF48Events:%d \tVerticies:%d\t PassedCuts:%d\t TotalTime:%f\t\n",
                   DumpName.c_str(),
                   TotalCount,
                   VF48Events,
                   Verticies,
                   PassedCuts,
                   time);
   }
};

class DumpSummaryList
{
public:
   std::vector<DumpSummary*> list;
   DumpSummaryList() {}
   ~DumpSummaryList()
   {
      const int size=list.size();
      for (int i=0; i<size; i++)
         delete list[i];
      list.clear();
   }
   
   void TrackDump(const char* d)
   {
      list.push_back(new DumpSummary(d));
      return;
   }
   void Fill(A2Spill* s)
   {
      //std::cout<<"Filling list "<<s->Name.c_str()<<std::endl;
      const int size=list.size();
      if (!size) return;
      for (int i=0; i<size; i++)
      {
         //std::cout<<list[i]->DumpName.c_str()<<"vs"<<s->Name.c_str()<<std::endl;
         if (strcmp(list[i]->DumpName.c_str(),s->Name.c_str())==0)
            list[i]->Fill(s);
      }
      return;
   }
   void Print()
   {
      const int size=list.size();
      for (int i=0; i<size; i++)
         list[i]->Print();
   }
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
   int seqcount[NUMSEQ]; //Count sequences run

   std::vector<DumpMarker> DumpMarkers[NUMSEQ][2];
   int DumpPosition[NUMSEQ]={0};  
   //Live spill log body:
   std::ofstream LiveSpillLog;
   //Column headers
   std::ofstream SpillLogHeader;
   //List of active dumps
   std::ofstream LiveSequenceLog[NUMSEQ];
   
   DumpSummaryList DumpLogs;
   
private:
   sqlite3 *ppDb; //SpillLogDatabase handle
   sqlite3_stmt * stmt;
public:
   
   
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



   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
     
      if (fFlags->fWriteSpillDB)
      {
         if (sqlite3_open("SpillLog/SpillLog.db",&ppDb) == SQLITE_OK)
         {
            std::cout<<"Database opened ok"<<std::endl;
         }
         else
         {
            exit(555);
         }
      }
      //Live spill log body:
      LiveSpillLog.open("SpillLog/reload.txt");
      
      LiveSpillLog<<"Begin run "<<runinfo->fRunNo<<"\t\t"<<std::endl;
      
      //Column headers
      SpillLogHeader.open("SpillLog/title.txt");
      SpillLogHeader<<"             Dump Time            | CAT Event       RCT Event       ATM Event       POS Event        | CATCH_OR  CATCH_AND ATOM_OR   ATOM_AND  CTSTICK   IO32_TRIG ATOMSTICK NewATOMSTICK "<<std::endl;
      SpillLogHeader<<"---------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<std::endl;
      SpillLogHeader.close();
      //List of active dumps

      for (int i=0; i<NUMSEQ; i++)
      {
         std::string name;
         
         name += "SpillLog/Sequencers/Seq";
         name += std::to_string(i);
         name += ".txt";
         std::cout<<name.c_str()<<std::endl;
         LiveSequenceLog[i].open(name.c_str());
      }
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

      Spill_List.clear();

      if (gIsOnline)
      {
      }
      if (fFlags->fPrintMixingSummary)
      {
         DumpLogs.TrackDump("\"Mixing\"");
      }
      if (fFlags->fPrintBackgroundSummary)
      {
         DumpLogs.TrackDump("\"Background\"");
      } 

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::EndRun, run %d\n", runinfo->fRunNo);
      //runinfo->State

      if (fFlags->fWriteSpillDB)
         sqlite3_close(ppDb);
      //Live spill log body:
      LiveSpillLog<<"End run " <<gRunNumber<<std::endl;
      LiveSpillLog.close();

      for (int i=0; i<NUMSEQ; i++)
      {
         LiveSequenceLog[i].close();
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
      DumpLogs.Print();
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
   //   if (!gIsOnline) return flow;
       time(&gTime);  /* get current time; same as: timer = time(NULL)  */
      #ifdef _TIME_ANALYSIS_
      clock_t timer_start=clock();
      #endif 

      const AgDumpFlow* DumpFlow = flow->Find<AgDumpFlow>();
      if (DumpFlow)
      { // I am a Dump Flow
         for (int i = 0; i < USED_SEQ; i++)
         {
            int iSeq=USED_SEQ_NUM[i];
            //Fix this to insert new vector at back (not this dumb loop)
            uint ndumps=DumpFlow->DumpMarkers[iSeq].size();
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
               LiveSequenceLog[i]<<msg<<std::endl;
               //Add the markers to a queue for timestamps later
               if (type_pos==0 || type_pos==1)
                  DumpMarkers[iSeq][type_pos].push_back(DumpFlow->DumpMarkers[iSeq].at(j));
            }
         }
      }

      const A2SpillFlow* SpillFlow= flow->Find<A2SpillFlow>();
      if (SpillFlow)
      {
         for (size_t i=0; i<SpillFlow->spill_events.size(); i++)
         {
            A2Spill* s=SpillFlow->spill_events.at(i);
            //s->Print();
            int thisSeq=s->SequenceNum;
            s->DumpID=DumpPosition[thisSeq];

            const char* DumpStartName;
            if (DumpPosition[thisSeq]>=(int)DumpMarkers[thisSeq][0].size())
               DumpStartName="MISSING_DUMP_NAME";
            else
               DumpStartName=DumpMarkers[thisSeq][0].at(DumpPosition[thisSeq]).Description.Data();

            const char* DumpStopName;
            if (DumpPosition[thisSeq]>=(int)DumpMarkers[thisSeq][1].size())
               DumpStopName="MISSING_DUMP_NAME";
            else
               DumpStopName=DumpMarkers[thisSeq][1].at(DumpPosition[thisSeq]).Description.Data();

            s->Name=DumpStartName;
            DumpPosition[thisSeq]++;
            if (strcmp(DumpStartName,DumpStopName)!=0)
               LiveSpillLog<<"Miss matching dump names!"<<DumpStartName <<" AND "<< DumpStopName<<std::endl;
            LiveSpillLog<<s->Content()<<std::endl;
            if (fFlags->fWriteSpillDB)
               s->AddToDatabase(ppDb,stmt);
            DumpLogs.Fill(s);

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
   void Usage()
   {
      std::cout<<"\t--elog\t\tWrite elog"<<std::endl;
      std::cout<<"\t--spilldb\t\tSwrite to Spill log sqlite database (local)"<<std::endl;
      std::cout<<"\t--mixingsummary\t\tPrint SVD summary for mixing dumps"<<std::endl;
      std::cout<<"\t--backgroundsummary\t\tPrint SVD summary for background dumps"<<std::endl;
   }
   void Init(const std::vector<std::string> &args)
   {
      printf("SpillLogFactory::Init!\n");
      

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--print")
            fFlags.fPrint = true;
         if (args[i] == "--elog")
            fFlags.fWriteElog = true;
         if (args[i] == "--spilldb")
            fFlags.fWriteSpillDB = true;
         if (args[i] == "--mixingsummary")
            fFlags.fPrintMixingSummary = true;
         if (args[i] == "--backgroundsummary")
            fFlags.fPrintBackgroundSummary = true;
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
