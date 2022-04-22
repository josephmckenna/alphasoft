#ifndef MC_BV_TREE_h
#define MC_BV_TREE_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "TClonesArray.h"
#include "TBSCHit.hh"
#include "Riostream.h"


class MC_BV_TREE {
public :
   TTree          *tBVBars;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain
   TString         filename;

// Fixed size dimensions of array or collections stored in the TTree if any.
   ///< Output trees
   TFile *fileOUT;
   TTree *treeMCBV;
   Int_t event; // event number
   Bool_t pbar; // pbar annihilation event?
   Bool_t mc;   // mc?
   Int_t nDigi; // number of MC hits/digi (one digi/hit = one track)
   Int_t nBars; // number of "ON" bars
   std::vector<Int_t> BarNumber;
   std::vector<Int_t> BarNTracks;
   std::vector<Float_t> Energy;
   std::vector<Float_t> Path;
   std::vector<Float_t> Zeta;
   std::vector<Float_t> Time;
   std::vector<Float_t> Phi;
   std::vector<Float_t> ATop;
   std::vector<Float_t> ABot;
   std::vector<Float_t> tTop;
   std::vector<Float_t> tBot;
   ///< "Couple of bars"
   std::vector<Float_t> TOFs;
   std::vector<Float_t> DPHIs;
   std::vector<Float_t> DZETAs;
   std::vector<Float_t> DISTs;
   Float_t TOF_MIN, TOF_MAX, TOF_MEAN, TOF_STD;
   Float_t DPHI_MIN, DPHI_MAX, DPHI_MEAN, DPHI_STD;
   Float_t DZETA_MIN, DZETA_MAX, DZETA_MEAN, DZETA_STD;
   Float_t DIST_MIN, DIST_MAX, DIST_MEAN, DIST_STD;

   // Declaration of leaf types
   TClonesArray    *ScintBarDigiMCTruth;


   // List of branches
   TBranch        *b_ScintBarDigiMCTruth;   //!

   MC_BV_TREE(string file_name);
   virtual ~MC_BV_TREE();
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadBVBarsTree(Long64_t entry);
   virtual void     InitBVBarsTree(TTree *tree);
   virtual void     AnalyzeBVBars();
   virtual Bool_t   Notify();
   virtual void     MeanSigma(std::vector<Float_t>, Float_t&, Float_t&, Float_t&, Float_t&);
   ///< Output tree methods
   virtual void     createOutTree();
   virtual void     resetMCBV();
   virtual void     fillVariables();

};

#endif

#ifdef MC_BV_TREE_cxx

MC_BV_TREE::MC_BV_TREE(string file_name) : tBVBars(0) 
{
   ///< Check the presence of the "pbar" string to assing pbar or "not pbar = cosmics" flag
   pbar = false; ///< default value
   mc = true; ///< Always true if generated with this macro (that reads the MC output)
   if(file_name.find("pbar") != std::string::npos) pbar = true;
   if(file_name.find("cosmic") != std::string::npos) pbar = false;
   
// used to generate this class and read the Tree.
   filename=file_name;
   Int_t dot = filename.Last('.');
   Int_t len = filename.Length();
   filename.Remove(dot,len-dot);

   TString fname =  "simulation/"+filename+".root";
   
   TTree *treeBVBars = 0;
   TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(fname);
   if (!f || !f->IsOpen()) {
      f = new TFile(fname);
   }
   f->GetObject("ScintBarsMCdata",treeBVBars);
   InitBVBarsTree(treeBVBars);

   std::ostringstream out_root_file;
   out_root_file << "simulation/" << "MC_BV_TREE_" << filename << ".root";
   fileOUT = new TFile(out_root_file.str().c_str(),"RECREATE");

}

MC_BV_TREE::~MC_BV_TREE()
{
   if (!tBVBars) return;
   delete tBVBars->GetCurrentFile();
}

Int_t MC_BV_TREE::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!tBVBars) return 0;
   return tBVBars->GetEntry(entry);
}

Long64_t MC_BV_TREE::LoadBVBarsTree(Long64_t entry)
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

void MC_BV_TREE::InitBVBarsTree(TTree *tree)
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

Bool_t MC_BV_TREE::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

#endif // #ifdef MC_BV_TREE_cxx
