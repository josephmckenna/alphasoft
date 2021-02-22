///< ##################################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##################################################
///< This class is a modification of the code contained 
///< in $ROOTSYS/tutorials/eve/alice_vsd.C and properly 
///< modified for a2mc. This class read the a2mc output 
///< (geo and data) and create a proper VSD output file
///< THE VSD output file MUST HAVE a fixed structure. 
///< For each "event" there is a Directory (such as Event0008).
///< Each "event directory" has the same TTrees substructure 
///<        (4 TTrees with a corresponding TBranch)
///<    TTree       -> TBranch
///<    Hits        ->  H
///<    Clusters    ->  C
///<    Kinematics  ->  K
///<    RecTracks   ->  R
///< Each TBranch is filled with a particular TObject
///<    TBranch     -> TObject
///<    H           -> TEveHit
///<    C           -> TEveCluster
///<    K           -> TEveMCTrack [in our case the a2MCTrack]
///<    R           -> TEveRecTrackF [in our case we don't have the reconstructed track - using the a2MCTrack]
///< These TObjects are defined in TEveVSDStructs.h [https://root.cern.ch/doc/v608/TEveVSDStructs_8h_source.html]
///< This Class "translate" the data of the Alpha2 MC output into a VSD output file
///<

#define a2mcToVSD_cxx
#include "a2mcToVSD.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

//____________________________________________________________________________
///< Visualization parameters
///< The user can decide to see events in which the silicon detector has hits (or not)
///< The user can decide to "visualize" only the tracks that crossed the detector (or all of them)
///< If you want to see all the events (also that don't cross the detectors) -> filter_nSilHits = -1
Int_t filter_nSilHits = 1; 
Bool_t tracks_through_det = true; 
///< Writing the VSD data file (from the MC output data file to the VSD data format)
//=====================
void a2mcToVSD::WriteVSD() {
    if (fChain == 0) return;
    if(tracks_through_det&&filter_nSilHits==-1) filter_nSilHits = 0; ///< if only tracks through dets, do not show events with 0 hits
    CreateOutputFile();
    fTotEvents = fChain->GetEntriesFast();
///< Loop on the events -> select the hits and the MC tracks to write to 
    Int_t nOutEvents = 0;
    cout << "Number of events " << fTotEvents << endl;
    for (fEvent=0; fEvent<fTotEvents; fEvent++) {
        Long64_t ientry = LoadTree(fEvent);
        if (ientry < 0) break;
        fChain->GetEntry(fEvent);
        if(SilHits_<=filter_nSilHits) continue; ///< Skipping events with n. hits <= filter_nSilHits
        //        cout << "Event " << fEvent << " - hits -> " << SilHits_ << " - tracks -> " << MCTracks_ << endl;
        ///< Loading the hits into a local vector<TEveHit>
        vector<TEveHit> hits;
        if(!isnan(fVox)) hits.push_back(PrimaryOriginToEveHit());
        if(!isnan(fVdx)) hits.push_back(PrimaryDecayToEveHit());
        for(UInt_t ih=0; ih<SilHits_; ih++) {
            hits.push_back(HitToEveHit(ih)); ///< Writing hits. It would be possible to write "Digi" instead
        }
        
        ///< Loading the tracks into a local vector<TEveMCTrack> [filtering with GoodTrack()]
        vector<TEveMCTrack> tracksMC;
        for(UInt_t it=0; it<MCTracks_; it++) {
            if(GoodTrack(it)) tracksMC.push_back(ToEveMCTrack(it));
        }
        ///< Loading the tracks into a local vector<TEveRecTrackF>
        vector<TEveRecTrackF> tracksRec;
//        This part can be used to load "reconstructed tracks" [NOT AVAILABLE AT THE MOMENT]
//        for(UInt_t it=0; it<MCTracks_; it++) {
//            tracksRec.push_back(ToEveRecTrack(it));
//        }
        ///< Writing the data in the VSD structure
        WriteEventVSD(hits, tracksMC, tracksRec, fEvent);
        nOutEvents++;
    }
    fVSDFile->Close();
    cout << "Writing " << nOutEvents << " \'good\' events in the VSD output file" << endl;
}

///< Writing the "Events" into the VSD data file 
//=====================
void a2mcToVSD::WriteEventVSD(vector<TEveHit>& hits, vector<TEveMCTrack>& mcTracks, vector<TEveRecTrackF>& recTracks, Int_t event) {
    fVSDFile->cd();
    ostringstream se;
    se << "Event" << setw(4) << setfill('0') << event;
    ///< Creating the "holding" tree directory
    fVSDFile->mkdir(se.str().c_str());
    fVSDFile->cd(se.str().c_str());
    ///< Creating, filling and writing the "hits" TTree
    //=====================
    TTree *treeH = new TTree("Hits","Hits");
    TEveHit HIT;
    treeH->Branch("H",&HIT);
    for(UInt_t ih=0; ih<hits.size(); ih++) {
        HIT = hits[ih];
        treeH->Fill();
    }
    treeH->Write();
    ///< Creating, filling and writing the "MC tracks" TTree
    //=====================
    TTree *treeMCT = new TTree("Kinematics","Kinematics");
    TEveMCTrack MCTRACK;
    treeMCT->Branch("K",&MCTRACK);
    for(UInt_t it=0; it<mcTracks.size(); it++) {
        MCTRACK = mcTracks[it];
        treeMCT->Fill();
    }
    treeMCT->Write();
    ///< Creating, filling and writing the "reconstructed tracks" TTree
    //=====================
    TTree *treeRecT = new TTree("RecTracks","RecTracks");
    TEveRecTrackF RECTRACK;
    treeRecT->Branch("R",&RECTRACK);
    for(UInt_t it=0; it<recTracks.size(); it++) {
        RECTRACK = recTracks[it];
        treeRecT->Fill();
    }
    treeRecT->Write();
}

