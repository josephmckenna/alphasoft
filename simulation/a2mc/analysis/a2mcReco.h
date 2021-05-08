#ifndef a2mcReco_h
#define a2mcReco_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "TObject.h"
#include "TClonesArray.h"
///< ================== a2mcReco CLASS ================== 
class a2mcReco {

///< Methods declaration 
public:
    a2mcReco();
    a2mcReco(Int_t runNumber=0, Double_t hit_threshold = 0., Int_t nMinHits=0);
    virtual ~a2mcReco();
    virtual void            Init();
    virtual void            InitTree(TTree *tree);
    virtual Long64_t        LoadTree(Long64_t entry);
    virtual Int_t           GetEntry(Long64_t entry);
    virtual void            InitReco();
    virtual Bool_t          GoodEvent();
    virtual Bool_t          FillAlphaEvent();
    virtual void            Reco(bool);
    virtual void            CreateHistos();
    virtual void            FillHistos();
    virtual void            ShowHistos(bool);
    virtual void            CreateOutputFile();

private:    
///< Histos
    TH1F *hMCVdx, *hMCVdy, *hMCPhi, *hMCVdz, *hMCVdr;
    TH1F *hRecVdx, *hRecVdy, *hRecPhi, *hRecVdz, *hRecVdr;
    TH1F *hDiffVdx, *hDiffVdy, *hDiffPhi, *hDiffVdz, *hDiffVdr;

///< Variables
    TFile          *fRAWFile;
    TTree          *fChain;     //!pointer to the analyzed TTree or TChain
    Int_t          fCurrent;    //!current Tree number in a TChain
    Int_t          fRunNumber;
    Int_t          fTotEvents;
    Long64_t       fEvent;
    Double_t       fHitThreshold; ///< In MeV
    Int_t          fNMinHits;

///< Variables for the reconstructed variables
    TAlphaEvent*    fAlphaEvent=nullptr;
    TAlphaEventMap* fAlphaEventMap=nullptr;
    Bool_t          isRecV;
    Double_t        fRecVdx = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin vertex
    Double_t        fRecVdy = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin vertex
    Double_t        fRecVdz = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin vertex
    Double_t        fRecPhi = std::numeric_limits<double>::quiet_NaN();
///< Variables for the MC output
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
    UInt_t          SilDigi_fUniqueID[kMaxSilDigi]; //[SilDigi_]
    UInt_t          SilDigi_fBits[kMaxSilDigi];     //[SilDigi_]
    Int_t           MCTracks_;
    Int_t           MCTracks_fPdgCode[kMaxMCTracks];    //[MCTracks_]
    Int_t           MCTracks_fMother[kMaxMCTracks][2];  //[MCTracks_]
    Int_t           MCTracks_fDaughter[kMaxMCTracks][2];//[MCTracks_]
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

};

#endif ///< #ifndef a2mcReco_h

#ifdef a2mcReco_cxx

///< Default constructor
a2mcReco::a2mcReco() : fChain(0) {
}

///< Constructor with the run number
a2mcReco::a2mcReco(Int_t runNumber=0, Double_t hit_threshold = 0., Int_t nMinHits=0) : fChain(0)  {
    fRunNumber      = runNumber;
    fHitThreshold   = hit_threshold;
    fNMinHits       = nMinHits;
    Init();
    InitReco();
}

///< Destructor
a2mcReco::~a2mcReco()
{
    if (!fChain) return;
    delete fChain->GetCurrentFile();
    if(fAlphaEventMap!=nullptr)  delete fAlphaEventMap;
    if(fAlphaEvent!=nullptr)     delete fAlphaEvent;
}

///< Initializer (it reads the MC output)
void a2mcReco::Init() {
    TTree *tree = 0;
    std::ostringstream sdata;
    sdata << "ls ../root/a2MC-*_" << fRunNumber << ".root";
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

///< Utility methods
Int_t a2mcReco::GetEntry(Long64_t entry)
{
// Read contents of entry.
    if (!fChain) return 0;
    return fChain->GetEntry(entry);
}

Long64_t a2mcReco::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
    if (!fChain) return -5;
    Long64_t centry = fChain->LoadTree(entry);
    if (centry < 0) return centry;
    if (fChain->GetTreeNumber() != fCurrent) {
        fCurrent = fChain->GetTreeNumber();
    }
    fChain->GetEntry(entry);
    return centry;
}

