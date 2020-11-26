///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "a2mcSilHit.h"

ClassImp(a2mcSilHit)

using namespace std;

//_____________________________________________________________________________
a2mcSilHit::a2mcSilHit() 
  : fTrackID(-1),
    fPdgCode(-1),
    fMotherID(-1),
    fEvent(-1),
    fHalfID(-1),
    fLayerID(-1),
    fModuleID(-1),
    fEdep(0.),
    fTime(0.),
    fStep(0.)
{
/// Default constructor
}
//_____________________________________________________________________________
a2mcSilHit::~a2mcSilHit() 
{
/// Destructor
}
//_____________________________________________________________________________
void a2mcSilHit::Print(const Option_t* /*opt*/) const
{
/// Printing

  cout << "  HIT| trackID: " << fTrackID 
       << "  Half:  " << fHalfID
       << "  Layer: " << fLayerID
       << "  Module:" << fModuleID
       << "  energy deposit (keV): " << fEdep * 1.0e06
       << "  position (cm): (" 
       << fPos.X() << ", " << fPos.Y() << ", " << fPos.Z() << ")"
       << endl;
}

