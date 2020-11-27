//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Nov 24 15:15:20 2020 by ROOT version 6.22/02
// from TTree a2MC/a2MC tree
// found on file: ../root/a2MC-2020-11-24-11-50-19_29.root
//////////////////////////////////////////////////////////
///< It has then modified to read the a2mc output 
///< (geo and data) and create a proper VSD output file.
#ifndef a2mcToVSD_h
#define a2mcToVSD_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "TObject.h"
#include "TClonesArray.h"
#include "TEveVSDStructs.h"

class a2mcToVSD {
public :
    TFile          *fVSDFile;
    TTree          *fChain;   //!pointer to the analyzed TTree or TChain
    Int_t           fCurrent; //!current Tree number in a TChain
    Int_t          fRunNumber;
    Int_t          fTotEvents;
    Long64_t       fEvent;

// Fixed size dimensions of array or collections stored in the TTree if any.
   static constexpr Int_t kMaxSilHits       = 5000;
   static constexpr Int_t kMaxSilDigi       = 5000;
   static constexpr Int_t kMaxa2mcMCTrack   = 1000;

// Declaration of leaf types
   Int_t           SilHits_;
   UInt_t          SilHits_fUniqueID[kMaxSilHits];   //[SilHits_]
   Int_t           SilHits_fTrackID[kMaxSilHits];   //[SilHits_]
   Int_t           SilHits_fPdgCode[kMaxSilHits];   //[SilHits_]
   Int_t           SilHits_fMotherID[kMaxSilHits];   //[SilHits_]
   Int_t           SilHits_fEvent[kMaxSilHits];   //[SilHits_]
   Int_t           SilHits_fHalfID[kMaxSilHits];   //[SilHits_]
   Int_t           SilHits_fLayerID[kMaxSilHits];   //[SilHits_]
   Int_t           SilHits_fModuleID[kMaxSilHits];   //[SilHits_]
   Double_t        SilHits_fEdep[kMaxSilHits];   //[SilHits_]
   Double_t        SilHits_fTime[kMaxSilHits];   //[SilHits_]
   Double_t        SilHits_fStep[kMaxSilHits];   //[SilHits_]
   TVector3        SilHits_fPos[kMaxSilHits];
   TVector3        SilHits_fMom[kMaxSilHits];
   Double_t        SilHits_fPosX[kMaxSilHits];   //[SilHits_]
   Double_t        SilHits_fPosY[kMaxSilHits];   //[SilHits_]
   Double_t        SilHits_fPosZ[kMaxSilHits];   //[SilHits_]
   Double_t        SilHits_fMomX[kMaxSilHits];   //[SilHits_]
   Double_t        SilHits_fMomY[kMaxSilHits];   //[SilHits_]
   Double_t        SilHits_fMomZ[kMaxSilHits];   //[SilHits_]
   Int_t           SilDigi_;
   UInt_t          SilDigi_fUniqueID[kMaxSilDigi];   //[SilDigi_]
   Int_t           SilDigi_fElemID[kMaxSilDigi];   //[SilDigi_]
   Double_t        SilDigi_fEnergy[kMaxSilDigi];   //[SilDigi_]
   Int_t           a2mcMCTrack_;
   UInt_t          a2mcMCTrack_fUniqueID[kMaxa2mcMCTrack];   //[a2mcMCTrack_]
   Int_t           a2mcMCTrack_fPdgCode[kMaxa2mcMCTrack];   //[a2mcMCTrack_]
   Int_t           a2mcMCTrack_fMother[kMaxa2mcMCTrack][2];   //[a2mcMCTrack_]
   Int_t           a2mcMCTrack_fDaughter[kMaxa2mcMCTrack][2];   //[a2mcMCTrack_]
   Double_t        a2mcMCTrack_fPx[kMaxa2mcMCTrack];   //[a2mcMCTrack_]
   Double_t        a2mcMCTrack_fPy[kMaxa2mcMCTrack];   //[a2mcMCTrack_]
   Double_t        a2mcMCTrack_fPz[kMaxa2mcMCTrack];   //[a2mcMCTrack_]
   Double_t        a2mcMCTrack_fE[kMaxa2mcMCTrack];   //[a2mcMCTrack_]
   Double_t        a2mcMCTrack_fVx[kMaxa2mcMCTrack];   //[a2mcMCTrack_]
   Double_t        a2mcMCTrack_fVy[kMaxa2mcMCTrack];   //[a2mcMCTrack_]
   Double_t        a2mcMCTrack_fVz[kMaxa2mcMCTrack];   //[a2mcMCTrack_]
   Double_t        a2mcMCTrack_fVt[kMaxa2mcMCTrack];   //[a2mcMCTrack_]

