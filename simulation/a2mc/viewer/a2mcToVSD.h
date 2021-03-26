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

#include <limits>
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "TObject.h"
#include "TClonesArray.h"
#include "TEveVSDStructs.h"

class a2mcToVSD {
public :
    a2mcToVSD();
    a2mcToVSD(Int_t runNumber=0);
    virtual ~a2mcToVSD();

    ///< Variables for the MC events
    TFile          *fVSDFile;
    TTree          *fChain;   //!pointer to the analyzed TTree or TChain
    Int_t          fCurrent; //!current Tree number in a TChain
    Int_t          fRunNumber;
    Int_t          fTotEvents;
    Long64_t       fEvent;
    TDatabasePDG   *pdgDB = new TDatabasePDG();

///< Variables for the reconstructed variables
    TAlphaEvent*    fAlphaEvent=nullptr;
    TAlphaEventMap* fAlphaEventMap=nullptr;
    Bool_t          isRecV;
    Double_t        fRecVdx = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin vertex
    Double_t        fRecVdy = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin vertex
    Double_t        fRecVdz = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin vertex
    Double_t        fRecPhi = std::numeric_limits<double>::quiet_NaN();

    ///< Methods
    virtual void            Init(Int_t runNumber=0);
    virtual void            ResetVariables();
    virtual Int_t           GetEntry(Long64_t entry);
    virtual Long64_t        LoadTree(Long64_t entry);
    virtual void            InitTree(TTree *tree);
    virtual void            CreateOutputFile();
    virtual void            WriteVSD(Double_t, Int_t);
    virtual Bool_t          FillVSDEvent(Int_t, Double_t, Int_t);
    virtual void            WriteVSDEvent(vector<TEveHit>&, vector<TEveMCTrack>&, vector<TEveRecTrackF>&, Int_t);
    virtual TEveHit         HitToEveHit(UInt_t);
    virtual TEveHit         PrimaryOriginToEveHit();
    virtual TEveHit         PrimaryDecayToEveHit();
    virtual Bool_t          GoodMCTrack(UInt_t);
    virtual TEveMCTrack     ToEveMCTrack(UInt_t);
    virtual TEveHit         RecoVertexToEveHit();
    virtual TEveRecTrackF   ToEveRecTrack(UInt_t);
    virtual TVector3        RecTrackVo(TVector3, TVector3);
    virtual Bool_t          InWard(TVector3,TVector3);
    virtual void            set_pdg_codes();
    ///< Reco stuff
    virtual void            InitReco();
    virtual Bool_t          FillAlphaEvent(Double_t);

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

};

#endif

#ifdef a2mcToVSD_cxx
///< Default constructor
a2mcToVSD::a2mcToVSD() : fChain(0) {
///<
}
///< Constructor with the run number
a2mcToVSD::a2mcToVSD(Int_t runNumber) : fChain(0)  {
    set_pdg_codes();
    Init(runNumber);
    InitReco();
    CreateOutputFile();
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
    fTotEvents = fChain->GetEntriesFast();
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
    fChain->GetEntry(fEvent);
    return centry;
}

void a2mcToVSD::InitReco() {
    // Initialize geometry
    new TGeoManager("TALPHAGeo", "ALPHA ROOT geometry manager");
  
    // load the material definitions
    TAlphaGeoMaterialXML * materialXML = new TAlphaGeoMaterialXML();
  
    Char_t pathname[128];
    //  sprintf(pathname,"geo/material.xml");
    sprintf(pathname,"%s/a2lib/geo/material2.xml",getenv("AGRELEASE"));
    materialXML->ParseFile(pathname);
    delete materialXML;

    TAlphaGeoEnvironmentXML * environmentXML = new TAlphaGeoEnvironmentXML();
    //  sprintf(pathname,"geo/environment_geo.xml");
    sprintf(pathname,"%s/a2lib/geo/environment2_geo.xml",getenv("AGRELEASE"));
    environmentXML->ParseFile(pathname);
    delete environmentXML;

    TAlphaGeoDetectorXML * detectorXML = new TAlphaGeoDetectorXML();
    //  sprintf(pathname,"geo/detector_geo.xml");
    sprintf(pathname,"%s/a2lib/geo/detector2_geo.xml",getenv("AGRELEASE"));
    detectorXML->ParseFile(pathname);
    delete detectorXML;

    // close geometry
    gGeoManager->CloseGeometry();

    fAlphaEventMap  =   new TAlphaEventMap();  
    fAlphaEvent     =   new TAlphaEvent(fAlphaEventMap);
}

