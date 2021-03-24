///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################
///< This class is a modification of the code contained in $ROOTSYS/tutorials/eve/alice_vsd.C
///< properly modified for a2mc

#define a2mcVSDReader_cxx
#include "a2mcVSDReader.h"

void a2mcVSDReader::AttachEvent() {
  // Attach event data from current directory.
  fVSD->LoadTrees();
  fVSD->SetBranchAddresses();
}

void a2mcVSDReader::DropEvent() {
   // Drup currently held event data, release current directory.

    // Drop old visualization structures.

    gEve->GetViewers()->DeleteAnnotations();
    gEve->GetCurrentEvent()->DestroyElements();

    // Drop old event-data.

    fVSD->DeleteTrees();
    delete fDirectory;
    fDirectory = 0;
}

//---------------------------------------------------------------------------
// Event navigation (NextEvent, PrevEvent, GotoEvent)
//---------------------------------------------------------------------------

void a2mcVSDReader::NextEvent() {
    GotoEvent(fCurEv + 1);
}

void a2mcVSDReader::PrevEvent() {
  GotoEvent(fCurEv - 1);
}

Bool_t a2mcVSDReader::GotoEvent() {
    return GotoEvent(fCurEv);
}

Bool_t a2mcVSDReader::GotoEvent(Int_t ev) {
    if (ev < 0 || ev >= fMaxEv) {
       Warning("GotoEvent", "Invalid event id %d.", ev);
       return kFALSE;
    }

    DropEvent();

    // Connect to new event-data.

    fCurEv = ev;
    fDirectory = (TDirectory*) ((TKey*) fEvDirKeys->At(fCurEv))->ReadObj();
    fVSD->SetDirectory(fDirectory);
    string s = (string)fDirectory->GetName();
    fEventMC = DirNameToEventN(s);
    AttachEvent();

    // Load event data into visualization structures
    LoadSilHits(fSilHits, "Hits");
    LoadPrimaryOriginHit(fPrimOriginHit, "PrimaryOrigin");
    LoadPrimaryDecayHit(fPrimDecayHit, "PrimaryDecay");
    LoadRecoVertexHit(fRecoVertexHit, "RecoVertex");
    LoadMCTracks();
    LoadRecTracks(); 

    // Fill projected views.
    auto top = gEve->GetCurrentEvent();

    gMultiView->DestroyEventRPhi();
    gMultiView->ImportEventRPhi(top);

    gMultiView->DestroyEventRhoZ();
    gMultiView->ImportEventRhoZ(top);

    gEve->Redraw3D(kFALSE, kTRUE);

///< Dump event info on screen
   DumpEvent();
   return kTRUE;
}

//---------------------------------------------------------------------------
// Hit loading
//---------------------------------------------------------------------------
///< SilHits
void a2mcVSDReader::LoadSilHits(TEvePointSet*& hits, const TString& det_name) {
    if (hits == nullptr) {
        hits = new TEvePointSet(det_name);
        hits->SetMainColor((Color_t)(kRed));
        hits->SetMarkerSize(2.5);
        hits->SetMarkerStyle(20);
        hits->IncDenyDestroy();
    } else {
        hits->Reset();
    }
    ///< The DetId is > 0 [see a2mcToVSD for encoding]
    TEvePointSelector ss(fVSD->fTreeH, hits, "fV.fX:fV.fY:fV.fZ", TString::Format("fDetId>0"));
    ss.Select();
    hits->SetTitle(TString::Format("N=%d", hits->Size()));
    gEve->AddElement(hits);
}

///< PrimaryOriginHit
void a2mcVSDReader::LoadPrimaryOriginHit(TEvePointSet*& hits, const TString& det_name) {
    if (hits == nullptr) {
        hits = new TEvePointSet(det_name);
        hits->SetMainColor((Color_t)(kRed));
        hits->SetMarkerSize(3.0);
        hits->SetMarkerStyle(47);
        hits->IncDenyDestroy();
    } else {
        hits->Reset();
    }
    ///< The DetId is 0, SubdetId = 0
    TEvePointSelector ss(fVSD->fTreeH, hits, "fV.fX:fV.fY:fV.fZ", TString::Format("fDetId==0&&fSubdetId==0"));
    ss.Select();
    hits->SetTitle(TString::Format("N=%d", hits->Size()));
    gEve->AddElement(hits);
}

