#include "FileWriters.h"


void WriteTStoreGEMFile(TStoreGEMFile* file)
{
    const std::vector<char> file_data = file->GetData();
    size_t size = file_data.size();
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
   for (int i=0; i<trees.size(); i++)
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
