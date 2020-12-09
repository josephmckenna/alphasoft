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
    ///< Register hits collection in the Root manager;
    Register();
    ///< fSensitiveID stores the ID number of the silicon module volumes. 
    ///< The VMC can recover them with the gMC->VolId functions. The name 
    ///< of the volume is the required input. The names have been stored 
    ///< in a2mcApparatus in the silNameIDMap variable.
    ///< fSensitiveID is used in a2mcSilSD::ProcessHits()
    silNameIDMap = a2mcApparatus::Instance()->GetSilNameIDMap();
    map<int, std::string>::iterator it = silNameIDMap.begin();
    while(it != silNameIDMap.end()) {
        std::string vol_name = it->second;
        fSensitiveID.push_back(gMC->VolId(vol_name.c_str()));
        hasModuleHits.push_back("false"); ///< Creating an entry for each module
        ++it;
    }
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
    Int_t silID;   ///< copy number of the volume [filled by gMC->CurrentVolID] ==> silicon module ID [0-71]
    Int_t volID = gMC->CurrentVolID(silID); ///< volID [MC internal numbering]
    Bool_t isSil=false;
    for(UInt_t i=0; i<fSensitiveID.size(); i++) {
        if(volID==fSensitiveID[i]) {
            isSil = true;
            break;
        }
    }
    Double_t edep_lim = 0.;
    if(!isSil)                      return false; ///< We are not in one of the silicon module volumes
    if(gMC->TrackCharge() == 0.)    return false; ///< Not considering "hits" from neutral tracks
    if(gMC->Edep()<=edep_lim)       return false; ///< Cutting on energy deposited (single hit)
    ///< OTHER POSSIBLE SELECTIONS
//    if((int)part->GetMother(0)!=-1) return false;      ///< ONLY "HITS" RELEASED BY THE PRIMARY MUON 
//    if(gMC->TrackStep()==0.)        return false; ///< No step length (probably unused)
//    if(!gMC->IsTrackEntering()&&!gMC->IsTrackExiting()) return false; ///< Only "IN" and "OUT" hits

    Int_t lay, mod;
    IDToLayMod(silID, lay, mod);
    // Track ID
    Int_t trackID = gMC->GetStack()->GetCurrentTrackNumber();
    TParticle* part = gMC->GetStack()->GetCurrentTrack();

    TLorentzVector pos;
    gMC->TrackPosition(pos);
    
