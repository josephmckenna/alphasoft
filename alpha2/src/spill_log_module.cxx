//
// ALPHA 2 (SVD AND SIS) spill_log_module
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

time_t LastUpdate;
//struct tm LastUpdate = {0};


#ifdef HAVE_XMLSERVER
#include "xmlServer.h"
#include "TROOT.h"
#endif

#ifdef XHAVE_LIBNETDIRECTORY
#include "netDirectoryServer.h"
#endif

#include "TSISChannels.h"

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
   
   std::vector<int> sis_channels;
   int n_sis_channels;

private:
   sqlite3 *ppDb; //SpillLogDatabase handle
   sqlite3_stmt * stmt;
public:
   TString SpillLogTitle;
   
   SpillLog(TARunInfo* runinfo, SpillLogFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      ModuleName="SpillLog";
      if (fTrace)
         printf("SpillLog::ctor!\n");
      
      // load the sqlite3 db
      TSISChannels* sisch = new TSISChannels(runinfo->fRunNo);
      std::vector<std::pair<std::string,std::string>> channels=
      {
         {"SIS_PMT_CATCH_OR","Catch OR"},
         {"SIS_PMT_CATCH_AND","Catch AND"},
         {"SIS_PMT_ATOM_OR","Atom OR"},
         {"SIS_PMT_ATOM_AND","Atom AND"},
         {"PMT_12_AND_13","CT Stick"},
         {"IO32_TRIG_NOBUSY","IO32_TRIG"},
         {"PMT_10","PMT 10"},
         {"ATOMSTICK","Atom Stick"}
      };
  
      for (size_t i=0; i<channels.size(); i++)
         sis_channels.push_back(sisch->GetChannel(channels.at(i).first.c_str()));
      SpillLogTitle="             Dump Time            | CAT Event       RCT Event       ATM Event       POS Event        |";
      char buf[200];
      for (size_t i=0; i<channels.size(); i++)
      {
         sprintf(buf,"%9s ",channels.at(i).second.c_str());
         SpillLogTitle+=buf;
      }
      sprintf(buf,"%9s %9s ","Cuts","MVA");
      SpillLogTitle+=buf;
      n_sis_channels=sis_channels.size();
      delete sisch;
   }

   ~SpillLog()
   {
      if (fTrace)
         printf("SpillLog::dtor!\n");
   }
   void SaveToTree(TARunInfo* runinfo,TA2Spill* s)
   {
         if (!s) return;
         #ifdef HAVE_CXX11_THREADS
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         #endif
         runinfo->fRoot->fOutputFile->cd();
         if (!SpillTree)
            SpillTree = new TTree("A2SpillTree","A2SpillTree");
         TBranch* b_variable = SpillTree->GetBranch("TA2Spill");
         if (!b_variable)
            SpillTree->Branch("TA2Spill","TA2Spill",&s,16000,1);
         else
            SpillTree->SetBranchAddress("TA2Spill",&s);
         SpillTree->Fill();
   }


   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::BeginRun, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
     
      if (fFlags->fWriteSpillDB)
      {
         if (sqlite3_open("SpillLog/A2SpillLog.db",&ppDb) == SQLITE_OK)
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
         //InMemorySpillTable.push_back("             Dump Time            | CAT Event       RCT Event       ATM Event       POS Event        | CATCH_OR  CATCH_AND ATOM_OR   ATOM_AND  CTSTICK   IO32_TRIG ATOMSTICK NewATOMSTICK ");
         InMemorySpillTable.push_back("---------------------------------------------------------------------------------------------------------------------------------------------------------------------");
      }
      if (fFlags->fWriteSpillTxt)
      {
         //Live spill log body:
         LiveSpillLog.open("SpillLog/reload.txt");
         LiveSpillLog<<"Begin run "<<runinfo->fRunNo<<"\t\t"<<std::endl;

         //Column headers
         SpillLogHeader.open("SpillLog/title.txt");
         SpillLogHeader<<SpillLogTitle.Data();
         //SpillLogHeader<<"             Dump Time            | CAT Event       RCT Event       ATM Event       POS Event        | CATCH_OR  CATCH_AND ATOM_OR   ATOM_AND  CTSTICK   IO32_TRIG ATOMSTICK NewATOMSTICK "<<std::endl;
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
      const A2SpillFlow* SpillFlow= flow->Find<A2SpillFlow>();
      if (SpillFlow)
      {
         for (size_t i=0; i<SpillFlow->spill_events.size(); i++)
         {
            TA2Spill* s=SpillFlow->spill_events.at(i);

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
                InMemorySpillTable.push_back(s->Content(&sis_channels,n_sis_channels).Data());
                continue;
            }
            if (!s->SeqData) continue;

            if (fFlags->fWriteSpillTxt)
               LiveSpillLog<<s->Content(&sis_channels,n_sis_channels);
            if (fFlags->fWriteSpillDB)
               s->AddToDatabase(ppDb,stmt);
            if (!fFlags->fNoSpillSummary)
               InMemorySpillTable.push_back(s->Content(&sis_channels,n_sis_channels).Data());
            SaveToTree(runinfo,s);
         }
      }
      else
      {
         *flags|=TAFlag_SKIP_PROFILE;
      }
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
         if (args[i] == "--elog")
            fFlags.fWriteElog = true;
         if (args[i] == "--spilldb")
            fFlags.fWriteSpillDB = true;
         if (args[i] == "--spilltxt")
            fFlags.fWriteSpillTxt = true;
         if (args[i] == "--nospillsummary")
            fFlags.fNoSpillSummary = true;
      }
 
   }

   void Finish()
   {
      if (fFlags.fPrint)
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
