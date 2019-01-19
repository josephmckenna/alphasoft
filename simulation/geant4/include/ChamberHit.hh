#ifndef CHAMBERHIT_HH
#define CHAMBERHIT_HH

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

class ChamberHit : public G4VHit {
    
    
public:
  ChamberHit();
  virtual ~ChamberHit();
  ChamberHit(const ChamberHit &);
    
  const ChamberHit& operator=(const ChamberHit&);
  G4int operator==(const ChamberHit&) const;
    
  inline void* operator new(size_t);
  inline void  operator delete(void*);
	
  virtual void Draw();
  virtual void Print();
    
  G4ThreeVector GetPos()  { return fPos; }
  G4double GetTime()      { return fTime; }
  G4string GetModelName() { return fModelName; }
    
  void SetPos(G4ThreeVector xyz) { fPos = xyz; }
  void SetTime(G4double t)       { fTime = t; }
  void SetModelName(G4string n)  { fModelName = n; }

private:
  G4double      fTime;
  G4ThreeVector fPos;
  G4string      fModelName;
};

using ChamberHitsCollection=G4THitsCollection<ChamberHit>;

extern G4ThreadLocal G4Allocator<ChamberHit>* ChamberHitAllocator;

inline void* ChamberHit::operator new(size_t){
  if (!ChamberHitAllocator) {
    ChamberHitAllocator = new G4Allocator<ChamberHit>;
  }
  return (void*)ChamberHitAllocator->MallocSingle();
}

inline void ChamberHit::operator delete(void *aHit){
  ChamberHitAllocator->FreeSingle((ChamberHit*) aHit);
}

#endif
