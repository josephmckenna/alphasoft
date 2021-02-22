///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include <iostream>

#include "a2mcHit.h"

/// \cond CLASSIMP
ClassImp(a2mcHit)
/// \endcond

using namespace std;
//_____________________________________________________________________________
a2mcHit::a2mcHit() 
  : fEvent(-1),
    fTrackID(-1),
    fPdgCode(-1),
    fMotherID(-1),
    fVolCopyNo(-1),
    fVolLSD(0.),
    fMom(),
    fPos(),
    fIsEntering(0),
    fIsExiting(0)
{
/// Default constructor
}
//_____________________________________________________________________________
a2mcHit::~a2mcHit() 
{
/// Destructor
}
//_____________________________________________________________________________
void a2mcHit::Print(const Option_t* /*opt*/) const
{
/// Printing

  cout << "  HIT| trackID: " << fTrackID 
       << "  position (cm): (" 
       << fPos.X() << ", " << fPos.Y() << ", " << fPos.Z() << ")"
       << endl;
}

