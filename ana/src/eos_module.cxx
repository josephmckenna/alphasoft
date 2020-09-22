//
// Module to handle fetching files from EOS
// JTK McKENNA
//




#include "manalyzer.h"
#include "midasio.h"
#include "AgFlow.h"
#include "RecoFlow.h"

#include <iostream>
#include <cassert>
#include "AnalysisTimer.h"
#include "TSystem.h"
#include <sys/stat.h>


enum EXPERIMENT    { EXP_ALPHA2, EXP_ALPHAg };
enum FILE_LOCATION { LOCAL, REMOTE, NOT_FOUND };


class EOSFlags
{
   public:
   bool fPrint = false;
   bool fEOS = false;
   bool fCustomOutput = false;
   TString fCustomOutputName = "auto";

   std::vector<int> FileLocation;

   TString LocalPath="";
   TString EOSDIR="";
   TString FileEnding="";
   int EXPERIMENT_NO=-1;
   TObjArray* RemoteFileList;
   TObjArray* LocalFileList;
   
   private:
   
   TString GetFileList(const char* filename, const char* dir)
   {
      assert(strcmp(filename,"")!=0);
      TString cmd;
      //Check if eos is installed
      if (system("which eos > /dev/null 2>&1"))
      {
         std::cout<<"eos bin not found, attempting to use fuse mount"<<std::endl;
         cmd="";
      }
      else
      {
         cmd="eos ";
      }

      char buf[200];
      if (strcmp(dir,EOSDIR.Data())==0)
         sprintf(buf,"%s ls '%s/%.11s*'",cmd.Data(),dir,filename);
      else
         sprintf(buf,"ls %s/%.11s*",dir,filename);
      std::cout<<"cmd:"<<buf<<std::endl;
      TString FileList=gSystem->GetFromPipe(buf);
      std::cout<<"File list:"<<FileList<<std::endl;
      return FileList;
   }
   void SetRemoteFileArray(const char* filename)
   {
      RemoteFileList = GetFileList(filename,EOSDIR.Data()).Tokenize('\n');
   }
   void SetLocalFileArray(const char* filename)
   {
      LocalFileList = GetFileList(filename,LocalPath.Data()).Tokenize('\n');
   }
   public:
   int CopyMidasFileFromEOS(TString filename, Int_t AllowedRetry=5);
   void Setup(int exp, const char* firstFile)
   {
      EXPERIMENT_NO=exp;
      if (exp==EXP_ALPHA2)
      {
         EOSDIR="/eos/experiment/alpha/midasdata/";
         FileEnding=".mid.gz";
         std::cout<<"Seting up paths for ALPHA2:"<<std::endl;
      }
      else if (exp==EXP_ALPHAg)
      {
         EOSDIR="/eos/experiment/ALPHAg/midasdata_old/";
         FileEnding=".mid.lz4";
         std::cout<<"Seting up paths for ALPHAg:"<<std::endl;
      }
      std::cout <<"EOS path: \t"<<EOSDIR<<std::endl;
      std::cout<<"FileEnding:\t"<<FileEnding<<std::endl;
      if (LocalPath.Sizeof()==1)
         LocalPath=".";
      std::cout<<"LocalStorage:\t"<<LocalPath<<std::endl;
      SetLocalFileArray(firstFile);
      SetRemoteFileArray(firstFile);
   }

   bool CheckFileInList(const char* filename)
   {
      if (!filename) return false;
      for (size_t i=0; i<TARunInfo::fgFileList.size(); i++)
      {
         TString listitem=TARunInfo::fgFileList.at(i);
         if (listitem.EndsWith(filename))
         {
            std::cout <<"File: "<<filename<<" already in list... (as "<<listitem<<")"<<std::endl;
            return true;
         }
      }
      //File not in list
      return false;
   }
   FILE_LOCATION GetFileLocation(const char* filename)
   {
      int nLocalFiles=LocalFileList->GetEntriesFast();
      for (int i =0; i<nLocalFiles; i++)
      {
         TString local(LocalFileList->At(i)->GetName());
         //If file is local
         if (local.EndsWith(filename))
         {
            std::cout <<filename<<" found locally "<<std::endl;
            return LOCAL;
         }
      }

      int nRemoteFiles=RemoteFileList->GetEntriesFast();
      for (int i =0; i<nRemoteFiles; i++)
      {
         const char* remote=RemoteFileList->At(i)->GetName();
         //If file is local
         if (strcmp(remote,filename) == 0)
         {
            std::cout <<filename<<" found remotely "<<std::endl;
            return REMOTE;
         }
      }
      return NOT_FOUND;
   }
   //Queue a file in TARunInfo, work out if its local or on EOS
   FILE_LOCATION AddFile(const char* filename, uint position)
   {
      //Check this file isn't already in the list of files
      

      //Setup full location of midas file
      TString FullPath="";
      if (strcmp(LocalPath,".")!=0)
         FullPath+=LocalPath;
      FullPath+=filename;

      FILE_LOCATION location=GetFileLocation(filename);
      //Make sure the file list is large enough for our entry
      if (location!=NOT_FOUND)
      {
         if (TARunInfo::fgFileList.size()<position+1)
         TARunInfo::fgFileList.resize(position+1);
      }
      if (location==LOCAL)
      {
         std::cout <<FullPath.Data()<<" found locally "<<std::endl;
         FileLocation.at(position)=LOCAL;
         TARunInfo::fgFileList.at(position)=FullPath.Data();
         return LOCAL;
      }
      else if (location==REMOTE)
      {
         std::cout <<FullPath.Data()<<" found remotely "<<std::endl;
         FileLocation.at(position)=REMOTE;
         TARunInfo::fgFileList.at(position)=FullPath.Data();
         return REMOTE;
      }
      FileLocation.at(position)=(NOT_FOUND);
      return NOT_FOUND;
   }
   