    // List of branches
    TBranch        *b_SilHits_;   //!
    TBranch        *b_SilHits_fUniqueID;   //!
    TBranch        *b_SilHits_fTrackID;   //!
    TBranch        *b_SilHits_fPdgCode;   //!
    TBranch        *b_SilHits_fMotherID;   //!
    TBranch        *b_SilHits_fEvent;   //!
    TBranch        *b_SilHits_fHalfID;   //!
    TBranch        *b_SilHits_fLayerID;   //!
    TBranch        *b_SilHits_fModuleID;   //!
    TBranch        *b_SilHits_fEdep;   //!
    TBranch        *b_SilHits_fTime;   //!
    TBranch        *b_SilHits_fStep;   //!
    TBranch        *b_SilHits_fPos;   //!
    TBranch        *b_SilHits_fMom;   //!
    TBranch        *b_SilHits_fPosX;   //!
    TBranch        *b_SilHits_fPosY;   //!
    TBranch        *b_SilHits_fPosZ;   //!
    TBranch        *b_SilHits_fMomX;   //!
    TBranch        *b_SilHits_fMomY;   //!
    TBranch        *b_SilHits_fMomZ;   //!
    TBranch        *b_SilDigi_;   //!
    TBranch        *b_SilDigi_fUniqueID;   //!
    TBranch        *b_SilDigi_fElemID;   //!
    TBranch        *b_SilDigi_fEnergy;   //!
    TBranch        *b_a2mcMCTrack_;   //!
    TBranch        *b_a2mcMCTrack_fUniqueID;   //!
    TBranch        *b_a2mcMCTrack_fPdgCode;   //!
    TBranch        *b_a2mcMCTrack_fMother;   //!
    TBranch        *b_a2mcMCTrack_fDaughter;   //!
    TBranch        *b_a2mcMCTrack_fPx;   //!
    TBranch        *b_a2mcMCTrack_fPy;   //!
    TBranch        *b_a2mcMCTrack_fPz;   //!
    TBranch        *b_a2mcMCTrack_fE;   //!
    TBranch        *b_a2mcMCTrack_fVx;   //!
    TBranch        *b_a2mcMCTrack_fVy;   //!
    TBranch        *b_a2mcMCTrack_fVz;   //!
    TBranch        *b_a2mcMCTrack_fVt;   //!

    a2mcToVSD();
    a2mcToVSD(Int_t runNumber=0);
    virtual ~a2mcToVSD();
    virtual void            Init(Int_t runNumber=0);
    virtual Int_t           Cut(Long64_t entry);
    virtual Int_t           GetEntry(Long64_t entry);
    virtual Long64_t        LoadTree(Long64_t entry);
    virtual void            InitTree(TTree *tree);
    virtual void            CreateOutputFile();
    virtual void            WriteVSD();
    virtual void            WriteEventVSD(vector<TEveHit>&, vector<TEveMCTrack>&, vector<TEveRecTrackF>&, Int_t);
    virtual TEveHit         HitToEveHit(UInt_t);
    virtual TEveRecTrackF   ToEveRecTrack(UInt_t);
    virtual TEveMCTrack     ToEveMCTrack(UInt_t);
    virtual Bool_t          GoodTrack(UInt_t);
    virtual Bool_t          Notify();
    virtual void            Show(Long64_t entry = -1);
};

#endif

#ifdef a2mcToVSD_cxx
///< Default constructor
a2mcToVSD::a2mcToVSD() : fChain(0) {
///<
}
///< Constructor with the run number
a2mcToVSD::a2mcToVSD(Int_t runNumber) : fChain(0)  {
    Init(runNumber);
}

///< Initializer
void a2mcToVSD::Init(Int_t runNumber) {
    fRunNumber = runNumber;
    TTree *tree = 0;
    std::ostringstream sdata;
    sdata << "ls ../root/a2MC-*_" << runNumber << ".root";
    TString file_name(gSystem->GetFromPipe(sdata.str().c_str()));
    string sfile = file_name.Data();
    if(strcmp(sfile.c_str(),"")==0) {
        cout << "Please check presence of file " << sfile << endl;
        return;
    }

    TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(sfile.c_str());
    if (!f || !f->IsOpen()) {
        f = new TFile(sfile.c_str());
    }
    f->GetObject("a2MC",tree);
    InitTree(tree);
}

a2mcToVSD::~a2mcToVSD()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t a2mcToVSD::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t a2mcToVSD::LoadTree(Long64_t entry)
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

