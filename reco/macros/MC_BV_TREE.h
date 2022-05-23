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
   Double_t t_event; // event time 
   Bool_t pbar; // pbar annihilation event?
   Bool_t mc;   // mc?
   Int_t nDigi; // number of MC hits/digi (one digi/hit = one track)
   Int_t nBars; // number of "ON" bars
   ///< nBarEnds = number of "bar ends" ON (without cuts nBarEnds = nBars * 2) 
   Int_t nBarEnds; 
   std::vector<Int_t> BarNumber;
   std::vector<Int_t> BarNTracks;
   std::vector<Double_t> Energy;
   std::vector<Double_t> Path;
   std::vector<Double_t> Zeta;
   std::vector<Double_t> Time;
   std::vector<Double_t> Phi;
   ///< -----------------------------------------
   ///< Variables to be stored in the output file
   std::vector<Int_t>    bars_id    ; // bars ID ON
   std::vector<Int_t>    bars_ntrks ; // bars number of tracks
   std::vector<Double_t> bars_edep  ; // bars Edep
   std::vector<Double_t> bars_path  ; // bars Edep
   std::vector<Double_t> bars_z     ; // bars Z
   std::vector<Double_t> bars_t     ; // bars Time
   std::vector<Double_t> bars_phi   ; // bars Phi
   std::vector<Double_t> bars_atop  ; // bars ATop
   std::vector<Double_t> bars_abot  ; // bars ABot
   std::vector<Double_t> bars_ttop  ; // bars tTop
   std::vector<Double_t> bars_tbot  ; // bars tBot
   ///< "Pair of bars" (vectors)
   std::vector<Double_t> pairs_tof  ; // TOF 
   std::vector<Double_t> pairs_dphi ; // Delta Phi
   std::vector<Double_t> pairs_dzeta; // Delta Zeta
   std::vector<Double_t> pairs_dist ; // Distance
   Double_t TOF_MIN, TOF_MAX, TOF_MEAN, TOF_STD;
   Double_t DPHI_MIN, DPHI_MAX, DPHI_MEAN, DPHI_STD;
   Double_t DZETA_MIN, DZETA_MAX, DZETA_MEAN, DZETA_STD;
   Double_t DIST_MIN, DIST_MAX, DIST_MEAN, DIST_STD;

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
   virtual void     MeanSigma(std::vector<Double_t>, Double_t&, Double_t&, Double_t&, Double_t&);
   ///< Output tree methods
   virtual void     createOutTree();
   virtual void     resetEventVariables();
   virtual void     fillOutputEvent();
   virtual Bool_t   applyTrigger();
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
   
   filename=file_name;
   Int_t dot, slash, len;
   dot = filename.Last('.');
   len = filename.Length();
   filename.Remove(dot,len-dot);
   slash = filename.First('/');
   filename.Remove(0,slash+1);

   // TString fname = filename+".root";
   
   TTree *treeBVBars = 0;
   TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(file_name.c_str());
   if (!f || !f->IsOpen()) {
      f = new TFile(file_name.c_str());
   }
   f->GetObject("ScintBarsMCdata",treeBVBars);
   InitBVBarsTree(treeBVBars);

   std::ostringstream out_root_file;
   struct stat st;
   if(stat("root_output_files/",&st) != 0 || ((st.st_mode & (S_IFDIR | S_IFLNK)) == 0)) { // It doesn't exist (we accept links too, sir)
      mkdir("root_output_files/", 0755);
   }
   out_root_file << "root_output_files/" << "MC_BV_TREE_" << filename << ".root";
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