   TString MidasFileName(int runno, int sub)
   {
      char name[50];
      if (EXPERIMENT_NO==EXP_ALPHA2)
      {
         sprintf(name,"run%05dsub%05d.mid.gz",runno,sub);
      }
      else if (EXPERIMENT_NO==EXP_ALPHAg)
      {
         sprintf(name,"run%05dsub%03d.mid.lz4",runno,sub);
      }
      TString filename(name);
      return filename;
   }
   void FillFileList(TARunInfo* runinfo)
   {
      int nLocalFiles=LocalFileList->GetEntriesFast();
      int nRemoteFiles=RemoteFileList->GetEntriesFast();
      int listSize=nLocalFiles+nRemoteFiles;
      //Set vector with file location data (they aren't found yet)
      FileLocation.resize(listSize);
      for (int sub=1; sub<listSize; sub++)
         FileLocation.at(sub)=NOT_FOUND;

      for (int sub=1; sub<listSize; sub++)
      {
         FileLocation.at(sub)=NOT_FOUND;
         TString file=MidasFileName(runinfo->fRunNo,sub);
         if (AddFile(file.Data(),sub) == NOT_FOUND)
            break;
      }
   }
   
   //int CopyMidasFileFromEOS(int runno, int sub, int AllowedRetry=5);
};

int EOSFlags::CopyMidasFileFromEOS(TString filename, Int_t AllowedRetry)
{
   TString cmd;
   //Check if eos is installed
   if (system("which eos > /dev/null 2>&1"))
   {
      std::cout<<"eos bin not found, attempting to use fuse mount"<<std::endl;
      cmd="";
   }
   else
   {
      cmd="eos ";
   }

   TString EOScheck=cmd;
   EOScheck+="ls ";
   EOScheck+=EOSDIR;
   EOScheck+=filename;
   //EOScheck+="*";

   assert(filename.EndsWith(FileEnding.Data()));
   Int_t status=-99;
   if (gSystem->GetFromPipe(EOScheck).Sizeof()!=1 ) //If file exists,
   {

      std::cout << "EOS::Midas file not found, --EOS enabled, hostname matches compatibility list... fetching file from EOS" << std::endl;  //Don't check the first file... I want an error printed if there is no file
      TString EOScopy=cmd;
      EOScopy+="cp ";
      EOScopy+=EOSDIR;
      EOScopy+=filename;
      EOScopy+=" ";
      EOScopy+=LocalPath;
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


std::mutex FileCopying;
void CopyMidasFileAsThread(EOSFlags* fFlags, int RunNo, int CurrentIndex)
{
   std::lock_guard<std::mutex> lock(FileCopying);
   std::cout <<"I think this is the start of a run file... Go get the next one... delete the last one"<<std::endl;
   int LastFileIndex=CurrentIndex-1;
   if (LastFileIndex>=0)
   {
      if (fFlags->FileLocation.at(LastFileIndex)==REMOTE)
      {
         TString Delete="rm ";
         Delete+=TARunInfo::fgFileList.at(LastFileIndex);
         std::cout <<"Delete command:"<< Delete <<std::endl;
         //int status=gSystem->Exec(Delete);
         gSystem->Exec(Delete);
      }
   }
   int NextFileIndex=TARunInfo::fgCurrentFileIndex+1;
   //Check if next file isn't beyond the end of the list:
   if (NextFileIndex<(int)fFlags->FileLocation.size())
      //Check if the file should be fetched from remote source
      if (fFlags->FileLocation.at(NextFileIndex)==REMOTE)
         //Fetch the midas file
         fFlags->CopyMidasFileFromEOS(fFlags->MidasFileName(RunNo,NextFileIndex));
   return;
}



class EOS: public TARunObject
{
private:
   bool SkipSpecial=false;

public:
   EOSFlags* fFlags;
   bool fTrace = true;

   
   EOS(TARunInfo* runinfo, EOSFlags* flags)
      : TARunObject(runinfo), fFlags(flags)
   {
      if (!fFlags->fEOS) return;
      
      if (fTrace)
         printf("EOS::ctor!\n");
      //TString n="/Experiment/Name";
      //const char* exp=runinfo->fOdb->odbReadString(n.Data(),0,0);
      fFlags->FillFileList(runinfo);
   }

   ~EOS()
   {
      if (fTrace)
         printf("EOS::dtor!\n");
      if (fFlags->fEOS)
         {
            std::cout<<"EOS::dtor Cleanup"<<std::endl;
         }
   }


   void BeginRun(TARunInfo* runinfo)
   {
      if (fTrace)
         printf("EOS::BeginRun, run %d\n", runinfo->fRunNo);
   }

   void EndRun(TARunInfo* runinfo)
   {
      if (!fFlags->fEOS) return;
      if (fTrace)
         printf("EOS::EndRun, run %d\n", runinfo->fRunNo);
      //Delete final subrun
      if (fFlags->FileLocation.back()==REMOTE)
      {
         TString Delete="rm ";
         Delete+=TARunInfo::fgFileList.back();
         std::cout <<"Delete command:"<< Delete <<std::endl;
         //int status=gSystem->Exec(Delete);
         gSystem->Exec(Delete);
      }
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

   void AnalyzeSpecialEvent(TARunInfo* runinfo, TMEvent* event)
   {
      if (!fFlags->fEOS) return;
      //if (fTrace)
         printf("EOS::AnalyzeSpecialEvent, run %d, event serno %d, id 0x%04x, data size %d\n",
                runinfo->fRunNo, event->serial_number, (int)event->event_id, event->data_size);
      if (event->event_id == 0x8000)
      {
         std::lock_guard<std::mutex> lock(FileCopying);
         int CurrentIndex=TARunInfo::fgCurrentFileIndex;
         {
            new std::thread(CopyMidasFileAsThread,fFlags, runinfo->fRunNo , CurrentIndex);
         }
      }
      if (SkipSpecial)
         {
            SkipSpecial=false;
            return;
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


      //gSystem->Exec(copy);
      
      //CopyMidasFileFromEOS(TARunInfo::fgFileList(TARunInfo::fgCurrentFileIndex), Int_t AllowedRetry=5)

      for (unsigned i=0; i<args.size(); i++) {
         if (args[i] == "--EOS")
            {
               fFlags.fEOS = true;
               
               TString firstfile=TARunInfo::fgFileList.at(TARunInfo::fgCurrentFileIndex).c_str();
               //Work out the local path the midas file is pointed... copy new files here
               fFlags.LocalPath=firstfile(0,firstfile.Last('/')+1); 
               TString filename=firstfile(firstfile.Last('/')+1,firstfile.Sizeof());
               if (firstfile.EndsWith(".mid.gz"))
                  fFlags.Setup(EXP_ALPHA2,filename.Data());
               else if (firstfile.EndsWith(".mid.lz4"))
                  fFlags.Setup(EXP_ALPHAg,filename.Data());
               
               fFlags.FileLocation.push_back(fFlags.GetFileLocation(filename.Data()));
               if (fFlags.FileLocation.at(0)==REMOTE)
                  fFlags.CopyMidasFileFromEOS(filename.Data());
               if(
                  //((strncmp(gSystem->HostName(),"alphacpc",8)==0) && (strcmp(gSystem->GetUserInfo()->fUser,"agana") != 0)) || //AND I am NOT an alphacpc* machine (a safety system to stop deletion of files) (user agana is allowed to run)
                   (strncmp(gSystem->HostName(),"alphagdaq",8)==0) || //AND I am NOT an alphagdaq* machine (a safety system to stop deletion of files)
                   (strncmp(gSystem->HostName(),"alphadaq",8)==0) ) //AND I am NOT an alphadaq* machine (a safety system to stop deletion of files)
                  {
                     std::cerr <<"EOS::This machine is blacklisted from using --EOS flag"<<std::endl;
                     exit(1);
                  }
               if( gSystem->Exec("which eos") != 0 )
                  {
                     std::cerr <<"EOS::eos command not found in path! I will try to use fuse mount"<<std::endl;
                     //exit(1);
                  }
            }
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
      if (fFlags.fPrint)
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
