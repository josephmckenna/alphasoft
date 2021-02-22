#ifndef a2mc_ROOT_MANAGER_H
#define a2mc_ROOT_MANAGER_H

#include <iostream>
#include <sstream>

#include <TObject.h>
#include <TTree.h>
#include "TSystemDirectory.h"
#include "TFile.h"

/// Root file mode
enum FileMode { 
  kRead,  ///< Read mode 
  kWrite  ///< Write mode
};

/// \ingroup E02
/// \brief Class that takes care of Root IO
///
/// Geant4 novice ExampleN02 adapted to Virtual Monte Carlo 
///
/// \date 05/04/2002
/// \author I. Hrivnacova; IPN, Orsay

class a2mcRootManager : public TObject
{
 public:
  a2mcRootManager(Int_t RunNumber, const std::string RunTime, const char* projectName, FileMode fileMode);
  a2mcRootManager();
  virtual ~a2mcRootManager();     
  
  // static access method
  static a2mcRootManager* Instance(); 
  
  // methods
  void  Register(const char* name, const char* className, void* objAddress);
  void  Fill();
  void  WriteAll();
  void  ReadEvent(Int_t i);
  
 private:
  // data members
  bool fileExist(const std::string&, int);
  static  a2mcRootManager* fgInstance; ///< Singleton instance
  
  // data members
  TFile*  fFile; ///< Root output file
  TTree*  fTree; ///< Root output tree 
  TString fPath; ///< The path to the root file
  
  ClassDef(a2mcRootManager,0) // Root IO manager
    };

#endif //a2mc_ROOT_MANAGER_H   


