///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "a2mcSilSD.h"
#include "a2mcRootManager.h"

ClassImp(a2mcSilSD)

using namespace std;

//_____________________________________________________________________________
a2mcSilSD::a2mcSilSD(const char* name)
    : TNamed(name, ""),
    fHitCollection(0),
    fDIGICollection(0),
    fVerboseLevel(0)
{
    /// Standard constructor
    /// \param name  The hits collection name

    fHitCollection = new TClonesArray("a2mcSilHit");
    fDIGICollection = new TClonesArray("a2mcSilDIGI");

}

//_____________________________________________________________________________
    a2mcSilSD::a2mcSilSD()
: TNamed(),
    fHitCollection(0),
    fDIGICollection(0),
    fVerboseLevel(0)
{
    /// Default constructor
}

//_____________________________________________________________________________
a2mcSilSD::~a2mcSilSD()
{
    /// Destructor
}

//
// -----------------------------------> PUBLIC FUNCTIONS
//
//_____________________________________________________________________________
void a2mcSilSD::Initialize()
{
    /// Register hits collection in the Root manager;
    /// set sensitive volumes.

        
    Register();
    fSensitiveID.push_back(gMC->VolId("silMod"));

}
//_____________________________________________________________________________
void a2mcSilSD::Register()
{
    /// Register the hits collection in the Root manager.

    a2mcRootManager::Instance()->Register("SilHits", "TClonesArray", &fHitCollection);
    a2mcRootManager::Instance()->Register("SilDigi", "TClonesArray", &fDIGICollection);
}

//_____________________________________________________________________________
void a2mcSilSD::BeginOfEvent()
{
}

//_____________________________________________________________________________
Bool_t a2mcSilSD::ProcessHits()
{
    /// Create hits (in stepping).
    Int_t copyNo;   // copy number of the volume ID
    Int_t volID = gMC->CurrentVolID(copyNo);
    Bool_t isSil=false;
    for(auto& id:fSensitiveID) {
        if(volID==id) isSil = true;
    }
    if (!isSil) return false; // We are not in the silicon detector volume

//    if(!gMC->IsTrackEntering()&&!gMC->IsTrackExiting()) return false; ///< Only "IN" and "OUT" hits

    ///< Decoding the copyNo [copyNo = 1000*half + 100*lay + mod;]
    Int_t half = (int)((double)(copyNo)/1000.);
    copyNo -= half*1000;
    Int_t lay = (int)((double)(copyNo)/100.);
    copyNo -= lay*100;
    Int_t mod = copyNo;
    // Track ID
    Int_t trackID = gMC->GetStack()->GetCurrentTrackNumber();
    TParticle* part = gMC->GetStack()->GetCurrentTrack();

    ///< ONLY "HITS" RELEASED BY THE PRIMARY MUON 
//    if((int)part->GetMother(0)!=-1) return false;  
    ///< THIS IS TO BE CHANGED ACCORDINGLY FOR MORE DETAILED STUDIES
    TLorentzVector pos, mom;
    gMC->TrackPosition(pos);
    gMC->TrackMomentum(mom);

    a2mcSilHit* newHit = AddHit();
    newHit->SetEventNumber  (gMC->CurrentEvent());
    newHit->SetTrackID(trackID);
    newHit->SetPdgCode(part->GetPdgCode());
    newHit->SetMotherID(part->GetMother(0)); // Get First Mother
    newHit->SetPos (TVector3(pos.X(), pos.Y(), pos.Z()));
    newHit->SetMom (TVector3(mom.X(), mom.Y(), mom.Z()));
    newHit->SetHalfID(half);
    newHit->SetLayerID(lay);
    newHit->SetModuleID(mod);
//    newHit->SetIsEntering(gMC->IsTrackEntering());
//    newHit->SetIsExiting(gMC->IsTrackExiting());
    return true;
}

//_____________________________________________________________________________
void a2mcSilSD::Digitalize()
{
     // Digitalize the hits collection

    Int_t nofHits = GetHitCollectionSize();

    for(int j=0; j<nofHits;j++) {
        a2mcSilHit* theHit = GetHit(j);
        bool createDIGI = true;
//        if(DigiID[hitElemID]!= -1) createDIGI = false;

        if(createDIGI) { // NEW DIGI
            a2mcSilDIGI* newDIGI = AddDIGI();
        }
    }
}
//_____________________________________________________________________________
void a2mcSilSD::EndOfEvent()
{
    /// Print hits collection (if verbose)
    /// and delete hits afterwards.

    if (fVerboseLevel>0)  Print();

    // Reset hits collection
    if(fHitCollection) fHitCollection->Delete();  
    // Reset hits collection
    if(fDIGICollection) fDIGICollection->Delete();  
}
//
// -----------------------------------> PRIVATE FUNCTIONS
//
//_____________________________________________________________________________
a2mcSilHit* a2mcSilSD::AddHit()
{
    /// Create a new hit in the TClonesArray.
    /// \return  The new hit

    TClonesArray& ref = *fHitCollection;
    Int_t size = ref.GetEntriesFast();
    return new(ref[size]) a2mcSilHit();
}
//_____________________________________________________________________________
a2mcSilHit* a2mcSilSD::GetHit(Int_t i)
{
    /// Get the a2mcSilHit from the SilSD Hits collection
    //  \return  The hit

    return (a2mcSilHit*)fHitCollection->At(i);    

}
//_____________________________________________________________________________
a2mcSilDIGI* a2mcSilSD::AddDIGI()
{
    // Create a new digit in the TClonesArray.
    //  \return  The new digit

    TClonesArray& ref = *fDIGICollection;
    Int_t size = ref.GetEntriesFast();

    return new(ref[size]) a2mcSilDIGI();    

}
//_____________________________________________________________________________
a2mcSilDIGI* a2mcSilSD::GetDIGI(Int_t i)
{
    /// Get the a2mcDIGI from the SilSD DIGI collection
    //  \return  The DIGI

    return (a2mcSilDIGI*)fDIGICollection->At(i);    

}
//_____________________________________________________________________________
Int_t a2mcSilSD::GetHitCollectionSize()
{
    /// Return Hits Collection size
    TClonesArray& ref = *fHitCollection;
    Int_t size = ref.GetEntriesFast();
    return size;
}
//_____________________________________________________________________________
Int_t a2mcSilSD::GetDIGICollectionSize()
{
    /// Return DIGI Collection size
    TClonesArray& ref = *fDIGICollection;
    Int_t size = ref.GetEntriesFast();
    return size;
}

// PRINT FUNCTION
//_____________________________________________________________________________
void a2mcSilSD::Print(const Option_t* /*option*/) const
{
    /// Print the hits collection and the DIGI collection.

    Int_t nofHits = fHitCollection->GetEntriesFast();
    Int_t nofDigi = fDIGICollection->GetEntriesFast();

    cout << "\n-------->Hits Collection: in this event there are " << nofHits 
            << " hits in the SilDet: " << endl;

    for (Int_t i=0; i<nofHits; i++) (*fHitCollection)[i]->Print();          
    for (Int_t i=0; i<nofDigi; i++) (*fDIGICollection)[i]->Print();          
}