///< PrimaryDecayHit
void a2mcVSDReader::LoadPrimaryDecayHit(TEvePointSet*& hits, const TString& det_name) {
    if (hits == nullptr) {
        hits = new TEvePointSet(det_name);
        hits->SetMainColor((Color_t)(kOrange));
        hits->SetMarkerSize(3.0);
        hits->SetMarkerStyle(kFullStar);
        hits->IncDenyDestroy();
    } else {
        hits->Reset();
    }
    ///< The DetId is 0, SubdetId = 1
    TEvePointSelector ss(fVSD->fTreeH, hits, "fV.fX:fV.fY:fV.fZ", TString::Format("fDetId==0&&fSubdetId==1"));
    ss.Select();
    hits->SetTitle(TString::Format("N=%d", hits->Size()));
    gEve->AddElement(hits);
}

///< PrimaryDecayHit
void a2mcVSDReader::LoadRecoVertexHit(TEvePointSet*& hits, const TString& det_name) {
    if (hits == nullptr) {
        hits = new TEvePointSet(det_name);
        hits->SetMainColor((Color_t)(kBlue));
        hits->SetMarkerSize(3.0);
        hits->SetMarkerStyle(kFullStar);
        hits->IncDenyDestroy();
    } else {
        hits->Reset();
    }
    ///< The DetId is 0, SubdetId = 2
    TEvePointSelector ss(fVSD->fTreeH, hits, "fV.fX:fV.fY:fV.fZ", TString::Format("fDetId==0&&fSubdetId==2"));
    ss.Select();
    hits->SetTitle(TString::Format("N=%d", hits->Size()));
    gEve->AddElement(hits);
}

//---------------------------------------------------------------------------
// Track loading
//---------------------------------------------------------------------------
///< MC Tracks
void a2mcVSDReader::LoadMCTracks() {
//       Read reconstructed tracks from current event.

    if (fTrackListMC == 0) {
        fTrackListMC = new TEveTrackList("MC Tracks");
        fTrackListMC->SetMainColor(8);
        fTrackListMC->SetMarkerColor(32);
        fTrackListMC->SetMarkerStyle(4);
        fTrackListMC->SetMarkerSize(0.5);
        fTrackListMC->SetLineWidth(3);
        fTrackListMC->IncDenyDestroy();
    } else {
        fTrackListMC->DestroyElements();
    }

    auto trkProp = fTrackListMC->GetPropagator();
    // !!!! Need to store field on file !!!!
    // Can store TEveMagField ?
    // trkProp->SetMagField(0., 0., 0.3); ///< in T (???)
    trkProp->SetMagField(0.); ///< No magnetic field here
    trkProp->SetStepper(TEveTrackPropagator::kRungeKutta);
    trkProp->SetMaxR(35.);  ///< Based on the Oxford magnet dimension and generation sphere
    trkProp->SetMaxZ(35.);  ///< Based on the Oxford magnet dimension and generation sphere
    trkProp->SetFitDecay(true); ///< Is it working?

    Double_t naan = std::numeric_limits<double>::quiet_NaN();
    Int_t nTracks = fVSD->fTreeK->GetEntries();
    for (Int_t n = 0; n < nTracks; n++) {
        fVSD->fTreeK->GetEntry(n);
        TEveTrack* track = new TEveTrack(&fVSD->fK, trkProp);
        track->SetName(Form("MC Track %d", fVSD->fK.fLabel));
        track->SetStdTitle();
        track->SetAttLineAttMarker(fTrackListMC);
        fTrackListMC->AddElement(track);
    }

    fTrackListMC->MakeTracks();

    gEve->AddElement(fTrackListMC);
}

