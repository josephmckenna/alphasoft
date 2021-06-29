#ifndef a2mcToRAW_h
#define a2mcToRAW_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "TObject.h"
#include "TClonesArray.h"

class a2mcToRAW {
public :
    TFile          *fRAWFile;
    TTree          *fChain;   //!pointer to the analyzed TTree or TChain
    Int_t          fCurrent; //!current Tree number in a TChain
    Int_t          fRunNumber;
    Int_t          fTotEvents;
    Long64_t       fEvent;

// Fixed size dimensions of array or collections stored in the TTree if any.
    static constexpr Int_t kMaxSilHits   = 5000;
    static constexpr Int_t kMaxSilDigi   = 5000;
    static constexpr Int_t kMaxMCTracks  = 1000;

    Int_t           fPdgCode;   ///< Primary PDG code
    Double_t        fVox = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin vertex
    Double_t        fVoy = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin vertex
    Double_t        fVoz = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin vertex
    Double_t        fPox = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin momentum
    Double_t        fPoy = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin momentum
    Double_t        fPoz = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin momentum
    Double_t        fEo  = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin energy
    Double_t        fVdx = std::numeric_limits<double>::quiet_NaN(); ///< Primary decay vertex
    Double_t        fVdy = std::numeric_limits<double>::quiet_NaN(); ///< Primary decay vertex
    Double_t        fVdz = std::numeric_limits<double>::quiet_NaN(); ///< Primary decay vertex
    UInt_t          fUniqueID;
    UInt_t          fBits;
    Int_t           SilHits_;                       ///< Number of silicon hits
    Int_t           SilHits_fTrackID[kMaxSilHits];  ///< Silicon hit track
    Int_t           SilHits_fPdgCode[kMaxSilHits];  ///< Silicon hit pdg code
    Int_t           SilHits_fMotherID[kMaxSilHits]; ///< Silicon hit mother ID (0 = from primary)
    Int_t           SilHits_fEvent[kMaxSilHits];    ///< Silicon hit 
    Int_t           SilHits_fSilID[kMaxSilHits];    ///< Silicon hit silicon module [0-71]
    Int_t           SilHits_fLayN[kMaxSilHits];     ///< Silicon hit silicon layer [0-5]
    Int_t           SilHits_fModN[kMaxSilHits];     ///< Silicon hit silicon layer/module [0-9], [0-11], [0-13] (depending on layer)
    Int_t           SilHits_fnStrp[kMaxSilHits];    ///< Silicon hit n-Strip
    Int_t           SilHits_fpStrp[kMaxSilHits];    ///< Silicon hit p-Strip
    Double_t        SilHits_fEdep[kMaxSilHits];     ///< Silicon hit deposited energy
    Double_t        SilHits_fPosX[kMaxSilHits];     ///< Silicon hit position in the global reference system
    Double_t        SilHits_fPosY[kMaxSilHits];     ///< Silicon hit position in the global reference system
    Double_t        SilHits_fPosZ[kMaxSilHits];     ///< Silicon hit position in the global reference system
    Double_t        SilHits_fMomX[kMaxSilHits];     ///< Silicon hit momentum of the track
    Double_t        SilHits_fMomY[kMaxSilHits];     ///< Silicon hit momentum of the track
    Double_t        SilHits_fMomZ[kMaxSilHits];     ///< Silicon hit momentum of the track
    UInt_t          SilHits_fUniqueID[kMaxSilHits]; 
    UInt_t          SilHits_fBits[kMaxSilHits];     
    Int_t           SilDigi_;
    Int_t           SilDigi_fElemID[kMaxSilDigi];   //[SilDigi_]
    Double_t        SilDigi_fEnergy[kMaxSilDigi];   //[SilDigi_]
    UInt_t          SilDigi_fUniqueID[kMaxSilDigi];   //[SilDigi_]
    UInt_t          SilDigi_fBits[kMaxSilDigi];   //[SilDigi_]
    Int_t           MCTracks_;
    Int_t           MCTracks_fPdgCode[kMaxMCTracks];    //[MCTracks_]
    Int_t           MCTracks_fMother[kMaxMCTracks][2];  //[MCTracks_]
    Int_t           MCTracks_fDaughter[kMaxMCTracks][2];//[MCTracks_]
    Int_t           MCTracks_fTrackID[kMaxMCTracks];    //[MCTracks_]
    Double_t        MCTracks_fPx[kMaxMCTracks];         //[MCTracks_]
    Double_t        MCTracks_fPy[kMaxMCTracks];         //[MCTracks_]
    Double_t        MCTracks_fPz[kMaxMCTracks];         //[MCTracks_]
    Double_t        MCTracks_fE[kMaxMCTracks];          //[MCTracks_]
    Double_t        MCTracks_fVx[kMaxMCTracks];         //[MCTracks_]
    Double_t        MCTracks_fVy[kMaxMCTracks];         //[MCTracks_]
    Double_t        MCTracks_fVz[kMaxMCTracks];         //[MCTracks_]
    Double_t        MCTracks_fVt[kMaxMCTracks];         //[MCTracks_]
    UInt_t          MCTracks_fUniqueID[kMaxMCTracks];   //[MCTracks_]
    UInt_t          MCTracks_fBits[kMaxMCTracks];       //[MCTracks_]
    // List of branches
    TBranch        *b_Primary_fUniqueID;   //!
    TBranch        *b_Primary_fBits;   //!
    TBranch        *b_Primary_fPdgCode;   //!
    TBranch        *b_Primary_fVox;   //!
    TBranch        *b_Primary_fVoy;   //!
    TBranch        *b_Primary_fVoz;   //!
    TBranch        *b_Primary_fPox;   //!
    TBranch        *b_Primary_fPoy;   //!
    TBranch        *b_Primary_fPoz;   //!
    TBranch        *b_Primary_fEo;   //!
    TBranch        *b_Primary_fVdx;   //!
    TBranch        *b_Primary_fVdy;   //!
    TBranch        *b_Primary_fVdz;   //!
    TBranch        *b_SilHits_;   //!
    TBranch        *b_SilHits_fUniqueID;   //!
    TBranch        *b_SilHits_fBits;   //!
    TBranch        *b_SilHits_fTrackID;   //!
    TBranch        *b_SilHits_fPdgCode;   //!
    TBranch        *b_SilHits_fMotherID;   //!
    TBranch        *b_SilHits_fEvent;   //!
    TBranch        *b_SilHits_fSilID;   //!
    TBranch        *b_SilHits_fLayN;   //!
    TBranch        *b_SilHits_fModN;   //!
    TBranch        *b_SilHits_fnStrp;   //!
    TBranch        *b_SilHits_fpStrp;   //!
    TBranch        *b_SilHits_fEdep;   //!
    TBranch        *b_SilHits_fPosX;   //!
    TBranch        *b_SilHits_fPosY;   //!
    TBranch        *b_SilHits_fPosZ;   //!
    TBranch        *b_SilHits_fMomX;   //!
    TBranch        *b_SilHits_fMomY;   //!
    TBranch        *b_SilHits_fMomZ;   //!
    TBranch        *b_SilDigi_;   //!
    TBranch        *b_SilDigi_fUniqueID;   //!
    TBranch        *b_SilDigi_fBits;   //!
    TBranch        *b_SilDigi_fElemID;   //!
    TBranch        *b_SilDigi_fEnergy;   //!
    TBranch        *b_MCTracks_;   //!
    TBranch        *b_MCTracks_fUniqueID;   //!
    TBranch        *b_MCTracks_fBits;   //!
    TBranch        *b_MCTracks_fPdgCode;   //!
    TBranch        *b_MCTracks_fMother;   //!
    TBranch        *b_MCTracks_fDaughter;   //!
    TBranch        *b_MCTracks_fTrackID;   //!
    TBranch        *b_MCTracks_fPx;   //!
    TBranch        *b_MCTracks_fPy;   //!
    TBranch        *b_MCTracks_fPz;   //!
    TBranch        *b_MCTracks_fE;   //!
    TBranch        *b_MCTracks_fVx;   //!
    TBranch        *b_MCTracks_fVy;   //!
    TBranch        *b_MCTracks_fVz;   //!
    TBranch        *b_MCTracks_fVt;   //!

