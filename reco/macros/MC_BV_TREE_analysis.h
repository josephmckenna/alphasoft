#ifndef MC_BV_TREE_analysis_h
#define MC_BV_TREE_analysis_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// // Header file for the classes stored in the TTree if any.
// #include "TClonesArray.h"
// #include "Riostream.h"


class MC_BV_TREE_analysis {
public :
   TTree          *treeMCBV;
   TString        filename;
   Int_t           fCurrent; //!current Tree number in a TChain
   Int_t          pbar_file_flag;
   Long64_t        n_tot_events;
   // Declaration of leaf types
   Int_t           event;
   Bool_t          pbar; ///< 0 = cosmic, 1 = pbar annihilation
   Bool_t          mc;
   Int_t           nDigi;
   Int_t           nBars;
   vector<int>     *bars_id;
   vector<int>     *bars_ntrks;
   vector<float>   *bars_edep;
   vector<float>   *bars_path;
   vector<float>   *bars_z;
   vector<float>   *bars_t;
   vector<float>   *bars_phi;
   vector<float>   *bars_atop;
   vector<float>   *bars_abot;
   vector<float>   *bars_ttop;
   vector<float>   *bars_tbot;
   vector<float>   *pairs_tof;
   vector<float>   *pairs_dphi;
   vector<float>   *pairs_dzeta;
   vector<float>   *pairs_dist;
   Float_t         tof_min;
   Float_t         tof_max;
   Float_t         tof_mean;
   Float_t         tof_std;
   Float_t         dphi_min;
   Float_t         dphi_max;
   Float_t         dphi_mean;
   Float_t         dphi_std;
   Float_t         dzeta_min;
   Float_t         dzeta_max;
   Float_t         dzeta_mean;
   Float_t         dzeta_std;
   Float_t         dist_min;
   Float_t         dist_max;
   Float_t         dist_mean;
   Float_t         dist_std;
   // List of branches
   TBranch        *b_event;   //!
   TBranch        *b_pbar;   //!
   TBranch        *b_mc;   //!
   TBranch        *b_nDigi;   //!
   TBranch        *b_nBars;   //!
   TBranch        *b_bars_id;   //!
   TBranch        *b_bars_ntrks;   //!
   TBranch        *b_bars_edep;   //!
   TBranch        *b_bars_path;   //!
   TBranch        *b_bars_z;   //!
   TBranch        *b_bars_t;   //!
   TBranch        *b_bars_phi;   //!
   TBranch        *b_bars_atop;   //!
   TBranch        *b_bars_abot;   //!
   TBranch        *b_bars_ttop;   //!
   TBranch        *b_bars_tbot;   //!
   TBranch        *b_pairs_tof;   //!
   TBranch        *b_pairs_dphi;   //!
   TBranch        *b_pairs_dzeta;   //!
   TBranch        *b_pairs_dist;   //!
   TBranch        *b_TOF_MIN;   //!
   TBranch        *b_TOF_MAX;   //!
   TBranch        *b_TOF_MEAN;   //!
   TBranch        *b_TOF_STD;   //!
   TBranch        *b_DPHI_MIN;   //!
   TBranch        *b_DPHI_MAX;   //!
   TBranch        *b_DPHI_MEAN;   //!
   TBranch        *b_DPHI_STD;   //!
   TBranch        *b_DZETA_MIN;   //!
   TBranch        *b_DZETA_MAX;   //!
   TBranch        *b_DZETA_MEAN;   //!
   TBranch        *b_DZETA_STD;   //!
   TBranch        *b_DIST_MIN;   //!
   TBranch        *b_DIST_MAX;   //!
   TBranch        *b_DIST_MEAN;   //!
   TBranch        *b_DIST_STD;   //!


   MC_BV_TREE_analysis(string file_name);
   virtual ~MC_BV_TREE_analysis();
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadMCBVTREE(Long64_t entry);
   virtual void     InitMCBVTREE(TTree *tree);
   virtual void     ShowBV();

   virtual void   CreateHistos();
   virtual void   FillHistos();
   virtual void   ShowHistos(Int_t);

};

#endif

#ifdef MC_BV_TREE_analysis_cxx

MC_BV_TREE_analysis::MC_BV_TREE_analysis(string file_name) : treeMCBV(0) 
{
   pbar_file_flag = -1;
   if(file_name.find("pbar") != std::string::npos) pbar_file_flag = 1;
   if(file_name.find("cosmic") != std::string::npos) pbar_file_flag = 0;
   // used to generate this class and read the Tree.
   filename=file_name;
   Int_t dot = filename.Last('.');
   Int_t len = filename.Length();
   filename.Remove(dot,len-dot);

   TString fname =  "simulation/"+filename+".root";
   
   TTree *tree = 0;
   TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(fname);
   if (!f || !f->IsOpen()) {
      f = new TFile(fname);
   }
   f->GetObject("tMCBV",tree);
   InitMCBVTREE(tree);

}

MC_BV_TREE_analysis::~MC_BV_TREE_analysis()
{
   if (!treeMCBV) return;
   delete treeMCBV->GetCurrentFile();
}

