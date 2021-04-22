///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "a2mcMCTrack.h"

using namespace std;

ClassImp(a2mcMCTrack)

// -----   Default constructor   -------------------------------------------
a2mcMCTrack::a2mcMCTrack()
{
  fPdgCode = 0;	 // PDG code of the particle
  fMother[0] = 0; 	  // Indices of the mother particles
  fDaughter[0] = -1;  // Indices of the daughter particles
  fMother[1] = 0; 	  // Indices of the mother particles
  fDaughter[1] = -1;  // Indices of the daughter particles
  fTrackID = -1;      // Track ID
  fPx = 0.; 		 // x component of momentum
  fPy = 0.; 		 // y component of momentum
  fPz = 0.; 		 // z component of momentum
  fE  = 0.;  		 // Energy
  fVx = 0.; 		 // x of production vertex
  fVy = 0.; 		 // y of production vertex
  fVz = 0.; 		 // z of production vertex
  fVt = 0.; 		 // t of production vertex
   
}
// -------------------------------------------------------------------------



// -----   Constructor from TParticle   ------------------------------------
a2mcMCTrack::a2mcMCTrack(TParticle* part, Int_t trkid=-1)
{
  fPdgCode     = part->GetPdgCode();
  fMother[0]   = part->GetFirstMother();
  fMother[1]   = part->GetSecondMother();
  fDaughter[0] = part->GetFirstDaughter();
  fDaughter[1] = part->GetLastDaughter();

  fTrackID     = trkid;

  fPx          = part->Px();
  fPy          = part->Py();
  fPz          = part->Pz();
  fE           = part->Energy();

  fVx          = part->Vx();
  fVy          = part->Vy();
  fVz          = part->Vz();
  fVt          = part->T();
}
// -------------------------------------------------------------------------

  
// -----   Destructor   ----------------------------------------------------
a2mcMCTrack::~a2mcMCTrack()
{
}
