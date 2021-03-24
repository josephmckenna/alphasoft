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
// hit_threshold, nMinHits
///< Writing the VSD data file (from the MC output data file to the VSD data format)
//=====================
void a2mcToVSD::WriteVSD(Double_t hit_threshold = 0., Int_t nMinHits=0) {
    if (fChain == 0) return;

///< Loop on the events -> select the hits and the MC tracks to write to 
    Int_t nOutEvents = 0;
    cout << "Number of events " << fTotEvents << endl;

    for (fEvent=0; fEvent<fTotEvents; fEvent++) {
        ResetVariables();
        if (LoadTree(fEvent) < 0) break;
        if(FillVSDEvent(fEvent, hit_threshold, nMinHits)) nOutEvents++;
        if(nOutEvents>1000) {
            cout << "Limiting visualization to 1000 events " << endl;
            break;
        }
    }
    fVSDFile->Close();
    cout << "Writing " << nOutEvents << " \'good\' events in the VSD output file" << endl;
}

///< Fill the VSD Event 

Bool_t a2mcToVSD::FillVSDEvent(Int_t fEvent, Double_t hit_threshold, Int_t nMinHits) {
        ///< Loading the hits into a local vector<TEveHit>
        vector<TEveHit> hits;
        for(UInt_t ih=0; ih<SilHits_; ih++) {
            if(SilHits_fEdep[ih]*1000.<hit_threshold) continue; ///< Multiplying by 1000. (from GeV -> MeV as for the cut)
            hits.push_back(HitToEveHit(ih)); ///< Writing hits
        }
        if((Int_t)hits.size()<nMinHits) return false; ///< Not enough "good hits" (above threshold) to be shown        
        ///< Loading the tracks into a local vector<TEveMCTrack> [filtering with GoodMCTrack()]
        vector<TEveMCTrack> tracksMC;
        for(UInt_t it=0; it<MCTracks_; it++) {
            if(GoodMCTrack(it)) tracksMC.push_back(ToEveMCTrack(it));
        }
        ///< Primary particle: origin and decay vertex
        if(!isnan(fVox)&&abs(fPdgCode)==13) hits.push_back(PrimaryOriginToEveHit()); ///< Origin only for muons
        if(!isnan(fVdx)) hits.push_back(PrimaryDecayToEveHit());

///< ########## RECONSTRUCT #############
        ///< Resetting fAlphaEvent and filling it with the hits of this event
        vector<TEveRecTrackF> tracksRec;
        fAlphaEvent->DeleteEvent(); ///< Resetting fAlphaEvent
        if(FillAlphaEvent(hit_threshold)) {
            fAlphaEvent->RecEvent();
            ///< Loading the tracks into a local vector<TEveRecTrackF>
            for(UInt_t it=0; it<fAlphaEvent->GetNTracks(); it++) {
                TVector3 pos = fAlphaEvent->GetTrack(it)->Getr0();
                TVector3 dir = fAlphaEvent->GetTrack(it)->Getunitvector();
                if(pos.Mag()!=0&&dir.Mag()!=0) {
                    tracksRec.push_back(ToEveRecTrack(it));
                } else {
                    cout << "a2mcToVSD::FillVSDEvent ==> Event (" << fEvent << ") Rec Track (" << it << ") with pos.Mag() == 0 && dir.Mag() == 0" << endl;
                }
            }
            ///< Extracting the information of the reconstructed vertex
            TAlphaEventVertex *vertex = fAlphaEvent->GetVertex();
            if(vertex!=NULL&&vertex->IsGood()) {
                fRecVdx = vertex->X();
                fRecVdy = vertex->Y();
                fRecVdz = vertex->Z();
                hits.push_back(RecoVertexToEveHit());
            }
        }
        ///< Writing the data in the VSD structure
        WriteVSDEvent(hits, tracksMC, tracksRec, fEvent);
        return true;
}

