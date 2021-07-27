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