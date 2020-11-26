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
    LoadMCTracks();
//    LoadRecTracks();

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
void a2mcVSDReader::LoadSilHits(TEvePointSet*& ps, const TString& det_name) {
    if (ps == nullptr) {
        ps = new TEvePointSet(det_name);
        ps->SetMainColor((Color_t)(kRed));
        ps->SetMarkerSize(1.5);
        ps->SetMarkerStyle(20);
        ps->IncDenyDestroy();
    } else {
        ps->Reset();
    }
    TEvePointSelector ss(fVSD->fTreeH, ps, "fV.fX:fV.fY:fV.fZ", TString::Format("fDetId==0||fDetId==1"));
    ss.Select();
    ps->SetTitle(TString::Format("N=%d", ps->Size()));
    gEve->AddElement(ps);
}
//---------------------------------------------------------------------------
// Track loading
//---------------------------------------------------------------------------
///< MC Tracks
void a2mcVSDReader::LoadMCTracks() {
//       Read reconstructed tracks from current event.

    if (fTrackListMC == 0) {
        fTrackListMC = new TEveTrackList("MC Tracks");
        fTrackListMC->SetMainColor(6);
        fTrackListMC->SetMarkerColor(kOrange);
        fTrackListMC->SetMarkerStyle(4);
        fTrackListMC->SetMarkerSize(0.5);
        fTrackListMC->IncDenyDestroy();
    } else {
        fTrackListMC->DestroyElements();
    }

    auto trkProp = fTrackListMC->GetPropagator();
    // !!!! Need to store field on file !!!!
    // Can store TEveMagField ?
//    trkProp->SetMagField(0.5);
    trkProp->SetMagField(0.); ///< No magnetic field here
    trkProp->SetStepper(TEveTrackPropagator::kRungeKutta);

    Double_t naan = std::numeric_limits<double>::quiet_NaN();
    Double_t Vx = naan, Vy = naan, Vz = naan;
    Int_t nTracks = fVSD->fTreeK->GetEntries();
    for (Int_t n = 0; n < nTracks; n++) {
        fVSD->fTreeK->GetEntry(n);
        if(fVSD->fK.fLabel==0) { ///< Primary particle -> origin
            Vx = fVSD->fK.Vx();
            Vy = fVSD->fK.Vy();
            Vz = fVSD->fK.Vz();
        }
        TEveTrack* track = new TEveTrack(&fVSD->fK, trkProp);
        track->SetName(Form("MC Track %d", fVSD->fK.fLabel));
        track->SetStdTitle();
        track->SetAttLineAttMarker(fTrackListMC);
        fTrackListMC->AddElement(track);
    }

    trkProp->SetMaxR(36.35); ///< Based on the Oxford magnet dimension and generation sphere
    trkProp->SetMaxZ(32.);   ///< Based on the Oxford magnet dimension and generation sphere
    fTrackListMC->MakeTracks();

    gEve->AddElement(fTrackListMC);
    if(!std::isnan(Vx)) AddOrigin(Vx, Vy, Vz); ///< Adding a point for the origin
}