    a2mcToRAW();
    a2mcToRAW(Int_t runNumber=0);
    virtual ~a2mcToRAW();
    virtual void            Init(Int_t runNumber=0);
    virtual Bool_t          GoodEvent();
    virtual Int_t           GetEntry(Long64_t entry);
    virtual Long64_t        LoadTree(Long64_t entry);
    virtual void            InitTree(TTree *tree);
    virtual void            WriteRAW();
    virtual void            CreateOutputFile();
};

#endif

#ifdef a2mcToRAW_cxx
///< Default constructor
a2mcToRAW::a2mcToRAW() : fChain(0) {
// default constructor
}
///< Constructor with the run number
a2mcToRAW::a2mcToRAW(Int_t runNumber) : fChain(0)  {
    Init(runNumber);
}

///< Initializer
void a2mcToRAW::Init(Int_t runNumber) {
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

a2mcToRAW::~a2mcToRAW()
{
    if (!fChain) return;
    delete fChain->GetCurrentFile();
}

Int_t a2mcToRAW::GetEntry(Long64_t entry)
{
// Read contents of entry.
    if (!fChain) return 0;
    return fChain->GetEntry(entry);
}

Long64_t a2mcToRAW::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
    if (!fChain) return -5;
    Long64_t centry = fChain->LoadTree(entry);
    if (centry < 0) return centry;
    if (fChain->GetTreeNumber() != fCurrent) {
        fCurrent = fChain->GetTreeNumber();
    }
    return centry;
}

