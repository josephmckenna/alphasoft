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
   int seqcount[NUMSEQ]; //Count sequences run

   std::vector<DumpMarker> DumpMarkers[NUMSEQ][2];
   int DumpPosition[NUMSEQ]={0};  
   //Live spill log body:
   std::ofstream LiveSpillLog;
   //Column headers
   std::ofstream SpillLogHeader;
   //List of active dumps
   std::ofstream LiveSequenceLog[NUMSEQ];
   
   std::vector<int> sis_channels;
   int n_sis_channels;
   
   //
   std::vector<std::string> InMemorySpillTable;
   
   TTree* SpillTree = NULL;

private:
   sqlite3 *ppDb; //SpillLogDatabase handle
   sqlite3_stmt * stmt;
public:
   
   
   SpillLog(TARunInfo* runinfo, SpillLogFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("SpillLog::ctor!\n");
      
      // load the sqlite3 db
      TSISChannels* sisch = new TSISChannels(runinfo->fRunNo);
      std::vector<std::string> channels={"SIS_PMT_CATCH_OR","SIS_PMT_CATCH_AND","SIS_PMT_ATOM_OR","SIS_PMT_ATOM_AND","PMT_12_AND_13","IO32_TRIG_NOBUSY","PMT_10","ATOMSTICK"};
  
      for (size_t i=0; i<channels.size(); i++)
         sis_channels.push_back(sisch->GetChannel(channels.at(i).c_str()));
      
      n_sis_channels=sis_channels.size();
   }

   ~SpillLog()
   {
      if (fTrace)
         printf("SpillLog::dtor!\n");
   }
   void SaveToTree(TARunInfo* runinfo,A2Spill* s)
   {
         if (!s) return;
         #ifdef HAVE_CXX11_THREADS
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
         #endif
         runinfo->fRoot->fOutputFile->cd();
         if (!SpillTree)
            SpillTree = new TTree("A2SpillTree","A2SpillTree");
         TBranch* b_variable = SpillTree->GetBranch("A2Spill");
         if (!b_variable)
            SpillTree->Branch("A2Spill","A2Spill",&s,16000,1);
         else
            SpillTree->SetBranchAddress("A2Spill",&s);
         SpillTree->Fill();
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
      if (!fFlags->fNoSpillSummary)
      {
         InMemorySpillTable.push_back("Begin run "+std::to_string(runinfo->fRunNo) );
         InMemorySpillTable.push_back("             Dump Time            | CAT Event       RCT Event       ATM Event       POS Event        | CATCH_OR  CATCH_AND ATOM_OR   ATOM_AND  CTSTICK   IO32_TRIG ATOMSTICK NewATOMSTICK ");
         InMemorySpillTable.push_back("---------------------------------------------------------------------------------------------------------------------------------------------------------------------");
      }
      if (fFlags->fWriteSpillTxt)
      {
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

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory

   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::EndRun, run %d\n", runinfo->fRunNo);
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
               InMemorySpillTable.push_back(new_seq_msg.Data());
            if (incomplete_starts && incomplete_stops)
            {
               int j=DumpPosition[iSeq];
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
                     InMemorySpillTable.push_back(miss_match.Data());
                  }
                  j++;
               }
            }
         }


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
   //   if (!gIsOnline) return flow;
       time(&gTime);  /* get current time; same as: timer = time(NULL)  */
      #ifdef _TIME_ANALYSIS_
      START_TIMER
      #endif 

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
            InMemorySpillTable.push_back(new_seq_msg.Data());

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
               if (fFlags->fWriteSpillTxt)
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
               DumpStartName=NULL;
            else
               DumpStartName=DumpMarkers[thisSeq][0].at(DumpPosition[thisSeq]).Description.Data();

            const char* DumpStopName;
            if (DumpPosition[thisSeq]>=(int)DumpMarkers[thisSeq][1].size())
               DumpStopName=NULL;
            else
               DumpStopName=DumpMarkers[thisSeq][1].at(DumpPosition[thisSeq]).Description.Data();

            if (DumpStartName)
            {
               s->Name=DumpStartName;
               DumpPosition[thisSeq]++;
            }
            if (!DumpStartName)
            {
               //If a name hasn't been already assigned to the dump (maybe from catch_efficiency_module)
               if (strcmp(s->Name.c_str(),"")==0)
                  s->Name="MISSING_DUMP_NAME";
               DumpStartName="MISSING_DUMP_NAME";
            }
            if (!DumpStopName)
               DumpStopName="MISSING_DUMP_NAME";

            if (strcmp(DumpStartName,DumpStopName)!=0)
            {
               char error[100];
               sprintf(error,"Miss matching dump names! %s AND %s",DumpStartName,DumpStopName);
               std::cerr<<error<<std::endl;
               if (!fFlags->fNoSpillSummary)
                  InMemorySpillTable.push_back(error);
               if (fFlags->fWriteSpillTxt)
                  LiveSpillLog<<error<<std::endl;
            }
            if (fFlags->fWriteSpillTxt)
               LiveSpillLog<<s->Content(&sis_channels,n_sis_channels);
            if (fFlags->fWriteSpillDB)
               s->AddToDatabase(ppDb,stmt);
            if (!fFlags->fNoSpillSummary)
               InMemorySpillTable.push_back(s->Content(&sis_channels,n_sis_channels).Data());
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
