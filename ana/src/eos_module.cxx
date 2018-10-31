//
// Module to handle fetching files from EOS
// JTK McKENNA
//




#include "manalyzer.h"
#include "midasio.h"
#include "AgFlow.h"

#include <iostream>
#include "AnalysisTimer.h"
#include "TSystem.h"
#include <sys/stat.h>

class EOSFlags
{
public:
   bool fPrint = false;
   bool fEOS = false;
   bool fCustomOutput = false;
   TString fCustomOutputName = "auto";
};

class EOS: public TARunObject
{
private:
   int RunNumber;
   int subrun;
   std::vector<bool> SubRunFetched;
   bool SendTimeReport;
   bool SkipSpecial;
public:
   EOSFlags* fFlags;
   bool fTrace = true;

   EOS(TARunInfo* runinfo, EOSFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (fTrace)
         printf("EOS::ctor!\n");
      if (flags->fCustomOutput)
      {
         TARootHelper* h=runinfo->fRoot;
         //h->fOutputFile->Write();
         //h->fOutputFile->Close();
         delete h->fOutputFile;
         h->fOutputFile=NULL;
         if (flags->fCustomOutputName.EqualTo("auto"))
         {
            flags->fCustomOutputName="data/tree";
            if (runinfo->fRunNo<10000)
               flags->fCustomOutputName+="0";
            flags->fCustomOutputName+=runinfo->fRunNo;
            flags->fCustomOutputName+="offline.root";
         }
         h->fOutputFile=new TFile(flags->fCustomOutputName.Data(), "RECREATE");
         assert(h->fOutputFile->IsOpen()); // FIXME: survive failure to open ROOT file
         h->fOutputFile->cd();
      }
   }

   ~EOS()
   {
      if (fTrace)
         printf("EOS::dtor!\n");
   }

   TString MidasFileName(int runno, int sub)
   {
      TString filename="run";
      if (runno<10000)
         filename+=0;
      filename+=runno;
      filename+="sub";
      if (sub<10)
         filename+="0";
      if (sub<100)
         filename+="0";
      filename+=sub;
      filename+=".mid.lz4";
      return filename;
   }

   int CheckMidasFileOnEOS(TString filename)
   {
      if( (strncmp(gSystem->HostName(),"alphacpc",8)==0) || //AND I am NOT an alphacpc* machine (a safety system to stop deletion of files)
         (strncmp(gSystem->HostName(),"alphagdaq",8)==0) || //AND I am NOT an alphagdaq* machine (a safety system to stop deletion of files)
         (strncmp(gSystem->HostName(),"alphadaq",8)==0) ) //AND I am NOT an alphadaq* machine (a safety system to stop deletion of files)
            {
               std::cerr <<"EOS::This machine is blacklisted from using --EOS flag"<<std::endl;
               return -99;
            }
      if (filename.Contains("midasdata/"))
         filename.Remove(0,10);
      TString EOSdir="/eos/experiment/ALPHAg/midasdata_old/";
      EOSdir+=filename;
      //There isn't get a midas folder...
      //TString LocalPath=getenv("MIDASDIR");
      //LocalPath+="/";
      TString LocalPath=filename;
      TString EOScheck="eos ls -l ";
      EOScheck+=EOSdir;
      EOScheck+=" | awk '{print $5}'";
      Int_t InitialSize=gSystem->GetFromPipe(EOScheck).Atoi();
      if (InitialSize>0)
         {
            std::cout<<filename<< " found and not empty"<<std::endl;
            gSystem->Sleep(5000);
            Int_t SizeAfterWait=gSystem->GetFromPipe(EOScheck).Atoi();
            if (InitialSize==SizeAfterWait)
               {
                  std::cout <<"EOS::File size confirmed to be :" << SizeAfterWait <<std::endl;
                  return 1;
               }
            else
               {
                  std::cout <<"EOS::File still being updated... try again later"<<std::endl;
                  return 0;
               }
         }
      else
         {
            std::cout <<"File "<<filename<< " not found" <<std::endl;
            return -1;
         }
   }