///< Rec Tracks
void a2mcVSDReader::LoadRecTracks() {
//       Read reconstructed tracks from current event.

    if (fTrackListRec == 0) {
        fTrackListRec = new TEveTrackList("Rec Tracks");
        fTrackListRec->SetMainColor(9);
        fTrackListRec->SetMarkerColor(38);
        fTrackListRec->SetMarkerStyle(3);
        fTrackListRec->SetMarkerSize(0.5);
        fTrackListRec->SetLineWidth(5);
        fTrackListRec->IncDenyDestroy();
    } else {
        fTrackListRec->DestroyElements();
    }

    auto trkProp = fTrackListRec->GetPropagator();
    // !!!! Need to store field on file !!!!
    // Can store TEveMagField ?
//    trkProp->SetMagField(0.5);
    trkProp->SetMagField(0.); ///< No magnetic field here
    trkProp->SetStepper(TEveTrackPropagator::kRungeKutta);
    trkProp->SetMaxR(35.);  ///< Based on the Oxford magnet dimension and generation sphere
    trkProp->SetMaxZ(35.);  ///< Based on the Oxford magnet dimension and generation sphere

    Int_t nTracks = fVSD->fTreeR->GetEntries();
    for (Int_t n = 0; n < nTracks; n++) {
        fVSD->fTreeR->GetEntry(n);
        TEveTrack* track = new TEveTrack(&fVSD->fR, trkProp);
        track->SetName(Form("Rec Track %d", fVSD->fR.fIndex));
        track->SetStdTitle();
        track->SetAttLineAttMarker(fTrackListRec);
        fTrackListRec->AddElement(track);
    }

    fTrackListRec->MakeTracks();

    gEve->AddElement(fTrackListRec);
}

////---------------------------------------------------------------------------
//// Cluster loading
////---------------------------------------------------------------------------
//void a2mcVSDReader::LoadClusters(TEvePointSet*& ps, const TString& det_name, Int_t det_id) {
//    if (ps == 0) {
//        ps = new TEvePointSet(det_name);
//        ps->SetMainColor((Color_t)(det_id + 2));
//        ps->SetMarkerSize(0.5);
//        ps->SetMarkerStyle(2);
//        ps->IncDenyDestroy();
//    } else {
//        ps->Reset();
//    }
//    
//    TEvePointSelector ss(fVSD->fTreeC, ps, "fV.fX:fV.fY:fV.fZ", TString::Format("fDetId==%d", det_id));
//    ss.Select();
//    ps->SetTitle(TString::Format("N=%d", ps->Size()));
//    gEve->AddElement(ps);
//}

///< ###################################################################
///< Dump (on screen) utilities
///< ###################################################################
void a2mcVSDReader::DumpEvent() {
    cout << "############################################# " << endl;
    cout << "                  MC Event " << setw(4) << setfill(' ') << fEventMC << endl;
    cout << "############################################# " << endl;
    DumpVertices();
    DumpMCTracks();
    DumpRecTracks();
    DumpSilHits();
}

void a2mcVSDReader::DumpVertices() {
    if(fVSD->fTreeH->GetEntriesFast()==0) return;
    for(UInt_t i=0; i<fVSD->fTreeH->GetEntriesFast(); i++) {
        fVSD->fTreeH->GetEntry(i); ///< load a TEveHit into fVSD->fH
        if(fVSD->fH.fDetId==0&&fVSD->fH.fSubdetId==1) { ///< MC antiproton decay vertex
            cout << "Antiproton MC decay vertex (R | Z) [cm]: " << fixed << setprecision(2) 
                 << "(" << fVSD->fH.fV.R() << " | " << fVSD->fH.fV.fZ << ")" << endl;
        }
        if(fVSD->fH.fDetId==0&&fVSD->fH.fSubdetId==2) { ///< reco vertex
            cout << "      Reconstructed vertex (R | Z) [cm]: " << fixed << setprecision(2) 
                 << "(" << fVSD->fH.fV.R() << " | " << fVSD->fH.fV.fZ << ")" << endl;
        }
    }

}

void a2mcVSDReader::DumpSilHits() {
    if(fVSD->fTreeH->GetEntriesFast()==0) return;
    cout << "Number of detector hits " << fSilHits->Size() << endl;
    if(fVSD->fTreeH->GetEntriesFast()>0) cout << "\t (sil/nstrip/pstrip)" << " | " << "track (id, name)" << endl;
    for(UInt_t i=0; i<fVSD->fTreeH->GetEntriesFast(); i++) {
        fVSD->fTreeH->GetEntry(i); ///< load a TEveHit into fVSD->fH
        if(fVSD->fH.fDetId>0) DumpSilHit(); ///< Only silicon hits
    }
}