Int_t MC_BV_TREE_analysis::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!treeMCBV) return 0;
   return treeMCBV->GetEntry(entry);
}

Long64_t MC_BV_TREE_analysis::LoadMCBVTREE(Long64_t entry)
{
// Set the environment to read one entry
   if (!treeMCBV) return -5;
   Long64_t centry = treeMCBV->LoadTree(entry);
   if (centry < 0) return centry;
   if (treeMCBV->GetTreeNumber() != fCurrent) {
      fCurrent = treeMCBV->GetTreeNumber();
   }
   return centry;
}

void MC_BV_TREE_analysis::InitMCBVTREE(TTree *tree)
{
   // Set object pointer
   bars_id = 0;
   bars_ntrks = 0;
   bars_edep = 0;
   bars_path = 0;
   bars_z = 0;
   bars_t = 0;
   bars_phi = 0;
   bars_atop = 0;
   bars_abot = 0;
   bars_ttop = 0;
   bars_tbot = 0;
   pairs_tof = 0;
   pairs_dphi = 0;
   pairs_dzeta = 0;
   pairs_dist = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   treeMCBV = tree;
   fCurrent = -1;
   treeMCBV->SetMakeClass(1);
   treeMCBV->SetBranchAddress("event", &event, &b_event);
   treeMCBV->SetBranchAddress("pbar", &pbar, &b_pbar);
   treeMCBV->SetBranchAddress("mc", &mc, &b_mc);
   treeMCBV->SetBranchAddress("nDigi", &nDigi, &b_nDigi);
   treeMCBV->SetBranchAddress("nBars", &nBars, &b_nBars);
   treeMCBV->SetBranchAddress("bars_id", &bars_id, &b_bars_id);
   treeMCBV->SetBranchAddress("bars_ntrks", &bars_ntrks, &b_bars_ntrks);
   treeMCBV->SetBranchAddress("bars_edep", &bars_edep, &b_bars_edep);
   treeMCBV->SetBranchAddress("bars_path", &bars_path, &b_bars_path);
   treeMCBV->SetBranchAddress("bars_z", &bars_z, &b_bars_z);
   treeMCBV->SetBranchAddress("bars_t", &bars_t, &b_bars_t);
   treeMCBV->SetBranchAddress("bars_phi", &bars_phi, &b_bars_phi);
   treeMCBV->SetBranchAddress("bars_atop", &bars_atop, &b_bars_atop);
   treeMCBV->SetBranchAddress("bars_abot", &bars_abot, &b_bars_abot);
   treeMCBV->SetBranchAddress("bars_ttop", &bars_ttop, &b_bars_ttop);
   treeMCBV->SetBranchAddress("bars_tbot", &bars_tbot, &b_bars_tbot);
   treeMCBV->SetBranchAddress("pairs_tof", &pairs_tof, &b_pairs_tof);
   treeMCBV->SetBranchAddress("pairs_dphi", &pairs_dphi, &b_pairs_dphi);
   treeMCBV->SetBranchAddress("pairs_dzeta", &pairs_dzeta, &b_pairs_dzeta);
   treeMCBV->SetBranchAddress("pairs_dist", &pairs_dist, &b_pairs_dist);
   treeMCBV->SetBranchAddress("tof_min", &tof_min, &b_TOF_MIN);
   treeMCBV->SetBranchAddress("tof_max", &tof_max, &b_TOF_MAX);
   treeMCBV->SetBranchAddress("tof_mean", &tof_mean, &b_TOF_MEAN);
   treeMCBV->SetBranchAddress("tof_std", &tof_std, &b_TOF_STD);
   treeMCBV->SetBranchAddress("dphi_min", &dphi_min, &b_DPHI_MIN);
   treeMCBV->SetBranchAddress("dphi_max", &dphi_max, &b_DPHI_MAX);
   treeMCBV->SetBranchAddress("dphi_mean", &dphi_mean, &b_DPHI_MEAN);
   treeMCBV->SetBranchAddress("dphi_std", &dphi_std, &b_DPHI_STD);
   treeMCBV->SetBranchAddress("dzeta_min", &dzeta_min, &b_DZETA_MIN);
   treeMCBV->SetBranchAddress("dzeta_max", &dzeta_max, &b_DZETA_MAX);
   treeMCBV->SetBranchAddress("dzeta_mean", &dzeta_mean, &b_DZETA_MEAN);
   treeMCBV->SetBranchAddress("dzeta_std", &dzeta_std, &b_DZETA_STD);
   treeMCBV->SetBranchAddress("dist_min", &dist_min, &b_DIST_MIN);
   treeMCBV->SetBranchAddress("dist_max", &dist_max, &b_DIST_MAX);
   treeMCBV->SetBranchAddress("dist_mean", &dist_mean, &b_DIST_MEAN);
   treeMCBV->SetBranchAddress("dist_std", &dist_std, &b_DIST_STD);

}

#endif // #ifdef MC_BV_TREE_analysis_cxx
