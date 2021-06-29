#ifndef a2mc_TRACK_H
#define a2mc_TRACK_H

#include <iostream>
#include "vector"
#include "TObject.h"
#include "TParticle.h"
#include "TClonesArray.h"

class TClonesArray;

class a2mcMCTrack : public TObject
{

 public:

  /**  Default constructor  **/
  a2mcMCTrack();

  /**  Constructor from TParticle  **/
  a2mcMCTrack(TParticle* particle, Int_t trkid);
      
    // data members

  /**  Destructor  **/
  virtual ~a2mcMCTrack();


  /**  Accessors  **/
  Int_t          GetFirstDaughter()  const { return fDaughter[0];}
  Int_t          GetMother()  const { return fMother[0];}
  Int_t          GetPdgCode() const { return fPdgCode;}
  Int_t          GetTrackID() const { return fTrackID;}
  /**  Modifiers  **/
  virtual void   SetFirstDaughter(Int_t trkid)      { fDaughter[0] = trkid; }
  virtual void   SetLastDaughter(Int_t trkid)       { fDaughter[1] = trkid; }
  virtual void   SetTrackID(Int_t trkid)            { fTrackID     = trkid; }   

private: 

/* Private variables - copying private variables of TParticle */

  Int_t          fPdgCode;              // PDG code of the particle
  Int_t          fMother[2];            // Indices of the mother particles
  Int_t          fDaughter[2];          // Indices of the daughter particles
  Int_t          fTrackID;              // Track ID
  Double_t       fPx;                   // x component of momentum
  Double_t       fPy;                   // y component of momentum
  Double_t       fPz;                   // z component of momentum
  Double_t       fE;                    // Energy

  Double_t       fVx;                   // x of production vertex
  Double_t       fVy;                   // y of production vertex
  Double_t       fVz;                   // z of production vertex
  Double_t       fVt;                   // t of production vertex

  ClassDef(a2mcMCTrack,1);

};

#endif //a2mc_TRACK_H   
