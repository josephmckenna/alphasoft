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
#include "RecoFlow.h"

#include "store_cb.h"
#include "TChronoChannel.h"
#include "TChronoChannelName.h"
#include "TROOT.h"
#include "TTree.h"

#include <vector>
//MAX DET defined here:
#include "TSpill.h"

#ifdef HAVE_MIDAS
#include "midas.h"
#include "msystem.h"
#include "mrpc.h"
#endif

#define MAXDET 10

#define DELETE(x) if (x) { delete (x); (x) = NULL; }

#define MEMZERO(p) memset((p), 0, sizeof(p))

#define HOT_DUMP_LOW_THR 500

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
   bool fOnlineSpillLog = false;
};

#ifdef HAVE_MIDAS
class SpillLogPrinter
{
   private:
      TString fSpillLogTitle;
      int fSpillLogLineNumber;
      const int fPrintInterval;
   public:
   SpillLogPrinter(): fPrintInterval(25)
   {
     fSpillLogLineNumber = 0;
   }
   ~SpillLogPrinter()
   {
     // The new manalzyer calls the dtor between runs... 
     //cm_msg1(MINFO, "SpillLog", "alphagonline","Exiting...");
   }
   void Reset()
   {
     fSpillLogLineNumber = 0;
   }
   void BeginRun(TString SpillLogTitle, int RunNo)
   {
      Reset();
      if (!RunNo)
         return;
      fSpillLogTitle = SpillLogTitle;
      int width = fSpillLogTitle.Length();
      
      int indent = width / 2 - 52/2;
      std::string logo_indent(indent,' ');
      //For readability make a large break from the last run
      cm_msg1(MINFO, "SpillLog", "alphagonline","%s%s", logo_indent.c_str(), "           /_/ |_/____/_/  /_//_/_/ |_|    ");
      cm_msg1(MINFO, "SpillLog", "alphagonline", "%s%s", logo_indent.c_str(), "/___/___/___/ __ |/ /__/ ___/ _  / __ /___/___/___/");
      cm_msg1(MINFO, "SpillLog", "alphagonline", "%s%s", logo_indent.c_str(), " ____________/ _ | / /  / _ \\/ // / _ |____________");
      cm_msg1(MINFO, "SpillLog", "alphagonline", "%s%s", logo_indent.c_str(), "              ___   __   ___  __ _____             ");


      std::string line(width,'-');
      std::string first_line(line);
      std::string title = std::string("# Begin run ") + std::to_string(RunNo) + std::string(" #");
      if (title.size() < first_line.size() + 10)
      {
         int title_pos = 0;
         int line_pos = line.size() / 2 - title.size() / 2; 
         while (title_pos < title.size())
         {
            first_line.at(line_pos++ ) = title.at(title_pos++);
         }
      }
      else
      {
         first_line = title;
      }
      
      //cm_msg_log((INT)MINFO, (const char*) "SpillLog", (const char*) first_line.c_str() );
      cm_msg1(MINFO, "SpillLog", "alphagonline", "%s", first_line.c_str());
      //cm_msg1(MINFO, "SpillLog", "alphagonline", "%s", line.c_str());
      cm_msg1(MINFO, "SpillLog", "alphagonline", "%s", fSpillLogTitle.Data());
      cm_msg1(MINFO, "SpillLog", "alphagonline", "%s", line.c_str());
      fSpillLogLineNumber++;
   }
   void EndRun(int RunNo)
   {
      int width = fSpillLogTitle.Length();
      std::string line(width,'=');
      cm_msg1(MINFO, "SpillLog", "alphagonline","%s", line.c_str());  
      cm_msg1(MINFO, "SpillLog", "alphagonline","End run %d",RunNo);
   }
   
   void PrintLine(const char *string)
   {
      fSpillLogLineNumber++;
      if ( (fSpillLogLineNumber % fPrintInterval ) == 0 )
      {
        cm_msg1(MINFO, "SpillLog", "alphagonline", "%s", fSpillLogTitle.Data());
      }
      cm_msg1(MINFO, "SpillLog", "alphagonline", "%s", string);
   }
   
};
#endif

class SpillLog: public TARunObject
{
public: 
   SpillLogFlags* fFlags;
   bool fTrace = false;
   Int_t RunState =-1;
   Int_t gRunNumber =0;
   time_t run_start_time=0;
   time_t run_stop_time=0;
#ifdef HAVE_MIDAS
   SpillLogPrinter fSpillLogPrinter;
#endif
   std::vector<std::string> InMemorySpillTable;
   TTree* SpillTree = NULL;

   //Live spill log body:
   std::ofstream LiveSpillLog;
   //Column headers
   std::ofstream SpillLogHeader;
   //List of active dumps
   std::ofstream LiveSequenceLog[NUMSEQ];

   std::vector<TChronoChannel> chrono_channels;
   int n_chrono_channels;


   //Dump Markers to give timestamps (From DumpFlow)
   //std::vector<TString> Description[4];
   //std::vector<Int_t> DumpType[4]; //1=Start, 2=Stop
   //std::vector<Int_t> fonCount[4];
   // Int_t SequencerNum[4];
   Int_t gADSpillNumber;
   TSpill* gADSpill;
   TChronoChannel gADSpillChannel={"",-1};
   