///< Writing the "Events" into the VSD data file 
//=====================
void a2mcToVSD::WriteVSDEvent(vector<TEveHit>& hits, vector<TEveMCTrack>& mcTracks, vector<TEveRecTrackF>& recTracks, Int_t event) {
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
    eveHit.fDetId       = 1000*SilHits_fLayN[ih]+SilHits_fnStrp[ih]+1;    ///< DetId    = LayN*1000+n_strip+1
    eveHit.fSubdetId    = 1000*SilHits_fModN[ih]+SilHits_fpStrp[ih]+1;    ///< SubdetId = ModN*1000+p_strip+1
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


///< Loading reconstructed vertex into a TEveHit
//=====================
TEveHit a2mcToVSD::RecoVertexToEveHit() {
    TEveHit eveHit;
    eveHit.fLabel       = 1;
    eveHit.fEvaLabel    = 0;
    eveHit.fDetId       = 0;    ///< DetId     = 0 for the primary particle decay
    eveHit.fSubdetId    = 2;    ///< SubdetId  = 0
    eveHit.fV.Set(fRecVdx, fRecVdy, fRecVdz);
    return eveHit;
}

///< Loading a "a2mc MC track" into a ToEveMCTrack
//=====================
TEveMCTrack a2mcToVSD::ToEveMCTrack(UInt_t it) {
    TEveMCTrack eveMCTrack;
    eveMCTrack.fIndex = 0;
    eveMCTrack.fLabel = it;
    eveMCTrack.fEvaLabel = MCTracks_fPdgCode[it];
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

///< Loading a "reconstructed track" into a TEveRecTrackF
//=====================
TEveRecTrackF a2mcToVSD::ToEveRecTrack(UInt_t it) {
//        This function can be used to fill the "EVE reconstructed tracks" [NOT AVAILABLE AT THE MOMENT]
    TEveRecTrackF eveRecTrack;
///< ...
    TVector3 pos = fAlphaEvent->GetTrack(it)->Getr0();
    TVector3 dir = fAlphaEvent->GetTrack(it)->Getunitvector();
    TVector3 Vo = RecTrackVo(pos,dir);
    eveRecTrack.fIndex = it;
    eveRecTrack.fLabel = 1;
    eveRecTrack.fV.Set(Vo.X(),Vo.Y(),Vo.Z());
    eveRecTrack.fP.Set(dir.X(),dir.Y(),dir.Z());
    return eveRecTrack;
}

///< Selecting "good tracks"
//=====================
Bool_t a2mcToVSD::GoodMCTrack(UInt_t it) {
///< _________________________________________________________________
///< This function select the tracks to write into the VSD output file

    ///< Only charged tracks that released a hit
    TParticlePDG* particlePDG = pdgDB->GetParticle(MCTracks_fPdgCode[it]);
    if(particlePDG) {
        if(particlePDG->Charge()==0) return false; ///< Do not add "neutral" track
    }
    ///< Always store the track of the primary in case of muons
    ///< Never store the track of the primary in case of antiprotons
    if(MCTracks_fMother[it][0]==-1&&MCTracks_fPdgCode[it]==-2212) return false;
    if(MCTracks_fMother[it][0]==-1&&abs(MCTracks_fPdgCode[it])==13) return true;
    ///< Only tracks with hits in the detector
    for(UInt_t ih=0; ih<SilHits_; ih++) {
        if(SilHits_fTrackID[ih]==it) return true; ///< Only tracks that released a hit in the detectors
    }
    return false;
}

Bool_t a2mcToVSD::FillAlphaEvent(Double_t hit_threshold = 0.) {
    Int_t nGoodHits = 0;
    for(UInt_t ih=0; ih<SilHits_; ih++) { ///< Loop over the silicon hits
        if(SilHits_fEdep[ih]*1000.<hit_threshold) continue; ///< Multiplying by 1000. (from GeV -> MeV as for the cut)
        Int_t nstrp = SilHits_fnStrp[ih]; /// hit n-strip
        Int_t pstrp = SilHits_fpStrp[ih]; /// hit p-strip
        Double_t q = SilHits_fEdep[ih]*1.e4;
        Int_t hybrid_number=SilHits_fSilID[ih];
        if(hybrid_number < 0 || hybrid_number > 71)
            cerr<<"a2mcReco::RecEvent  Hybrid "<<hybrid_number <<" doesn't exist!!! PANIC NOW!"<<endl;
        TAlphaEventSil* sil = fAlphaEvent->GetSilByNumber(hybrid_number);
        if(!sil) {
            sil = new TAlphaEventSil(hybrid_number, fAlphaEvent, fAlphaEventMap);
            fAlphaEvent->AddSil(sil);
        }
        Double_t* nside=0;
        Double_t* nrms=0;
        if(nstrp>=0 && nstrp<128) {
            nside=sil->GetASIC1();
            nside[nstrp]+=q;
            nrms=sil->GetRMS1();
            nrms[nstrp]=1000.; // To Be Reviewed
        }
        else if(nstrp>=128 && nstrp<256) {
            nside=sil->GetASIC2();
            nside[nstrp-128]+=q;
            nrms=sil->GetRMS2();
            nrms[nstrp-128]=1000.; // To Be Reviewed
        }
        Double_t* pside=0;
        Double_t* prms=0;
        if(pstrp>=0 && pstrp<128) {
            pside=sil->GetASIC3();
            pside[pstrp]+=q;
            prms=sil->GetRMS3();
            pside[pstrp]=1000.; 
        } else if(pstrp>=128 && pstrp<256) {
            pside=sil->GetASIC4();
            pside[pstrp-128]+=q;
            prms=sil->GetRMS4();
            pside[pstrp-128]=1000.;
        }
        if(nside && pside) nGoodHits++;
    } ///< End of loop over the silicon hits
    if(nGoodHits>=3) return true;
    return false;
}

//_____________________________________________________________________________
TVector3 a2mcToVSD::RecTrackVo(TVector3 P0, TVector3 Dir) {
    Double_t R = 34.9; ///< limits for track visualization
    Double_t H = 69.9; ///< limits for track visualization
    ///< This method check if the generated muon "crosses" a cylinder with radius R and heigth H
    // Z coordinate is along the H
    //  (y)| 
    //     |  / (x)
    //     | /
    //     |/_________ (z)
    ///< Line P = P0 + Dir*t [P = (X, Y, Z), P0 = (X0, Y0, Z0), Dir = (cx, cy, cz) or (px, py, pz)]
    ///< 
    ///< For formulas [see my Evernote note "Intersezione Linea-Cilindro (Retta/Cilindro)"]
    ///< Find free parameter t, if any (interection of a line with a circle) - checking in the  X, Y plane
    Double_t a = Dir.X()*Dir.X()+ Dir.Y()*Dir.Y();
    Double_t b = 2*(Dir.X()*P0.X()+Dir.Y()*P0.Y());
    Double_t c = P0.X()*P0.X() + P0.Y()*P0.Y() - R*R;
    
    ///< Finding the two intercepts with the lateral surface
    Double_t det = b*b-4*a*c;
    if(det>=0) { ///< Two intercepts 
        Double_t t1 = (-b - sqrt(det))/(2.*a);
        Double_t Z1 = P0.Z() + t1*Dir.Z();
        TVector3 P1 = P0 + t1*Dir;
        if(fabs(Z1)<=H/2.&&InWard(P1,Dir)) {
            return P1; ///< Intersection within cylinder and pointin inward
        }
        Double_t t2 = (-b + sqrt(det))/(2.*a);
        Double_t Z2 = P0.Z() + t2*Dir.Z();
        TVector3 P2 = P0 + t2*Dir;
        if(fabs(Z2)<=H/2.&&InWard(P2,Dir)) {
            return P2; ///< Intersection within cylinder and pointin inward
        }
    }
    ///< Finding the two intercepts with the bases of the cylinder
    ///< Find free parameter t, extrapolating to the bases of the cylinder
    if(Dir.Z()!=0.) {
        Double_t t1 = (H/2.-P0.Z())/Dir.Z();
        TVector3 P1 = P0 + t1*Dir;
        if((sqrt(P1.X()*P1.X()+P1.Y()*P1.Y())<R)&&InWard(P1,Dir)) {
            return P1; ///< Intersection within cylinder and pointin inward
        }
        Double_t t2 = (-H/2.-P0.Z())/Dir.Z();
        TVector3 P2 = P0 + t2*Dir;
        if((sqrt(P2.X()*P2.X()+P2.Y()*P2.Y())<R)&&InWard(P2,Dir)) {
            return P2; ///< Intersection within cylinder and pointin inward
        }
    }
    // cout << "a2mcToVSD::RecTrackVo ==> Something fishy: reconstucted track is not intercepting a cylinder around the detector " <<  endl;
    // cout << sqrt(P0.X()*P0.X()+P0.Y()*P0.Y()) << " | " << P0.Z() << endl;
    // cout << Dir.X() << " | " << Dir.Y() << " | " << Dir.Z() << endl;
    return  P0;
}

Bool_t a2mcToVSD::InWard(TVector3 P0, TVector3 Dir) { ///< Ro = point, Po = direction
    Double_t s = P0.X()*Dir.X() + P0.Y()*Dir.Y();
    if(s>0) return false;
    return true;
}

