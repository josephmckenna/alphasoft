//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Feb 17 10:45:30 2022 by ROOT version 6.24/06
// from TTree ScintBarsMCdata/ScintBarsMCdata
// found on file: outAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30.root
//////////////////////////////////////////////////////////

#ifndef ReadScintBarHits_h
#define ReadScintBarHits_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>


// Header file for the classes stored in the TTree if any.
#include "TClonesArray.h"
#include "TScintDigi.hh"
#include "Riostream.h"

class ReadScintBarHits {
public :
   Bool_t          Bcosmic=1; 
   TString         filename="outAgTPC_det_AWtime16ns_PADtime16ns_B1.00T_Q30";
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   TClonesArray    *ScintBarHits;

   // List of branches
   TBranch        *b_ScintBarHits;   //!

   ReadScintBarHits(TTree *tree=0);
   virtual ~ReadScintBarHits();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop(Float_t EnergyCut=-999.0, Float_t DeltaPhiCut = -999.0, Int_t MultCut = 999, Float_t smearingTime = -999.0, Int_t   PDGcode = -999);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef ReadScintBarHits_cxx
ReadScintBarHits::ReadScintBarHits(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      if(Bcosmic) filename+="_cosmics_01";
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(filename+".root");
      if (!f || !f->IsOpen()) {
         f = new TFile(filename+".root");
      }
      f->GetObject("ScintBarsMCdata",tree);

   }
   Init(tree);
}

ReadScintBarHits::~ReadScintBarHits()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t ReadScintBarHits::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t ReadScintBarHits::LoadTree(Long64_t entry)
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

void ReadScintBarHits::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   ScintBarHits = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("ScintBarHits", &ScintBarHits, &b_ScintBarHits);
   Notify();
}

Bool_t ReadScintBarHits::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void ReadScintBarHits::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t ReadScintBarHits::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef ReadScintBarHits_cxx