//  Transforming global (world/master) position into local (fiber reference system)
//    Double_t top_pos[3], loc_pos[3];
//    top_pos[0] = pos.X(); top_pos[1] = pos.Y(); top_pos[2] = pos.Z(); 
//    gMC->Gmtod(top_pos, loc_pos, 1);
    
    Int_t n_strip = ReturnNStrip(lay, pos.Z());
    Int_t p_strip = ReturnPStrip(lay, pos.X());
    ///< Checking that the hit is in the "active part" of the silicon
    if(n_strip==-1||p_strip==-1) return false; ///< return only if one of the two is -1 or need BOTH????
    
    TLorentzVector mom;
    gMC->TrackMomentum(mom);
    Double_t edep = gMC->Edep();
    if (edep==0.) return false;

    Double_t step = 0.;
    step = gMC->TrackStep();

    ///< Checking if "THIS TRACK" already released a hit in "THIS MODULE" and in "THESE STRIPS"
    bool createHit = true;
    Int_t updateHitID = -1;
    if((UInt_t)silID>=hasModuleHits.size()) {
        std::cout << "a2mcSilSD::ProcessHits -> check silID (" << silID << "). It should be < " << hasModuleHits.size() << std::endl;
        return false;
    }
    if(hasModuleHits[silID]) { ///< "THIS MODULE" (silID [0-71])
       for(int j=0; j<GetHitCollectionSize();j++) {
            a2mcSilHit* theHit = GetHit(j);
//            if(theHit->GetSilID() != silID) continue; ///< Not the same module (should be redundant indeed)
            if(theHit->GetNStrip() != n_strip) continue; ///< Not the same n-strip
            if(theHit->GetPStrip() != p_strip) continue; ///< Not the same p-strip
            Int_t hitTrack = theHit->GetTrackID();
            if(trackID==hitTrack) { ///< Same module, same strips, same track -> merge the hits
                createHit = false;
                updateHitID = j; // This previous hit was released in this layer by the same track  
            }
        }
    }
    if(createHit) { ///< CREATE A NEW HIT
        a2mcSilHit* newHit = AddHit();
        newHit->SetEventNumber  (gMC->CurrentEvent());
        newHit->SetTrackID(trackID);
        newHit->SetPdgCode(part->GetPdgCode());
        newHit->SetMotherID(part->GetMother(0)); // Get First Mother
        newHit->SetPos (TVector3(pos.X(), pos.Y(), pos.Z()));
        newHit->SetMom (TVector3(mom.X(), mom.Y(), mom.Z()));
        newHit->SetEdep(gMC->Edep());
        newHit->SetSilID(silID);
        newHit->SetLayN(lay);
        newHit->SetModN(mod);
        newHit->SetNStrip(n_strip);
        newHit->SetPStrip(p_strip);
        hasModuleHits[silID] = true;
    } else { ///< UPDATE AN EXISTING HIT (SAME MODULE, SAME TRACK)
        a2mcSilHit* theHit = GetHit(updateHitID);
        ///< double check
        if(theHit->GetSilID()!=silID||theHit->GetTrackID()!=trackID||theHit->GetNStrip()!=n_strip||theHit->GetPStrip()!=p_strip) {
            std::cout << "a2mcSilSD::ProcessHits -> check hit update procedure " << std::endl;
        }
        ///< Update energy (leave the other values unchanged)
        theHit->SetEdep(theHit->GetEdep()+gMC->Edep());
    }
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

//_____________________________________________________________________________
void a2mcSilSD::IDToLayMod(Int_t ID, Int_t& lay, Int_t& mod) 
{
    std::string sil_name="";
    map<int, std::string>::iterator it = silNameIDMap.find(ID);
    if(it!=silNameIDMap.end()) {
        sil_name = it->second;
    } else {
        cout << "a2mcSilSD::NameToLayMod --> Could not find sil_name for silID " << ID << endl;
        return;
    }
    lay = stoi(&sil_name[0],0,10);
    mod = stoi(&sil_name[3],0,16);
}

//_____________________________________________________________________________
Int_t a2mcSilSD::ReturnNStrip(Int_t lay, Double_t pos) {
///< This function, given the position of the hit, return the corresponding n-strip number
///< The position here is the Z coordinate of the hit
///< It returns -1 if the hit is outside the "active area"
///< In the MC, the Z  of the hit (in the local r.s.) is in the range [-11.5; 11.5]
    ///< These values have been imported from a2lib TAlphaEventObject class (a2lib/src/TAlphaEventObject.cxx)
    Double_t ASIC1_end      = -29.9463 - fPCBmountZ; ///< 0.27870
    Double_t ASIC1_start    = -18.8338 - fPCBmountZ; ///< 11.3912
    Double_t ASIC2_end      = -18.4063 - fPCBmountZ; ///< 11.8187
    Double_t ASIC2_start    = -7.29380 - fPCBmountZ; ///< 22.9312

//    pos += a2mcApparatus::Instance()->GetSilDet_L()/2.; ///< To have Z in the range [0.;23.0]
    Int_t strip = -1;
    Double_t s  = 0.;
    
    if(lay<=2) pos *= -1.; ///< Reverting the position for the "negative" layers (see Silicon Strips numbering scheme)
//    if (pos <= ASIC1_start && pos >= ASIC1_end) { ///< ASIC1
    ///< Germano (07/12/2020) - Adding half strip at the edges to avoid loosing hits on half of the first strips
    if (pos <= ASIC1_start+fnPitch/2. && pos >= ASIC1_end-fnPitch/2.) { ///< ASIC1
        s = (ASIC1_start - pos )/fnPitch; 
        strip = TMath::Nint(s);
    }

//    if (pos <= ASIC2_start && pos >= ASIC2_end) { ///< ASIC2
    ///< Germano (07/12/2020) - Adding half strip at the edges to avoid loosing hits on half of the first strips
    if (pos <= ASIC2_start+fnPitch/2. && pos >= ASIC2_end-fnPitch/2.) { ///< ASIC2
        s = (ASIC2_start - pos )/fnPitch;
        strip = TMath::Nint(s)+128;
    }
    return strip;
}