void a2mcVSDReader::DumpSilHit() {
    Int_t lay, mod;
    Int_t nStrip, pStrip;
    string track_particle_name("other");
    TParticlePDG* particlePDG = TDatabasePDG::Instance()->GetParticle(fVSD->fH.fEvaLabel);
    if(particlePDG) track_particle_name = (string)particlePDG->GetName();
    ///< Decoding layer and module from DetId and SubdetId
    lay = (int)((double)fVSD->fH.fDetId/1000.);
    mod = (int)((double)fVSD->fH.fSubdetId/1000.);
    if(0<=lay&&lay<=5) { ///< fDetId = layer number [0-5]
        ///< Decoding n-strip and p-strip from SubdetId
        nStrip = fVSD->fH.fDetId - 1000*lay;
        pStrip = fVSD->fH.fSubdetId - 1000*mod;
        ostringstream s;
        s << lay << "si" << std::uppercase << std::hex << mod;
        cout << "\t o) " << s.str() << " [";
        cout << right << setw(3) << setfill('0') << nStrip << ",";
        cout << right << setw(3) << setfill('0') << pStrip << "] | ";
        cout << "(" << left << setw(3) << setfill(' ') << fVSD->fH.fLabel << ", " << track_particle_name.c_str() << ")";
        cout << endl;
    }
}

void a2mcVSDReader::DumpMCTracks() {
    Int_t nMCTracks = fVSD->fTreeK->GetEntriesFast();
    if(nMCTracks==0) return;
    cout << "Number of charged particles " << nMCTracks << " (may be only the ones with a hit in the silicon detector)" << endl;
    cout << "\t particle (id, name)" << " | " << "origin [x,y,z]-[decay if recorded]" << endl;
    for(UInt_t i=0; i<nMCTracks; i++) {
        fVSD->fTreeK->GetEntry(i); ///< load a TEveHit into fVSD->fH
        DumpMCTrack();
    }
}

void a2mcVSDReader::DumpRecTracks() {
    Int_t nRecTracks = fVSD->fTreeR->GetEntriesFast();
    if(nRecTracks==0) return;
    cout << "Number of reconstructed tracks " << nRecTracks << endl;
    // cout << "\t particle (id, name)" << " | " << "origin [x,y,z]-[decay if recorded]" << endl;
    // for(UInt_t i=0; i<nRecTracks; i++) {
    //     fVSD->fTreeR->GetEntry(i); ///< load a TEveHit into fVSD->fH
    //     DumpRecTrack();
    // }
}

void a2mcVSDReader::DumpMCTrack() {
    ostringstream track_particle_name;
    TParticlePDG* particlePDG = TDatabasePDG::Instance()->GetParticle(fVSD->fK.GetPdgCode());
    if(particlePDG) track_particle_name << (string)particlePDG->GetName(); else track_particle_name << fVSD->fK.GetPdgCode();
    Double_t px   = fVSD->fK.Px();
    Double_t py   = fVSD->fK.Py();
    Double_t pz   = fVSD->fK.Pz();
    Double_t ptot = sqrt(px*px + py*py + pz*pz);
    Double_t pt   = sqrt(px*px + py*py);
    cout << "\t -) " << left << setw(3) << fVSD->fK.fLabel << "| " << track_particle_name.str().c_str() << " | ";
    cout << "[" << fixed << setprecision(2) << fVSD->fK.Vx() << ", " << fVSD->fK.Vy() << ", " << fVSD->fK.Vz() << "] | ";
    // if(ptot<=0.001) {
    //     cout << "Ptot = " << fixed << setprecision(2) << ptot*1.e6;
    //     cout << " (Pt = " << fixed << setprecision(2) << pt*1.e6;
    //     cout << " Pz = "  << fixed << setprecision(2) << pz*1.e6 << ") KeV/c";
    // }
    // if(0.001<ptot&&ptot<=0.1) {
        cout << "Ptot = " << fixed << setprecision(2) << ptot*1.e3;
        cout << " (Pt = " << fixed << setprecision(2) << pt*1.e3;
        cout << " Pz = "  << fixed << setprecision(2) << pz*1.e3 << ") MeV/c";
    // }
    // if(ptot>0.1) {
    //     cout << "Ptot = " << fixed << setprecision(2) << ptot;
    //     cout << " (Pt = " << fixed << setprecision(2) << pt;
    //     cout << " Pz = "  << fixed << setprecision(2) << pz << ") GeV/c";
    // }
    if(fVSD->fK.fDecayed) {
        cout << " | [decay R=" << fixed << setprecision(2) << sqrt(fVSD->fK.fVDecay.fX*fVSD->fK.fVDecay.fX+fVSD->fK.fVDecay.fY*fVSD->fK.fVDecay.fY) << ",Z=" << fVSD->fK.fVDecay.fZ << "]";
    }
    cout << endl;
}