///< Creating/Opening the VSD data output file 
//=====================
void a2mcToVSD::CreateOutputFile() {
///< Create the VSD output file
    std::ostringstream fileName;
    fileName << "./root/a2mcVSD_" << fRunNumber << ".root";
    fVSDFile = new TFile(fileName.str().c_str(), "recreate");
    std::cout << "Creating " << fileName.str() << " output file" << std::endl;
}

///< Loading a "a2mc hit" into a TEveHit
//=====================
TEveHit a2mcToVSD::HitToEveHit(UInt_t ih) {
    TEveHit eveHit;
    eveHit.fLabel       = SilHits_fTrackID[ih];
    eveHit.fEvaLabel    = SilHits_fPdgCode[ih];
    ///< Coding hit info into detId and subdetId
    eveHit.fDetId       = 1000*SilHits_fLayN[ih]+SilHits_fnStrp[ih]+1;    ///< DetId    = LayN*1000+n-strip+1
    eveHit.fSubdetId    = 1000*SilHits_fModN[ih]+SilHits_fpStrp[ih]+1;    ///< SubdetId = ModN*1000+p-strip+1
    eveHit.fV.Set(SilHits_fPosX[ih], SilHits_fPosY[ih], SilHits_fPosZ[ih]);
    return eveHit;
}

///< Loading primary origin into a TEveHit
//=====================
TEveHit a2mcToVSD::PrimaryOriginToEveHit() {
    TEveHit eveHit;
    eveHit.fLabel       = 0;
    eveHit.fEvaLabel    = fPdgCode;
    eveHit.fDetId       = 0;    ///< DetId     = 0 for the primary particle origin
    eveHit.fSubdetId    = 0;    ///< SubdetId  = 0
    eveHit.fV.Set(fVox, fVoy, fVoz);
    return eveHit;
}

///< Loading primary decay into a TEveHit
//=====================
TEveHit a2mcToVSD::PrimaryDecayToEveHit() {
    TEveHit eveHit;
    eveHit.fLabel       = 0;
    eveHit.fEvaLabel    = fPdgCode;
    eveHit.fDetId       = 0;    ///< DetId     = 0 for the primary particle decay
    eveHit.fSubdetId    = 1;    ///< SubdetId  = 0
    eveHit.fV.Set(fVdx, fVdy, fVdz);
    return eveHit;
}

///< Loading a "a2mc MC track" into a TEveRecTrackF
//=====================
TEveRecTrackF a2mcToVSD::ToEveRecTrack(UInt_t it) {
//        This function can be used to fill the "EVE reconstructed tracks" [NOT AVAILABLE AT THE MOMENT]
    TEveRecTrackF eveRecTrack;
///< ...
//    eveRecTrack.fIndex = it;
//    eveRecTrack.fLabel = MCTracks_fPdgCode[it];
//    eveRecTrack.fV.Set(MCTracks_fVx[it],MCTracks_fVy[it],MCTracks_fVz[it]);
//    eveRecTrack.fP.Set(MCTracks_fPx[it],MCTracks_fPy[it],MCTracks_fPz[it]);
    return eveRecTrack;
}

///< Loading a "a2mc MC track" into a ToEveMCTrack
//=====================
TEveMCTrack a2mcToVSD::ToEveMCTrack(UInt_t it) {
    TEveMCTrack eveMCTrack;
    eveMCTrack.fIndex = fEvent;
    eveMCTrack.fLabel = it;
    eveMCTrack.SetPdgCode(MCTracks_fPdgCode[it]);
    eveMCTrack.SetProductionVertex(MCTracks_fVx[it],MCTracks_fVy[it],MCTracks_fVz[it],0.);
    eveMCTrack.SetMomentum(MCTracks_fPx[it],MCTracks_fPy[it],MCTracks_fPz[it],MCTracks_fE[it]);
    if(MCTracks_fMother[it][0]==-1) { ///< This track is the "primary particle"
//    eveMCTrack --> fVDecay (decay vertex)
        if(!isnan(fVdx)&&!isnan(fVdy)&&!isnan(fVdz)) {
            eveMCTrack.fDecayed = true;
            eveMCTrack.fVDecay.Set(fVdx, fVdy, fVdz);
        }
    }
    return eveMCTrack;
}

///< Loading a "a2mc MC track" into a ToEveMCTrack
//=====================
Bool_t a2mcToVSD::GoodTrack(UInt_t it) {
///< _________________________________________________________________
///< This function select the tracks to write into the VSD output file

    ///< Only charged tracks that released a hit
    TParticlePDG* particlePDG = pdgDB->GetParticle(MCTracks_fPdgCode[it]);
    if(particlePDG) {
        if(particlePDG->Charge()==0) return false; ///< Do not add "neutral" track
    }
    ///< Always store the track of the primary
    if(MCTracks_fMother[it][0]==-1) return true; ///< This track is the "primary particle"
    ///< All tracks
    if(!tracks_through_det) return true;
    ///< Only tracks with hits in the detector
    for(UInt_t ih=0; ih<SilHits_; ih++) {
        if(SilHits_fTrackID[ih]==it) return true; ///< Only tracks that released a hit in the detectors
    }
    return false;
}
