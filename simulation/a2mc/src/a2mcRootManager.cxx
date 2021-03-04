///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "a2mcRootManager.h"

ClassImp(a2mcRootManager)

a2mcRootManager* a2mcRootManager::fgInstance = 0;

//_____________________________________________________________________________
a2mcRootManager::a2mcRootManager(Int_t run_num, const std::string run_time, const char* projectName, FileMode fileMode)
  : TObject()
{
/// Standard constructor
/// \param projectName  The project name (passed as the Root tree name)
/// \param fileMode     Option for opening Root file (read or write mode)

  if (fgInstance) {
    Fatal("a2mcRootManager", "Singleton instance already exists.");
    return;
  }  

  TString fileName("root/");
  fileName += projectName;
  fileName += "-";
  fileName += run_time;
  fileName += "_";
  fileName += run_num;
  fileName += ".root";
  
  TString treeTitle(projectName);
  treeTitle += " tree";
  
  if (fileMode == kRead) {
    fFile = new TFile(fileName);
    fTree = (TTree*) fFile->Get(projectName);
  } else {
      if(fileExist(projectName, run_num)) exit(0);
      fFile = new TFile(fileName, "recreate");
      fTree = new TTree(projectName, treeTitle);
  }
  
  fPath = gDirectory->GetPath();
  fgInstance = this;
}

//_____________________________________________________________________________
a2mcRootManager::a2mcRootManager()
  : TObject(),
    fFile(0),
    fTree(0) 
{
/// Default constructor

  if (fgInstance) {
    Fatal("a2mcRootManager", "Singleton instance already exists.");
    return;
  }  

  fgInstance = this;
}

//_____________________________________________________________________________
a2mcRootManager::~a2mcRootManager() 
{
/// Destructor

  delete  fTree->GetCurrentFile();
  fgInstance = 0;
}

//
// static methods
//

//_____________________________________________________________________________
a2mcRootManager* a2mcRootManager::Instance()
{
/// \return The singleton instance.

  return fgInstance;
}  

//
// public methods
//

//_____________________________________________________________________________
void  a2mcRootManager::Register(const char* name, const char* className, 
                                void* objAddress)
{
/// Create a branch and associates it with the given address.
/// \param name       The branch name
/// \param className  The class name of the object
/// \param objAddress The object address

  if (!fTree->GetBranch(name)) 
    fTree->Branch(name, className, objAddress, 32000, 99);
  else  
    fTree->GetBranch(name)->SetAddress(objAddress);
}

//_____________________________________________________________________________
void  a2mcRootManager::Fill()
{
/// Fill the Root tree.

  fTree->Fill();
}  

//_____________________________________________________________________________
void a2mcRootManager:: WriteAll()
{
/// Write the Root tree in the file.

  TFile* file =  fTree->GetCurrentFile();
  file->Write();
  file->Close();
}  

//_____________________________________________________________________________
void  a2mcRootManager::ReadEvent(Int_t i)
{
/// Read the event data for \em i -th event for all connected branches.
/// \param i  The event to be read

  fTree->GetEntry(i);
}

//_____________________________________________________________________________
bool a2mcRootManager::fileExist(const std::string& projectName, int run) {
    bool exist = false;
    TSystemDirectory dir("root", "root");
    TList *files = dir.GetListOfFiles();
    if (files)
    {
        TSystemFile *file;
        std::string fname;
        TIter next(files);
        while ((file=(TSystemFile*)next()))
        {
            fname = file->GetName();
            size_t find1 = fname.find(projectName.c_str());
            std::ostringstream s;
            s << "_" << run << ".root";
            size_t find2 = fname.find(s.str().c_str());
            if(find1!=std::string::npos&&find2!=std::string::npos)
            {
                std::cout << "a2mcRootManager::fileExist --> Run number " << run
                          << " is already present (root/" << fname << ")" << std::endl; 
                exist = true;
                break;
            }
        }
    }
    return exist;
}
