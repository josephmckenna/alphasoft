//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//

#ifndef SCINTBARHIT_h
#define SCINTBARHIT_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

class G4AttDef;

class ScintBarHit : public G4VHit
{
public:

  ScintBarHit();
  ~ScintBarHit();
  ScintBarHit(const ScintBarHit &right);
  const ScintBarHit& operator=(const ScintBarHit &right);
  G4int operator==(const ScintBarHit &right) const;
  
  inline void *operator new(size_t);
  inline void operator delete(void *aHit);
  
  void Draw();
  const std::map<G4String,G4AttDef>* GetAttDefs() const;
  std::vector<G4AttValue>* CreateAttValues() const;
  void Print();
  void PrintPolar();
  
  inline void          SetEdep(G4double de)           { edep = de; }
  inline G4double      GetEdep()                      { return edep; }
  inline void          SetParentID(G4int pID)         { parentID = pID; }
  inline G4int         GetParentID()                  { return parentID; }  
  inline void          SetTrackID(G4int tID)          { trackID = tID; }
  inline G4int         GetTrackID()                   { return trackID; }
  inline void          SetPDGcode(G4int code)         { PDGcode = code; }
  inline G4int         GetPDGcode()                   { return PDGcode; }
  inline void          SetPosition(G4ThreeVector xyz) { position = xyz; }
  inline G4ThreeVector GetPosition()                  { return position; }
  inline void          SetTime(G4double t)            { time = t; }
  inline G4double      GetTime()                      { return time; }

private:
  G4double      edep;
  G4int         parentID;
  G4int         trackID;
  G4int         PDGcode;
  G4ThreeVector position;
  G4double      time;
  static std::map<G4String,G4AttDef> fAttDefs;

};

typedef G4THitsCollection<ScintBarHit> ScintBarHitsCollection;

extern G4Allocator<ScintBarHit> ScintBarHitAllocator;

inline void* ScintBarHit::operator new(size_t)
{
  void *aHit;
  aHit = (void *) ScintBarHitAllocator.MallocSingle();
  return aHit;
}

inline void ScintBarHit::operator delete(void *aHit)
{
  ScintBarHitAllocator.FreeSingle( (ScintBarHit*) aHit);
}

#endif
