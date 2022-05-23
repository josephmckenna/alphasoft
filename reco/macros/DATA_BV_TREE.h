//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Sun Mar 13 15:42:07 2022 by ROOT version 6.24/06
// from TTree StoreEventTree/StoreEventTree
// found on file: output06201.root
//////////////////////////////////////////////////////////

#ifndef DATA_BV_TREE_h
#define DATA_BV_TREE_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "TBarEvent.hh"
#include "TBarHit.h"
#include "TBarEndHit.h"


class DATA_BV_TREE {
public :
  TTree          *fChain;   //!pointer to the analyzed TTree or TChain
  Int_t           fCurrent; //!current Tree number in a TChain
  TFile *fOut;  ///< File for the output
  // Fixed size dimensions of array or collections stored in the TTree if any.

  // Declaration of leaf types
  TBarEvent    *BarEvent;

  // List of branches
  TBranch        *b_BarEvent;   //!

  DATA_BV_TREE(std::string, Double_t);
  void CreateOutputTree();
  void ClearTreeVariables();
  void FillVariables();
  void MeanSigma(std::vector<Double_t> , Double_t& , Double_t& , Double_t& , Double_t&);

  virtual ~DATA_BV_TREE();
  virtual Int_t    Cut(Long64_t entry);
  virtual Int_t    GetEntry(Long64_t entry);
  virtual Long64_t LoadTree(Long64_t entry);
  virtual void     Init(TTree *tree);
  virtual void     Loop();
  virtual Bool_t   Notify();
  virtual void     Show(Long64_t entry = -1);
  


  //********** Tree output variables *********************
  TTree *treeDataBV;
  
  Double_t  t_event;
    ///< t_pbars is the time (in s) in which there are pbars in the ALPHA-g traps
    ///< if t_pbars = std::numeric_limits<double>::max() => cosmics run
  Double_t  t_pbars;
  Int_t     event;
  Bool_t    pbar ;
  Bool_t    mc   ;
  Int_t     nDigi;
  Int_t     nBars;

  std::vector<Int_t>    BarNumber ;
  std::vector<Int_t>    BarNTracks;
  std::vector<Double_t>  Energy    ;  
  std::vector<Double_t>  Path      ;  
  std::vector<Double_t>  Zeta      ;  
  std::vector<Double_t> Time      ;  
  std::vector<Double_t>  Phi       ;  
  std::vector<Double_t>  ATop      ;  
  std::vector<Double_t>  ABot      ;  
  std::vector<Double_t>  tTop      ;  
  std::vector<Double_t>  tBot      ;    

  
  std::vector<Double_t> TOFs  ;
  std::vector<Double_t> DPHIs ;
  std::vector<Double_t> DZETAs;
  std::vector<Double_t> DISTs ;

  Double_t TOF_MIN   ;
  Double_t TOF_MAX   ;
  Double_t TOF_MEAN  ;
  Double_t TOF_STD   ;
  Double_t DPHI_MIN  ;
  Double_t DPHI_MAX  ;
  Double_t DPHI_MEAN ;
  Double_t DPHI_STD  ;
  Double_t DZETA_MIN ;
  Double_t DZETA_MAX ;
  Double_t DZETA_MEAN;
  Double_t DZETA_STD ;
  Double_t DIST_MIN  ;
  Double_t DIST_MAX  ;
  Double_t DIST_MEAN ;
  Double_t DIST_STD  ;
  
};

#endif

#ifdef DATA_BV_TREE_cxx
DATA_BV_TREE::DATA_BV_TREE(std::string filename, Double_t t_switch) : fChain(0) 
{
  t_pbars = t_switch; 
  // if parameter tree is not specified (or zero), connect the file
  // used to generate this class and read the Tree.
  TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(filename.c_str());
  if (!f || !f->IsOpen()) {
    f = new TFile(filename.c_str());
  } 
  if(!f) {
    std::cout << "File " << filename.c_str() << " not found " << std::endl;
  }
  TTree *tree;
  f->GetObject("TBarEventTree",tree);
  Init(tree);

  ///< Removing directory from file name (to be used later to open output file)
  Int_t slash = filename.find_first_of('/');
  std::string sf = filename.substr(slash+1,filename.length()-slash); 

  std::string sout;
  if(t_pbars<std::numeric_limits<double>::max()) { ///< There is a time in which we have pbars
    sout = "root_output_files/DATA_BV_TREE_pbars_run_"+sf;
  } else { ///< No pbars => cosmics run
    sout = "root_output_files/DATA_BV_TREE_cosmics_run_"+sf;  
  }
  fOut = new TFile(sout.c_str(),"RECREATE");
}

DATA_BV_TREE::~DATA_BV_TREE()
{
  if (!fChain) return;
  delete fChain->GetCurrentFile();
}

Int_t DATA_BV_TREE::GetEntry(Long64_t entry)
{
  // Read contents of entry.
  if (!fChain) return 0;
  return fChain->GetEntry(entry);
}
Long64_t DATA_BV_TREE::LoadTree(Long64_t entry)
{
  // Set the environment to read one entry
  if (!fChain) return -5;
  Long64_t centry = fChain->LoadTree(entry);
  if (centry < 0) return centry;
  if (fChain->GetTreeNumber() != fCurrent) {
    fCurrent = fChain->GetTreeNumber();
    Notify();
  }
  return centry;
}

void DATA_BV_TREE::Init(TTree *tree)
{
  // The Init() function is called when the selector needs to initialize
  // a new tree or chain. Typically here the branch addresses and branch
  // pointers of the tree will be set.
  // It is normally not necessary to make changes to the generated
  // code, but the routine can be extended by the user if needed.
  // Init() will be called many times when running on PROOF
  // (once per file to be processed).

  // Set object pointer
  BarEvent = 0;
  // Set branch addresses and branch pointers
  if (!tree) return;
  fChain = tree;
  fCurrent = -1;
  fChain->SetMakeClass(1);

  fChain->SetBranchAddress("TBarEvent", &BarEvent, &b_BarEvent);
  Notify();
}

Bool_t DATA_BV_TREE::Notify()
{
  // The Notify() function is called when a new file is opened. This
  // can be either for a new TTree in a TChain or when when a new TTree
  // is started when using PROOF. It is normally not necessary to make changes
  // to the generated code, but the routine can be extended by the
  // user if needed. The return value is currently not used.

  return kTRUE;
}

void DATA_BV_TREE::Show(Long64_t entry)
{
  // Print contents of entry.
  // If entry is not specified, print current entry
  if (!fChain) return;
  fChain->Show(entry);
}
Int_t DATA_BV_TREE::Cut(Long64_t entry)
{
  // This function may be called from Loop.
  // returns  1 if entry is accepted.
  // returns -1 otherwise.
  return 1;
}
#endif // #ifdef DATA_BV_TREE_cxx