void a2mcToVSD::InitTree(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

    TLeaf *hits = fChain->FindLeaf("SilHits.fUniqueID");
    if(hits) {
        fChain->SetBranchAddress("SilHits", &SilHits_, &b_SilHits_);
        fChain->SetBranchAddress("SilHits.fUniqueID", SilHits_fUniqueID, &b_SilHits_fUniqueID);
        fChain->SetBranchAddress("SilHits.fTrackID", SilHits_fTrackID, &b_SilHits_fTrackID);
        fChain->SetBranchAddress("SilHits.fPdgCode", SilHits_fPdgCode, &b_SilHits_fPdgCode);
        fChain->SetBranchAddress("SilHits.fMotherID", SilHits_fMotherID, &b_SilHits_fMotherID);
        fChain->SetBranchAddress("SilHits.fEvent", SilHits_fEvent, &b_SilHits_fEvent);
        fChain->SetBranchAddress("SilHits.fHalfID", SilHits_fHalfID, &b_SilHits_fHalfID);
        fChain->SetBranchAddress("SilHits.fLayerID", SilHits_fLayerID, &b_SilHits_fLayerID);
        fChain->SetBranchAddress("SilHits.fModuleID", SilHits_fModuleID, &b_SilHits_fModuleID);
        fChain->SetBranchAddress("SilHits.fEdep", SilHits_fEdep, &b_SilHits_fEdep);
        fChain->SetBranchAddress("SilHits.fTime", SilHits_fTime, &b_SilHits_fTime);
        fChain->SetBranchAddress("SilHits.fStep", SilHits_fStep, &b_SilHits_fStep);
        fChain->SetBranchAddress("SilHits.fPos", SilHits_fPos, &b_SilHits_fPos);
        fChain->SetBranchAddress("SilHits.fMom", SilHits_fMom, &b_SilHits_fMom);
        fChain->SetBranchAddress("SilHits.fPosX", SilHits_fPosX, &b_SilHits_fPosX);
        fChain->SetBranchAddress("SilHits.fPosY", SilHits_fPosY, &b_SilHits_fPosY);
        fChain->SetBranchAddress("SilHits.fPosZ", SilHits_fPosZ, &b_SilHits_fPosZ);
        fChain->SetBranchAddress("SilHits.fMomX", SilHits_fMomX, &b_SilHits_fMomX);
        fChain->SetBranchAddress("SilHits.fMomY", SilHits_fMomY, &b_SilHits_fMomY);
        fChain->SetBranchAddress("SilHits.fMomZ", SilHits_fMomZ, &b_SilHits_fMomZ);
    } else {
        SilHits_ = 0;
    }
    TLeaf *digi = fChain->FindLeaf("SilDigi.fUniqueID");
    if(digi) {
        fChain->SetBranchAddress("SilDigi", &SilDigi_, &b_SilDigi_);
        fChain->SetBranchAddress("SilDigi.fUniqueID", SilDigi_fUniqueID, &b_SilDigi_fUniqueID);
        fChain->SetBranchAddress("SilDigi.fElemID", SilDigi_fElemID, &b_SilDigi_fElemID);
        fChain->SetBranchAddress("SilDigi.fEnergy", SilDigi_fEnergy, &b_SilDigi_fEnergy);
    } else {
        SilDigi_ = 0;
    }
    TLeaf *trks = fChain->FindLeaf("a2mcMCTrack.fUniqueID");
    if(trks) {
        fChain->SetBranchAddress("a2mcMCTrack", &a2mcMCTrack_, &b_a2mcMCTrack_);
        fChain->SetBranchAddress("a2mcMCTrack.fUniqueID", a2mcMCTrack_fUniqueID, &b_a2mcMCTrack_fUniqueID);
        fChain->SetBranchAddress("a2mcMCTrack.fPdgCode", a2mcMCTrack_fPdgCode, &b_a2mcMCTrack_fPdgCode);
        fChain->SetBranchAddress("a2mcMCTrack.fMother[2]", a2mcMCTrack_fMother, &b_a2mcMCTrack_fMother);
        fChain->SetBranchAddress("a2mcMCTrack.fDaughter[2]", a2mcMCTrack_fDaughter, &b_a2mcMCTrack_fDaughter);
        fChain->SetBranchAddress("a2mcMCTrack.fPx", a2mcMCTrack_fPx, &b_a2mcMCTrack_fPx);
        fChain->SetBranchAddress("a2mcMCTrack.fPy", a2mcMCTrack_fPy, &b_a2mcMCTrack_fPy);
        fChain->SetBranchAddress("a2mcMCTrack.fPz", a2mcMCTrack_fPz, &b_a2mcMCTrack_fPz);
        fChain->SetBranchAddress("a2mcMCTrack.fE", a2mcMCTrack_fE, &b_a2mcMCTrack_fE);
        fChain->SetBranchAddress("a2mcMCTrack.fVx", a2mcMCTrack_fVx, &b_a2mcMCTrack_fVx);
        fChain->SetBranchAddress("a2mcMCTrack.fVy", a2mcMCTrack_fVy, &b_a2mcMCTrack_fVy);
        fChain->SetBranchAddress("a2mcMCTrack.fVz", a2mcMCTrack_fVz, &b_a2mcMCTrack_fVz);
        fChain->SetBranchAddress("a2mcMCTrack.fVt", a2mcMCTrack_fVt, &b_a2mcMCTrack_fVt);
    } else {
        a2mcMCTrack_ = 0;
    }
    Notify();
}

Bool_t a2mcToVSD::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void a2mcToVSD::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t a2mcToVSD::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef a2mcToVSD_cxx