void a2mcReco::InitTree(TTree *tree)
{
    // Set branch addresses
    if (!tree) return;
    fChain = tree;
    fCurrent = -1;
    fChain->SetMakeClass(1);

    TLeaf *prim = fChain->FindLeaf("fUniqueID");
    if(prim) {
        fChain->SetBranchAddress("fPdgCode", &fPdgCode);
        fChain->SetBranchAddress("fVox", &fVox);
        fChain->SetBranchAddress("fVoy", &fVoy);
        fChain->SetBranchAddress("fVoz", &fVoz);
        fChain->SetBranchAddress("fPox", &fPox);
        fChain->SetBranchAddress("fPoy", &fPoy);
        fChain->SetBranchAddress("fPoz", &fPoz);
        fChain->SetBranchAddress("fEo" , &fEo);
        fChain->SetBranchAddress("fVdx", &fVdx);
        fChain->SetBranchAddress("fVdy", &fVdy);
        fChain->SetBranchAddress("fVdz", &fVdz);
    }
    TLeaf *hits = fChain->FindLeaf("SilHits.fUniqueID");
    if(hits) {
        fChain->SetBranchAddress("SilHits", &SilHits_);
        fChain->SetBranchAddress("SilHits.fTrackID", SilHits_fTrackID);
        fChain->SetBranchAddress("SilHits.fPdgCode", SilHits_fPdgCode);
        fChain->SetBranchAddress("SilHits.fMotherID", SilHits_fMotherID);
        fChain->SetBranchAddress("SilHits.fEvent", SilHits_fEvent);
        fChain->SetBranchAddress("SilHits.fSilID", SilHits_fSilID);
        fChain->SetBranchAddress("SilHits.fLayN", SilHits_fLayN);
        fChain->SetBranchAddress("SilHits.fModN", SilHits_fModN);
        fChain->SetBranchAddress("SilHits.fnStrp", SilHits_fnStrp);
        fChain->SetBranchAddress("SilHits.fpStrp", SilHits_fpStrp);
        fChain->SetBranchAddress("SilHits.fEdep", SilHits_fEdep);
        fChain->SetBranchAddress("SilHits.fPosX", SilHits_fPosX);
        fChain->SetBranchAddress("SilHits.fPosY", SilHits_fPosY);
        fChain->SetBranchAddress("SilHits.fPosZ", SilHits_fPosZ);
        fChain->SetBranchAddress("SilHits.fMomX", SilHits_fMomX);
        fChain->SetBranchAddress("SilHits.fMomY", SilHits_fMomY);
        fChain->SetBranchAddress("SilHits.fMomZ", SilHits_fMomZ);
    } else {
        SilHits_ = 0;
    }
    TLeaf *digi = fChain->FindLeaf("SilDigi.fUniqueID");
    if(digi) {
        fChain->SetBranchAddress("SilDigi", &SilDigi_);
        fChain->SetBranchAddress("SilDigi.fElemID", SilDigi_fElemID);
        fChain->SetBranchAddress("SilDigi.fEnergy", SilDigi_fEnergy);
    } else {
        SilDigi_ = 0;
    }
    TLeaf *trks = fChain->FindLeaf("MCTracks.fUniqueID");
    if(trks) {
        fChain->SetBranchAddress("MCTracks", &MCTracks_);
        fChain->SetBranchAddress("MCTracks.fPdgCode", MCTracks_fPdgCode);
        fChain->SetBranchAddress("MCTracks.fMother[2]", MCTracks_fMother);
        fChain->SetBranchAddress("MCTracks.fDaughter[2]", MCTracks_fDaughter);
        fChain->SetBranchAddress("MCTracks.fPx", MCTracks_fPx);
        fChain->SetBranchAddress("MCTracks.fPy", MCTracks_fPy);
        fChain->SetBranchAddress("MCTracks.fPz", MCTracks_fPz);
        fChain->SetBranchAddress("MCTracks.fE" , MCTracks_fE );
        fChain->SetBranchAddress("MCTracks.fVx", MCTracks_fVx);
        fChain->SetBranchAddress("MCTracks.fVy", MCTracks_fVy);
        fChain->SetBranchAddress("MCTracks.fVz", MCTracks_fVz);
        fChain->SetBranchAddress("MCTracks.fVt", MCTracks_fVt);
    } else {
        MCTracks_ = 0;
    }
}

#endif ///< #ifdef a2mcReco_cxx