///< Rec Tracks
void a2mcVSDReader::LoadRecTracks() {
//       Read reconstructed tracks from current event.

    if (fTrackListRec == 0) {
        fTrackListRec = new TEveTrackList("Rec Tracks");
        fTrackListRec->SetMainColor(6);
        fTrackListRec->SetMarkerColor(kYellow);
        fTrackListRec->SetMarkerStyle(4);
        fTrackListRec->SetMarkerSize(0.5);
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

void a2mcVSDReader::AddOrigin(double& Vx, double& Vy, double& Vz) {
    ///< ############################################
    ///< Adding a point for the origin of the muon
    ///< ############################################
    ///< Add the Vertex of the Primary particle
    if (fVertices == nullptr) {
        fVertices = new TEvePointSet("Origin");
        fVertices->SetMainColor((Color_t)(kBlue));
        fVertices->SetMarkerSize(2.5);
        fVertices->SetMarkerStyle(29);
        fVertices->IncDenyDestroy();
//        fVertices->SetPointId(new TNamed(Form("Origin"), "");
        fVertices->SetTitle(TString::Format("Origin"));
        fVertices->SetNextPoint(Vx, Vy, Vz);
    } else {
        fVertices->Reset();
        fVertices->SetNextPoint(Vx, Vy, Vz);
    }
    gEve->AddElement(fVertices);
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
    DumpMCTracks();
    DumpHits();
}

void a2mcVSDReader::DumpHits() {
    if(fVSD->fTreeH->GetEntriesFast()==0) return;
    cout << "Number of detector   hits " << fSilHits->Size() << endl;
    cout << "\t (lay/mod/half)" << " | " << "track (id, name)" << endl;
    for(UInt_t i=0; i<fVSD->fTreeH->GetEntriesFast(); i++) {
        fVSD->fTreeH->GetEntry(i); ///< load a TEveHit into fVSD->fH
        DumpHit();
    }
}

void a2mcVSDReader::DumpHit() {
    Int_t det, subdet, half, lay, mod;
    string track_particle_name("other");
    TParticlePDG* particlePDG = TDatabasePDG::Instance()->GetParticle(fVSD->fH.fEvaLabel);
    if(particlePDG) track_particle_name = (string)particlePDG->GetName();
    det = fVSD->fH.fDetId;
    if(det == 0) {
        subdet = fVSD->fH.fSubdetId;
        half = (int)((double)(subdet)/1000.);
        subdet -= half*1000;
        lay = (int)((double)(subdet)/100.);
        subdet -= lay*100;
        mod = subdet;
        cout << "\t o) " << lay << " | " << mod << " | " << setw(2) << setfill(' ') << half << " | ";
        cout << "[" << left << setw(3) << fVSD->fH.fLabel << ", " << track_particle_name.c_str() << "]";
        cout << endl;    
    }
}

void a2mcVSDReader::DumpMCTracks() {
    Int_t nMCTracks = fVSD->fTreeK->GetEntriesFast();
    if(nMCTracks==0) return;
    cout << "Number of Tracks " << nMCTracks << endl;
    cout << "\t track (id, name)" << " | " << "origin (x,y,z)" << endl;
    for(UInt_t i=0; i<nMCTracks; i++) {
        fVSD->fTreeK->GetEntry(i); ///< load a TEveHit into fVSD->fH
        DumpMCTrack();
    }
}

void a2mcVSDReader::DumpMCTrack() {
    ostringstream track_particle_name;
    TParticlePDG* particlePDG = TDatabasePDG::Instance()->GetParticle(fVSD->fK.GetPdgCode());
    if(particlePDG) track_particle_name << (string)particlePDG->GetName(); else track_particle_name << fVSD->fK.GetPdgCode();
    Double_t ptot = sqrt(fVSD->fK.Px()*fVSD->fK.Px() + fVSD->fK.Py()*fVSD->fK.Py() + fVSD->fK.Pz()*fVSD->fK.Pz());

    cout << "\t -) " << left << setw(3) << fVSD->fK.fLabel << "| " << track_particle_name.str().c_str() << " | ";
    cout << "[" << fixed << setprecision(2) << fVSD->fK.Vx() << ", " << fVSD->fK.Vy() << ", " << fVSD->fK.Vz() << "] " << " | ";
    if(ptot<=0.001) cout << "Ptot = " << fixed << setprecision(2) << ptot*1.e6 << " KeV/c" << endl;
    if(0.001<ptot&&ptot<=0.1) cout << "Ptot = " << fixed << setprecision(2) << ptot*1.e3 << " MeV/c" << endl;
    if(ptot>0.1) cout << "Ptot = " << fixed << setprecision(2) << ptot << " GeV/c" << endl;
    cout << endl;

}