void a2mcToVSD::InitTree(TTree *tree)
{
    // The Init() function is called when the selector needs to initialize
    // a new tree or chain. Typically here the branch addresses and branch
    // pointers of the tree will be set.
    // It is normally not necessary to make changes to the generated
    // code, but the routine can be extended by the user if needed.
    // Init() will be called many times when running on PROOF
    // (once per file to be processed   
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
    TLeaf *sil = fChain->FindLeaf("SilHits.fUniqueID");
    if(sil) {
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

void a2mcToVSD::ResetVariables() { ///< Resetting variables for the next events
    fVox = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin vertex
    fVoy = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin vertex
    fVoz = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin vertex
    fPox = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin momentum
    fPoy = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin momentum
    fPoz = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin momentum
    fEo  = std::numeric_limits<double>::quiet_NaN(); ///< Primary origin energy
    fVdx = std::numeric_limits<double>::quiet_NaN(); ///< Primary decay vertex
    fVdy = std::numeric_limits<double>::quiet_NaN(); ///< Primary decay vertex
    fVdz = std::numeric_limits<double>::quiet_NaN(); ///< Primary decay vertex
    SilHits_    = 0;
    SilDigi_    = 0;
    MCTracks_   = 0;
}

void a2mcToVSD::set_pdg_codes() {
    double mp = 0.938272; // mass of the proton in GeV/c2
    ///< muons          => PDGCode (mu+) 13 or (mu-) -13
    ///< antiprotons    => PDGCode -2212
//TParticlePDG * TDatabasePDG::AddParticle 	(name,title,mass,stable,width,charge,class,PDGcode)
    pdgDB->AddParticle("D","D",         mp*2.,1.,0.,1.,"Nuclei",      1000010020);
    pdgDB->AddParticle("T","T",         mp*3.,1.,0.,1.,"Nuclei",      1000010030);
    pdgDB->AddParticle("He3","He3",     mp*3.,1.,0.,2.,"Nuclei",      1000020030);
    pdgDB->AddParticle("He4","He4",     mp*4.,1.,0.,2.,"Nuclei",      1000020040);
    pdgDB->AddParticle("Li","Li",       mp*6.,1.,0.,3.,"Nuclei",      1000030060);
    pdgDB->AddParticle("Li7","Li7",     mp*7.,1.,0.,27.,"Nuclei",     1000030070);
    pdgDB->AddParticle("B11","B11",     mp*11.,1.,0.,5.,"Nuclei",     1000050110);
    pdgDB->AddParticle("C","C",         mp*12.,1.,0.,6.,"Nuclei",     1000060120);
    pdgDB->AddParticle("N14","N14",     mp*14.,1.,0.,7.,"Nuclei",     1000070140);
    pdgDB->AddParticle("N","N",         mp*15.,1.,0.,7.,"Nuclei",     1000070150);
    pdgDB->AddParticle("O","O",         mp*16.,1.,0.,8.,"Nuclei",     1000080160);
    pdgDB->AddParticle("O15","O15",     mp*15.,1.,0.,8.,"Nuclei",     1000080150);
    pdgDB->AddParticle("F","F",         mp*18.,1.,0.,9.,"Nuclei",     1000090180);
    pdgDB->AddParticle("Ne","Ne",       mp*20.,1.,0.,10.,"Nuclei",    1000100200);
    pdgDB->AddParticle("Ne21","Ne21",   mp*21.,1.,0.,10.,"Nuclei",    1000100210);
    pdgDB->AddParticle("Ne22","Ne22",   mp*22.,1.,0.,10.,"Nuclei",    1000100220);
    pdgDB->AddParticle("Na","Na",       mp*22.,1.,0.,11.,"Nuclei",    1000110220);
    pdgDB->AddParticle("Na23","Na23",   mp*23.,1.,0.,11.,"Nuclei",    1000110230);
    pdgDB->AddParticle("Mg","Mg",       mp*24.,1.,0.,12.,"Nuclei",    1000120240);
    pdgDB->AddParticle("Mg25","Mg25",   mp*25.,1.,0.,12.,"Nuclei",    1000120250);
    pdgDB->AddParticle("Mg26","Mg26",   mp*26.,1.,0.,12.,"Nuclei",    1000120260);
    pdgDB->AddParticle("Al","Al",       mp*25.,1.,0.,13.,"Nuclei",    1000130250);
    pdgDB->AddParticle("Al27","Al27",   mp*27.,1.,0.,13.,"Nuclei",    1000130270);
    pdgDB->AddParticle("Si27","Si27",   mp*27.,1.,0.,14.,"Nuclei",    1000140270);
    pdgDB->AddParticle("Si","Si",       mp*28.,1.,0.,14.,"Nuclei",    1000140280);
    pdgDB->AddParticle("Si30","Si30",   mp*30.,1.,0.,14.,"Nuclei",    1000140300);
    pdgDB->AddParticle("Ca42","Ca42",   mp*42.,1.,0.,27.,"Nuclei",    1000200420);
    pdgDB->AddParticle("Sc43","Sc43",   mp*43.,1.,0.,27.,"Nuclei",    1000210430);
    pdgDB->AddParticle("Ti46","Ti46",   mp*46.,1.,0.,27.,"Nuclei",    1000220460);
    pdgDB->AddParticle("Cr50","Cr50",   mp*50.,1.,0.,27.,"Nuclei",    1000240500);
    pdgDB->AddParticle("Co57","Co57",   mp*57.,1.,0.,27.,"Nuclei",    1000270570);
    pdgDB->AddParticle("Ni60","Ni60",   mp*60.,1.,0.,27.,"Nuclei",    1000280600);
    pdgDB->AddParticle("F21", "F21",    mp*21.,1.,0.,9., "Nuclei",    1000090210);
    pdgDB->AddParticle("K40", "K40",    mp*40.,1.,0.,19.,"Nuclei",    1000190400);
    pdgDB->AddParticle("Ti48", "Ti48",  mp*48.,1.,0.,22.,"Nuclei",    1000220480);
    pdgDB->AddParticle("V47", "V47",    mp*47.,1.,0.,23.,"Nuclei",    1000230470);
    pdgDB->AddParticle("Mn54", "Mn54",  mp*54.,1.,0.,25.,"Nuclei",    1000250540);
    pdgDB->AddParticle("Fe55", "Fe55",  mp*55.,1.,0.,26.,"Nuclei",    1000260550);
    pdgDB->AddParticle("Co59", "Co59",  mp*59.,1.,0.,27.,"Nuclei",    1000270590);
    pdgDB->AddParticle("Ni64", "Ni64",  mp*64.,1.,0.,28.,"Nuclei",    1000280640);
    pdgDB->AddParticle("Cu63", "Cu63",  mp*63.,1.,0.,29.,"Nuclei",    1000290630);
    pdgDB->AddParticle("Y86", "Y86",    mp*86.,1.,0.,39.,"Nuclei",    1000390860);
}

#endif // #ifdef a2mcToVSD_cxx