   Int_t gPOSSpillNumber;
   TChronoChannel gPOSSpillChannel={"",-1};
private:
   sqlite3 *ppDb; //SpillLogDatabase handle
   sqlite3_stmt * stmt;
public:
   TString SpillLogTitle;

   SpillLog(TARunInfo* runinfo, SpillLogFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
#ifdef HAVE_MANALYZER_PROFILER
      fModuleName="Spill Log Module";
#endif
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
         std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
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

      std::vector<std::string> channels =
      {
         //channel name, descriptive name
         "PMT_CATCH_OR",
         "SiPM_B",
         "SiPM_E",
         "TPC_TRIG",
         "SiPM_A_AND_D",
         "SiPM_C_AND_F",
         "SiPM A_OR_C-AND-D_OR_F",
         "SiPM_C",
         "SiPM_D",
         "SiPM_F"
      };

      std::vector<std::string> channel_names =
      {
         //channel name, descriptive name
         "CATCH OR",
         "TOP PM",
         "BOT PM",
         "TPC",
         "SiPM_A_AND_D",
         "SiPM_C_AND_F",
         "SiPM A_OR_C-AND-D_OR_F",
         "SiPM_C",
         "SiPM_D",
         "SiPM_F"
      };
      MVOdb* channel_settings = runinfo->fOdb->Chdir("Equipment/alphagonline/Settings", true);
      channel_settings->RSA("ChannelIDName",&channels, true, 60, 250);
      channel_settings->RSA("ChannelDisplayName",&channel_names, true, 60, 250);
      //Remove empty channels
      std::vector<std::string> tmpchan(channels);
      std::vector<std::string> tmpnames(channel_names);
      channels.clear();
      channel_names.clear();
      for (int i = 0; i < tmpchan.size(); i++)
      {
         if (tmpchan.at(i).size() || tmpnames.at(i).size())
         {
            channels.emplace_back(tmpchan.at(i));
            channel_names.emplace_back(tmpnames.at(i));
         }
      }
      //Print channel list into spill log
      std::string channel_summary = "Channel List: ";
      for (int i = 0; i < channels.size(); i++)
      {
         if (channels.at(i).empty()) continue;
         channel_summary += channels.at(i);
         channel_summary += " = ";
         channel_summary += channel_names.at(i);
         if (i != channels.size() - 1)
         channel_summary +=", ";
      }
#if HAVE_MIDAS
      cm_msg1(MINFO, "SpillLog", "alphagonline", channel_summary.c_str());
      //Check the number of Channels and Channel names match
      int n_chans = 0;
      int n_names = 0;
      for (const std::string& s: channels)
      {
         if (s.size())
            n_chans++;
      }
      for (const std::string& s: channel_names)
      {
         if (s.size())
            n_names++;
      }
      if (n_chans != n_names)
         cm_msg1(MERROR, "SpillLog", "alphagonline", "ChannelIDName entires (%d) does not match ChannelDisplayName entires (%d)",n_chans, n_names);
#endif


      for (size_t i=0; i<channels.size(); i++)
      {
         bool found = false;
         for (const std::pair<std::string, int>& board: CBMAP)
         {
            TChronoChannelName name(runinfo->fOdb,board.first);
            int channel = name.GetChannel(channels.at(i));
            std::cout<<"CHANNEL"<<channel<<std::endl;
            if (channel>0)
            {
               found = true;
               chrono_channels.emplace_back(TChronoChannel(board.first,channel));
               break;
            }
         }
         if (!found)
            chrono_channels.emplace_back(TChronoChannel("",-1));
      }


      n_chrono_channels=chrono_channels.size();

      for (auto c: chrono_channels)
         std::cout<<"\t"<<c<<std::endl;
      SpillLogTitle="            Dump Time            | ";
      std::string dump_names = "";
      for (int i = 0; i < NUMSEQ; i++)
      {
         dump_names += SEQ_NAMES_SHORT[i];
         dump_names += " ";
      }
      assert(dump_names.size() < DUMP_NAME_WIDTH - 1);
      //Pad to proper width
      dump_names.insert(dump_names.size(), DUMP_NAME_WIDTH - dump_names.size() - 1, ' ');
      SpillLogTitle += dump_names + std::string("|");
      char buf[200];
      for (size_t i=0; i<channel_names.size(); i++)
      {
         if (channels.at(i).empty()) continue;
         sprintf(buf,"%9s ",channel_names.at(i).c_str());
         SpillLogTitle+=buf;
      }

#ifdef HAVE_MIDAS
      if (runinfo->fRunNo)
      {
         fSpillLogPrinter.BeginRun(SpillLogTitle, runinfo->fRunNo);
      }
#endif
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
         InMemorySpillTable.push_back(std::string(SpillLogTitle.Length(),'-'));
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
         SpillLogHeader<<std::string(SpillLogTitle.Length(),'-')<<std::endl;
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
      runinfo->fOdb->RU32("Runinfo/Start time binary",(uint32_t*) &run_start_time);
      runinfo->fOdb->RU32("Runinfo/Stop time binary",(uint32_t*) &run_stop_time);
      runinfo->fOdb->RI("runinfo/State",&RunState);
#endif

      std::cout<<"START:"<< run_start_time<<std::endl;
      std::cout<<"STOP: "<< run_stop_time<<std::endl;

      runinfo->fRoot->fOutputFile->cd(); // select correct ROOT directory
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("SpillLog::EndRun, run %d\n", runinfo->fRunNo);
      //runinfo->State
      //if (fFlags->fOnlineSpillLog)
      if (runinfo->fRunNo)
      {
#ifdef HAVE_MIDAS
         fSpillLogPrinter.EndRun(runinfo->fRunNo);
#else
         std::cout<<"WARNING: fOnlineSpillLog set but software not build with MIDAS"<<std::endl;
#endif

      }
 //     SpillTree->Write();

      InMemorySpillTable.push_back("End run");
      InMemorySpillTable.push_back(std::string(SpillLogTitle.Length(),'-'));
      const size_t lines = InMemorySpillTable.size();
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
      if (!fFlags->fNoSpillSummary)
         for (size_t i=0; i<lines; i++)
            std::cout<<InMemorySpillTable[i].c_str()<<std::endl;

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
      
      //if (fileCache)
      if (runinfo->fRunNo)
      {
         //TString DataLoaderPath=outfileName(0,outfileName.Length()-5);
         TString spillLogName="R";
         spillLogName+=gRunNumber;
         spillLogName+=".log";
         std::cout <<"Log file: "<<spillLogName<<std::endl;
         std::ofstream spillLog (spillLogName);
         spillLog<<"[code]";
         for (size_t i=0; i<lines; i++)
            spillLog<<InMemorySpillTable[i].c_str()<<std::endl;
         spillLog<<"[/code]"<<std::endl;
         spillLog.close();
#ifdef HAVE_MIDAS
         char cmd[200]={0};
         sprintf(cmd,"cat %s | ssh -x alpha@alphadaq /home/alpha/packages/elog/elog -h localhost -p 8080 -l SpillLog -a Run=%d -a Author=alphagonline &",spillLogName.Data(),gRunNumber);
         printf("--- Command: \n%s\n", cmd);
         if ( fFlags->fWriteElog )
            system(cmd);
#endif
      }
#if 0
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
      const TInfoSpillFlow* TInfoFlow= flow->Find<TInfoSpillFlow>();
      if (TInfoFlow)
      {
         for (TInfoSpill* s: TInfoFlow->spill_events)
         {
            InMemorySpillTable.push_back(s->Name.c_str());
#ifdef HAVE_MIDAS
                fSpillLogPrinter.PrintLine(s->Name.c_str());
#endif
         }
      }
      const AGSpillFlow* SpillFlow= flow->Find<AGSpillFlow>();
      if (SpillFlow)
      {
         for (size_t i=0; i<SpillFlow->spill_events.size(); i++)
         {
            TAGSpill* s = SpillFlow->spill_events.at(i);

            //Add spills that just have text data
            if (!s->IsDumpType && !s->IsInfoType)
            {
                InMemorySpillTable.push_back(s->Name.c_str());
#ifdef HAVE_MIDAS
                fSpillLogPrinter.PrintLine(s->Name.c_str());
#endif
                //continue;
            }
            //Add spills that have analysis data in (eg Catching efficiency: Cold Dump / Hot Dump)
            if (s->IsInfoType)
            {
                //s->Print();
                InMemorySpillTable.push_back(s->Content(chrono_channels).Data());
#ifdef HAVE_MIDAS
                fSpillLogPrinter.PrintLine(s->Content(chrono_channels).Data());
#endif
                continue;
            }
            if (!s->SeqData) continue;

            if (fFlags->fWriteSpillTxt)
               LiveSpillLog<<s->Content(chrono_channels);
            if (fFlags->fWriteSpillDB)
               s->AddToDatabase(ppDb,stmt);
            if (!fFlags->fNoSpillSummary)
               InMemorySpillTable.push_back(s->Content(chrono_channels).Data());
#ifdef HAVE_MIDAS
            fSpillLogPrinter.PrintLine(s->Content(chrono_channels).Data());
#endif
            SaveToTree(runinfo,s);
         }
      }
      else
      {
#ifdef HAVE_MANALYZER_PROFILER
         *flags|=TAFlag_SKIP_PROFILE;
#endif
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
      std::cout<<"SpillLogFactor::Help!"<<std::endl;
      std::cout<<"\t--elog\t\tWrite elog (not implemented)"<<std::endl;
      std::cout<<"\t--spilldb\t\tSwrite to Spill log sqlite database (local)"<<std::endl;
      std::cout<<"\t--spilltxt\t\tWrite Spill log to SpillLog/reload.txt"<<std::endl;
      std::cout<<"\t--nospillsummary\t\tTurn off spill log table printed at end of run"<<std::endl;
      std::cout<<"\t--onlinespills\t\tWrite spills live to SpillLog in midas"<<std::endl;
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
         if (args[i] == "--onlinespills")
            fFlags.fOnlineSpillLog = true;
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
