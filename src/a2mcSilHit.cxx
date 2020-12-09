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
    fSilID(-1),
    fLayN(-1),
    fModN(-1),
    fnStrp(-1),
    fpStrp(-1),
    fEdep(0.)
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
       << "  SilID:  " << fSilID
       << "  Layer: " << fLayN
       << "  Module:" << fModN
       << "  n-strip (Z): " << fnStrp
       << "  p-strip (X): " << fpStrp
       << "  energy deposit (keV): " << fEdep * 1.0e06
       << "  position (cm): (" 
       << fPosX << ", " << fPosY << ", " << fPosZ << ")"
       << endl;
}