   int CopyMidasFileFromEOS(TString filename, Int_t AllowedRetry=5)
   {
      if( (strncmp(gSystem->HostName(),"alphacpc",8)==0) || //AND I am NOT an alphacpc* machine (a safety system to stop deletion of files)
         (strncmp(gSystem->HostName(),"alphagdaq",8)==0) || //AND I am NOT an alphagdaq* machine (a safety system to stop deletion of files)
         (strncmp(gSystem->HostName(),"alphadaq",8)==0) ) //AND I am NOT an alphadaq* machine (a safety system to stop deletion of files)
            {
               std::cerr <<"EOS::This machine is blacklisted from using --EOS flag"<<std::endl;
               return -99;
            }
      if (filename.Contains("midasdata/"))
         filename.Remove(0,10);
      TString EOSdir="/eos/experiment/ALPHAg/midasdata_old/";
      EOSdir+=filename;
      //There isn't get a midas folder...
      //TString LocalPath=getenv("MIDASDIR");
      //LocalPath+="/";
      TString LocalPath=filename;
      TString EOScheck="eos ls ";
      EOScheck+=EOSdir;
      EOScheck+="*";

      assert(EOSdir.EndsWith(".mid.lz4"));
      Int_t status=-99;
      if (gSystem->GetFromPipe(EOScheck).Sizeof()!=1 ) //If file exists, 
         {
            std::cout << "EOS::Midas file not found, --EOS enabled, hostname matches compatibility list... fetching file from EOS" << std::endl;  //Don't check the first file... I want an error printed if there is no file
            TString EOScopy="eos cp ";
            EOScopy+=EOSdir;
            EOScopy+=" .";
            //EOScopy+=getenv("MIDASDIR");
            //EOScopy+="/";
            status=gSystem->Exec(EOScopy);
            if (status!=0 )
               {
                  if (AllowedRetry<=0) exit(555);
                  std::cout <<"EOS::Fetching failed with error: "<<status<<std::endl;
                  gSystem->Sleep(5000);
                  std::cout <<"EOS::Trying again ("<<AllowedRetry<<" more attempt(s) until abort"<<std::endl;
                  CopyMidasFileFromEOS(filename,AllowedRetry-1);
                }
            return status;
         }
      else
         {
            std::cout<< std::endl <<filename<< " not found on EOS" << std::endl;
            return -2;
         }
   }

   int CopyMidasFileFromEOS(int runno, int sub, int AllowedRetry=5)
   {
      TString filename=MidasFileName(runno,sub);
      if (CheckMidasFileOnEOS(filename))
         return CopyMidasFileFromEOS(filename,AllowedRetry);
      else
         return 0;
   }

   int CheckLocallyForMidasFile(int runno, int sub)
   {
      TString filename=MidasFileName(runno,sub);
      struct stat buffer;   
         return (stat (filename.Data(), &buffer) == 0); 
   }

   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("EOS::BeginRun, run %d\n", runinfo->fRunNo);
      RunNumber=runinfo->fRunNo;
      subrun=0;
      SendTimeReport=false;
      if (fFlags->fEOS)
         {
            SkipSpecial=true;
            SubRunFetched.push_back(false);
            printf("EOS::Fetching files as we need them... the first file must already be here...");
         }
   }

   void NextSubrun(TARunInfo* runinfo)
   {
      if (fFlags->fEOS)
      {
         //std::cout<<"EOS::NEXT! sub:"<<subrun<<"  "<<SubRunFetched.size()<<"  "<<std::endl;
         if (SubRunFetched.at(subrun))
            {
               std::cout <<"EOS::Last file ("<<MidasFileName(RunNumber,subrun)<<") was fetched from EOS... removing it"<<std::endl;
               TString cmd="rm -v ";
               cmd+=MidasFileName(RunNumber,subrun);
               gSystem->Exec(cmd); 
            }
         subrun++;
         SkipSpecial=true;
      }
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("EOS::EndRun, run %d\n", runinfo->fRunNo);
   }

   void PauseRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("EOS::PauseRun, run %d\n", runinfo->fRunNo);
   }

   void ResumeRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("ResumeModule, run %d\n", runinfo->fRunNo);
   }

   TAFlowEvent* Analyze(TARunInfo* runinfo, TMEvent* me, TAFlags* flags, TAFlowEvent* flow)
   {
      if (SendTimeReport)
         {
            #ifdef _TIME_ANALYSIS_
               if (TimeModules) flow=new AgAnalysisReportFlow(flow,"eos_module");
            #endif
            SendTimeReport=false;
         }
      return flow;
   }

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (fTrace)
         printf("EOS::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n", 
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      if (SkipSpecial)
         {
            SkipSpecial=false;
            return;
         }
      
      if (fFlags->fEOS)
         {
            if (!CheckLocallyForMidasFile(RunNumber,subrun+1))
               {
                  SubRunFetched.push_back(true);
                  CopyMidasFileFromEOS(RunNumber,subrun+1);
                  SendTimeReport=true;
               }
            else
               {
                  SubRunFetched.push_back(false);
                  std::cout <<"EOS::Sub run "<<subrun+1<<" found locally!"<<std::endl;
               }
         }
   }
};

class EOSFactory: public TAFactory
{
public:
   EOSFlags fFlags;

public:
   void Init(const std::vector<std::string> &args)
   {
      printf("EOSFactory::Init!\n");

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--EOS")
            fFlags.fEOS = true;
         if (args[i] == "--offline")
            fFlags.fCustomOutput = true;
         if (args[i] == "--treeout")
         {
            fFlags.fCustomOutput = true;
            i++;
            fFlags.fCustomOutputName = args[i];
         }
      }
      
   }

   void Finish()
   {
      printf("EOSFactory::Finish!\n");
   }

   TARunObject* NewRunObject(TARunInfo* runinfo)
   {
      printf("EOSFactory::NewRunObject, run %d, file %s\n", runinfo->fRunNo, runinfo->fFileName.c_str());
      return new EOS(runinfo, &fFlags);
   }
};

static TARegister tar(new EOSFactory);

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */

