#include "FileWriters.h"


std::string WriteTStoreGEMFile(TStoreGEMFile* file)
{
    const std::vector<char> file_data = file->GetData();
    int size = file_data.size();
    std::string filename, filepath, fileMD5;
    char* FILE = nullptr;
    const char* d = file_data.data();
    int i;
    //The data contains Filename NULL FilePath NULL MD5 NULL FILEDATA
    for (i =0; i <size; i++ )
    {
        if (strncmp(d + i,"Filename:",9)==0)
        {
            i += 9;
            filename=(char*)d + i;
        }
        else if (strncmp(d + i,"FilePath:",9)==0)
        {
            i += 9;
            filepath=(char*)d + i;
        }
        else if (strncmp(d + i,"MD5:",4)==0)
        {
            i += 4;
            fileMD5=(char*)d + i;
        }
        else if (strncmp(d + i,"FILE:",5)==0)
        {
            i += 5;
            FILE=(char*)d + i;
        }
        //Do we have all args
        if (filename.size() && filepath.size() && fileMD5.size() && FILE)
            break;
        while (file_data[i] != 0)
            i++;
    }
    assert (filename == file->GetFileName());
    assert (filepath == file->GetFilePath());
    assert (fileMD5 == file->GetFileMD5());

    std::fstream bin (filename.c_str(), std::ios::out | std::ios::binary);
    bin.write(reinterpret_cast<char *> (FILE),size - i );
    bin.close();

    return filename;
}

std::vector<std::string> DumpFilesSavedInMIDAS(Int_t runNumber, const char* category, const char* varname)
{
   std::string filename = std::string("FILE:") + category + "\\" + varname;
   std::vector<std::string> files_written;
   std::vector<TTreeReader*> trees = Get_feGEM_File_Trees(runNumber,filename);
   for (size_t i=0; i<trees.size(); i++)
   {
      TTreeReader* reader = trees.at(i);
      TTreeReaderValue<TStoreGEMFile> file(*reader, "TStoreGEMFile<char>");
      while (reader->Next())
      {
         (*file).PrintFileInfo();
         files_written.push_back(WriteTStoreGEMFile(&(*file)));
      }
   }
   return files_written;
}

#if BUILD_A2
void DumpSpillLogsToCSV(std::vector<TA2Spill> dumps, std::string filename)
{
    
   std::ofstream spillLog;
   spillLog.open(filename);
   
   int runNumber = 0;
   TSISChannels * sis_chans = new TSISChannels();

   for (size_t i=0; i< dumps.size(); i++)
   {
       if (dumps.at(i).RunNumber != runNumber)
       {
           //New run number... the SIS channels can change name, insert a new title line
           runNumber = dumps.at(i).RunNumber;
           std::vector<std::string> channelNames;
           for (int channel=0; channel < NUM_SIS_MODULES * NUM_SIS_CHANNELS; channel++ )
              channelNames.push_back(sis_chans->GetDescription( channel,runNumber ).Data());
           spillLog << dumps.at(i).ContentCSVTitle(channelNames) << "\n";
       }
       spillLog << dumps.at(i).ContentCSV() << "\n";
   }
   spillLog.close();
   std::cout<< filename << " saved\n";
}

void DumpA2SpillLogToCSV(int runNumber)
{
    std::string filename = "R" + std::to_string(runNumber) + ".spilllog.csv";
    std::vector<TA2Spill> spills = Get_All_A2_Spills(runNumber);
    return DumpSpillLogsToCSV(spills,filename);
}
#endif


//Static function, do not reveal this function to user
template <typename T>
static void WriteFEGEMData(const std::string filename, TTreeReader* gemReader, const char* name, double firstTime, double lastTime)
{
   std::ofstream gem_data;
   gem_data.open(filename);
   
   gem_data << "Data type:," << name << ",Time cut from," << firstTime << ",to," << lastTime << "\n";
   gem_data << "RunNumber,Midas RunTime, LabVIEW timestamp, data...\n";
   gem_data << std::setprecision(17);
   TTreeReaderValue<TStoreGEMData<T>> gemEvent(*gemReader, name);
   // I assume that file IO is the slowest part of this function... 
   // so get multiple channels and multiple time windows in one pass
   while (gemReader->Next())
   {
      double runTime = gemEvent->GetRunTime();
      //A rough cut on the time window is very fast...
      if (runTime < firstTime)
         continue;
      if (runTime > lastTime)
         break;

      gem_data << gemEvent->GetRunNumber() << ",";
      gem_data << gemEvent->GetRunTime() << ",";
      gem_data << gemEvent->GetLVTimestamp() << ",";

      const std::vector<T> data = gemEvent->GetData();
      for (const T& d: data)
         gem_data << d << ",";
      gem_data <<"\n";
   }
   
   gem_data.close();
   std::cout<< filename << " saved\n";
   return;
}


void DumpfeGEMDataToCSV(const int runNumber, const std::string category, const std::string varname,  const double firstTime , const double lastTime )
{
   std::string filename  = category + std::string("_") + varname + std::string(".csv");
   
   TTreeReader* feGEMReader = Get_feGEM_Tree(runNumber, category, varname);
   TTree* tree = feGEMReader->GetTree();
   if  (!tree)
   {
      std::cout<<"Warning: " << category << " ("<<varname<<") not found for run " << runNumber << std::endl;
      return;
   }
   if (tree->GetBranchStatus("TStoreGEMData<double>"))
      WriteFEGEMData<double>(filename, feGEMReader, "TStoreGEMData<double>", firstTime, lastTime);
   else if (tree->GetBranchStatus("TStoreGEMData<float>"))
      WriteFEGEMData<float>(filename, feGEMReader, "TStoreGEMData<float>", firstTime, lastTime);
   else if (tree->GetBranchStatus("TStoreGEMData<bool>"))
      WriteFEGEMData<bool>( filename,feGEMReader, "TStoreGEMData<bool>", firstTime, lastTime);
   else if (tree->GetBranchStatus("TStoreGEMData<int32_t>"))
      WriteFEGEMData<int32_t>( filename, feGEMReader, "TStoreGEMData<int32_t>", firstTime, lastTime);
   else if (tree->GetBranchStatus("TStoreGEMData<uint32_t>"))
      WriteFEGEMData<uint32_t>( filename, feGEMReader, "TStoreGEMData<uint32_t>", firstTime, lastTime);
   else if (tree->GetBranchStatus("TStoreGEMData<uint16_t>"))
      WriteFEGEMData<uint16_t>( filename, feGEMReader, "TStoreGEMData<uint16_t>", firstTime, lastTime);
   else if (tree->GetBranchStatus("TStoreGEMData<char>"))
      WriteFEGEMData<char>( filename, feGEMReader, "TStoreGEMData<char>", firstTime, lastTime);
   else
      std::cout << "Warning unable to find TStoreGEMData type" << std::endl;   
}