//_____________________________________________________________________________
Int_t a2mcSilSD::ReturnPStrip(Int_t lay, Double_t pos) {
///< This function, given the position of the hit, return the corresponding p-strip number
///< The position here is the X coordinate of the hit
///< It returns -1 if the hit is outside the "active area"
///< In the MC, the X  of the hit (in the local r.s.) is in the range [-3.0; 3.0]
    Double_t ASIC3_start = +2.89380;
    Double_t ASIC3_end   = +0.01089;
    Double_t ASIC4_start = -0.01135;
    Double_t ASIC4_end   = -2.89425;

    Int_t strip = -1;
    Double_t s  = 0.;

//  // Invert the other half of the detector
    if(lay<=2) pos *= -1.; ///< Reverting the position for the "negative" layers (see Silicon Strips numbering scheme)

//    if(pos <= ASIC3_start  && pos >= ASIC3_end) { ///< ASIC3
    ///< Germano (07/12/2020) - Adding half strip at the edges to avoid loosing hits on half of the first strips
    if(pos <= ASIC3_start+fpPitch/2.  && pos >= ASIC3_end-fpPitch/2.) { ///< ASIC3
        s = (ASIC3_start-pos)/fpPitch;
        strip = TMath::Nint(s);
    }
//    if(pos <= ASIC4_start && pos >= ASIC4_end) { ///< ASIC4
    ///< Germano (07/12/2020) - Adding half strip at the edges to avoid loosing hits on half of the first strips
    if(pos >= ASIC4_end-fpPitch/2. && pos <= ASIC4_start+fpPitch/2. ) { ///< ASIC4
        s = (ASIC4_start-pos)/fpPitch;
        strip = TMath::Nint(s)+128;
    }
    return strip;    
}

//______________________________________________________________________________
Double_t a2mcSilSD::GetpPos(Int_t strip)
{
// given strip number [0,255] returns strip position (y coordinate in the 
// local reference frame of the hybrid)
// strips [0,127] are ASIC 3 which starts at 
// "ASIC3_start" (y coordinate in the local reference frame of the hybrid)
// strips [128,255] are ASIC 4 which starts at
// "ASIC4_start" (y coordinate in the local reference frame of the hybrid)
// values in mm, output in cm

  assert( strip >= 0  );
  assert( strip < 256 );

  double ASIC3_start = 28.938;
  double ASIC4_start = -0.1135;

  double p = 0.;
  double s = (double)strip;
  if ( strip < 128 ) p = (ASIC3_start - fpPitch*s); // ASIC 3
  else p = (ASIC4_start - fpPitch*(s-128.)); // ASIC 4
  
//  if( fSilNum < nSil/2 ) p *= -1.0;
  
  return p/10.; // return in cm
}

//______________________________________________________________________________
Double_t a2mcSilSD::GetnPos(Int_t strip)
{
// given strip number [0,255] returns strip position (z coordinate in MRS)
// strips [0,127] are ASIC 1 which starts at 
// "ASIC1_start" (z coordinate in the local reference frame of the hybrid)
// strips [128,255] are ASIC 2 which starts at
// "ASIC2_start" (z coordinate in the local reference frame of the hybrid)
// values in mm, output in cm

  assert( strip >= 0  );
  assert( strip < 256 );

  double fPCBmountZ = 302.25;
  double ASIC1_start = -188.338;
  double ASIC2_start = -72.938;

  double n = 0.;
  double s =(double)strip;
  if( strip < 128 ) n = ASIC1_start - fnPitch*s; // ASIC 1
  else n = ASIC2_start - fnPitch*(s-128.);  // ASIC 2

  n+=fPCBmountZ*10.;
//  if( fSilNum < nSil/2 ) n *= -1.;

  return n/10.;
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
