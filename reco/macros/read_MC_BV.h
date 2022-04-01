//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Mar 22 17:14:35 2022 by ROOT version 6.22/06
// from TTree MCinfo/MCinfo
// found on file: ecomug_sky_22.03.2022.root
//////////////////////////////////////////////////////////

#ifndef read_MC_BV_h
#define read_MC_BV_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "TClonesArray.h"
#include "TBSCHit.hh"
#include "Riostream.h"


class read_MC_BV {
public :
   TTree          *tMCinfo;   //!pointer to the analyzed TTree or TChain
   TTree          *tBVBars;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain
   TString         filename;

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   TClonesArray    *MCvertex;
   TClonesArray    *MCpions;
   TClonesArray    *ScintBarDigiMCTruth;


   // List of branches
   TBranch        *b_MCvertex;   //!
   TBranch        *b_MCpions;   //!
   TBranch        *b_ScintBarDigiMCTruth;   //!

   read_MC_BV(string file_name);
   virtual ~read_MC_BV();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadMCInfoTree(Long64_t entry);
   virtual Long64_t LoadBVBarsTree(Long64_t entry);
   virtual void     InitMCinfoTree(TTree *tree);
   virtual void     InitBVBarsTree(TTree *tree);
   virtual void     AnalyzeMCinfo();
   virtual void     AnalyzeBVBars(Float_t EnergyCut=-999.0, Float_t DeltaPhiCut = -999.0, Int_t MultCut = 999, Float_t smearingTime = -999.0, Float_t v_reluncertainty = -999.0);
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef read_MC_BV_cxx

read_MC_BV::read_MC_BV(string file_name) : tMCinfo(0), tBVBars(0) 
{
// used to generate this class and read the Tree.
   filename=file_name;
   Int_t dot = filename.Last('.');
   Int_t len = filename.Length();
   filename.Remove(dot,len-dot);

   TString fname =  "simulation/"+filename+".root";
   
   TTree *treeMCinfo = 0;
   TTree *treeBVBars = 0;
   TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(fname);
   if (!f || !f->IsOpen()) {
      f = new TFile(fname);
   }
   f->GetObject("MCinfo",treeMCinfo);
   f->GetObject("ScintBarsMCdata",treeBVBars);
   InitMCinfoTree(treeMCinfo);
   InitBVBarsTree(treeBVBars);
}

read_MC_BV::~read_MC_BV()
{
   if (!tMCinfo) return;
   delete tMCinfo->GetCurrentFile();
}

Int_t read_MC_BV::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!tMCinfo) return 0;
   return tMCinfo->GetEntry(entry);
}

Long64_t read_MC_BV::LoadMCInfoTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!tMCinfo) return -5;
   Long64_t centry = tMCinfo->LoadTree(entry);
   if (centry < 0) return centry;
   if (tMCinfo->GetTreeNumber() != fCurrent) {
      fCurrent = tMCinfo->GetTreeNumber();
      Notify();
   }
   return centry;
}

Long64_t read_MC_BV::LoadBVBarsTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!tBVBars) return -5;
   Long64_t centry = tBVBars->LoadTree(entry);
   if (centry < 0) return centry;
   if (tBVBars->GetTreeNumber() != fCurrent) {
      fCurrent = tBVBars->GetTreeNumber();
      Notify();
   }
   return centry;
}

void read_MC_BV::InitMCinfoTree(TTree *tree)
{
   // Set object pointer
   MCvertex = 0;
   MCpions = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   tMCinfo = tree;
   fCurrent = -1;
   tMCinfo->SetMakeClass(1);

   tMCinfo->SetBranchAddress("MCvertex", &MCvertex, &b_MCvertex);
   tMCinfo->SetBranchAddress("MCpions", &MCpions, &b_MCpions);
   Notify();
}

void read_MC_BV::InitBVBarsTree(TTree *tree)
{
   // Set object pointer
   ScintBarDigiMCTruth = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   tBVBars = tree;
   fCurrent = -1;
   tBVBars->SetMakeClass(1);

   tBVBars->SetBranchAddress("ScintBarDigiMCTruth", &ScintBarDigiMCTruth, &b_ScintBarDigiMCTruth);
   Notify();
}

Bool_t read_MC_BV::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void read_MC_BV::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!tMCinfo) return;
   tMCinfo->Show(entry);
}
Int_t read_MC_BV::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef read_MC_BV_cxx