void a2mcToRAW::InitTree(TTree *tree)
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

    TLeaf *prim = fChain->FindLeaf("fUniqueID");
    if(prim) {
        fChain->SetBranchAddress("fPdgCode", &fPdgCode, &b_Primary_fPdgCode);
        fChain->SetBranchAddress("fVox", &fVox, &b_Primary_fVox);
        fChain->SetBranchAddress("fVoy", &fVoy, &b_Primary_fVoy);
        fChain->SetBranchAddress("fVoz", &fVoz, &b_Primary_fVoz);
        fChain->SetBranchAddress("fPox", &fPox, &b_Primary_fPox);
        fChain->SetBranchAddress("fPoy", &fPoy, &b_Primary_fPoy);
        fChain->SetBranchAddress("fPoz", &fPoz, &b_Primary_fPoz);
        fChain->SetBranchAddress("fEo", &fEo, &b_Primary_fEo);
        fChain->SetBranchAddress("fVdx", &fVdx, &b_Primary_fVdx);
        fChain->SetBranchAddress("fVdy", &fVdy, &b_Primary_fVdy);
        fChain->SetBranchAddress("fVdz", &fVdz, &b_Primary_fVdz);
    }
    TLeaf *hits = fChain->FindLeaf("SilHits.fUniqueID");
    if(hits) {
        fChain->SetBranchAddress("SilHits", &SilHits_, &b_SilHits_);
        fChain->SetBranchAddress("SilHits.fTrackID", SilHits_fTrackID, &b_SilHits_fTrackID);
        fChain->SetBranchAddress("SilHits.fPdgCode", SilHits_fPdgCode, &b_SilHits_fPdgCode);
        fChain->SetBranchAddress("SilHits.fMotherID", SilHits_fMotherID, &b_SilHits_fMotherID);
        fChain->SetBranchAddress("SilHits.fEvent", SilHits_fEvent, &b_SilHits_fEvent);
        fChain->SetBranchAddress("SilHits.fSilID", SilHits_fSilID, &b_SilHits_fSilID);
        fChain->SetBranchAddress("SilHits.fLayN", SilHits_fLayN, &b_SilHits_fLayN);
        fChain->SetBranchAddress("SilHits.fModN", SilHits_fModN, &b_SilHits_fModN);
        fChain->SetBranchAddress("SilHits.fnStrp", SilHits_fnStrp, &b_SilHits_fnStrp);
        fChain->SetBranchAddress("SilHits.fpStrp", SilHits_fpStrp, &b_SilHits_fpStrp);
        fChain->SetBranchAddress("SilHits.fEdep", SilHits_fEdep, &b_SilHits_fEdep);
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
        fChain->SetBranchAddress("SilDigi.fElemID", SilDigi_fElemID, &b_SilDigi_fElemID);
        fChain->SetBranchAddress("SilDigi.fEnergy", SilDigi_fEnergy, &b_SilDigi_fEnergy);
    } else {
        SilDigi_ = 0;
    }
    TLeaf *trks = fChain->FindLeaf("MCTracks.fUniqueID");
    if(trks) {
        fChain->SetBranchAddress("MCTracks", &MCTracks_, &b_MCTracks_);
        fChain->SetBranchAddress("MCTracks.fPdgCode", MCTracks_fPdgCode, &b_MCTracks_fPdgCode);
        fChain->SetBranchAddress("MCTracks.fMother[2]", MCTracks_fMother, &b_MCTracks_fMother);
        fChain->SetBranchAddress("MCTracks.fDaughter[2]", MCTracks_fDaughter, &b_MCTracks_fDaughter);
        fChain->SetBranchAddress("MCTracks.fTrackID", MCTracks_fTrackID, &b_MCTracks_fTrackID);
        fChain->SetBranchAddress("MCTracks.fPx", MCTracks_fPx, &b_MCTracks_fPx);
        fChain->SetBranchAddress("MCTracks.fPy", MCTracks_fPy, &b_MCTracks_fPy);
        fChain->SetBranchAddress("MCTracks.fPz", MCTracks_fPz, &b_MCTracks_fPz);
        fChain->SetBranchAddress("MCTracks.fE", MCTracks_fE, &b_MCTracks_fE);
        fChain->SetBranchAddress("MCTracks.fVx", MCTracks_fVx, &b_MCTracks_fVx);
        fChain->SetBranchAddress("MCTracks.fVy", MCTracks_fVy, &b_MCTracks_fVy);
        fChain->SetBranchAddress("MCTracks.fVz", MCTracks_fVz, &b_MCTracks_fVz);
        fChain->SetBranchAddress("MCTracks.fVt", MCTracks_fVt, &b_MCTracks_fVt);
    } else {
        MCTracks_ = 0;
    }
}

#endif // #ifdef a2mcToRAW_cxx
