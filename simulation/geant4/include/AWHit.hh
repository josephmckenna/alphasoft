#ifndef AWHIT_HH
#define AWHIT_HH

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

#include <vector>

class AWHit : public G4VHit {
    
    
public:
  AWHit(G4double&, G4double&, G4double&, G4double&);
  virtual ~AWHit();
  AWHit(const AWHit &);
    
  const AWHit& operator=(const AWHit&);
  G4int operator==(const AWHit&) const;
    
  inline void* operator new(size_t);
  inline void  operator delete(void*);
	
  // virtual void Draw();
  // virtual void Print();
    
  //  G4int GetAnode() const           { return fAW; }
  //  std::vector<double> GetWaveform(){ return fData; }
  G4ThreeVector GetPos()  const { return fPos; }
  G4double GetTime()      const { return fTime; } 
  G4double GetGain()      const { return fGain; }
  G4String GetModelName() const { return fModelName; }

  //  void SetAnode(G4int w)                   { fAW = w; }
  //  void SetWaveform(std::vector<double> wf) { fData = wf; }  
  void SetPos(G4ThreeVector xyz) { fPos = xyz; }
  void SetTime(G4double t)       { fTime = t; }
  void SetGain(G4double g)       { fGain = g; }
  void SetModelName(G4String n)  { fModelName = n; }

private:
  //  G4int fAW;
  G4ThreeVector fPos;
  G4double fTime;
  G4double fGain;
  //  std::vector<double> fData; 
  G4String            fModelName;
};

using AWHitsCollection=G4THitsCollection<AWHit>;

extern G4ThreadLocal G4Allocator<AWHit>* AWHitAllocator;

inline void* AWHit::operator new(size_t){
  if (!AWHitAllocator) {
    AWHitAllocator = new G4Allocator<AWHit>;
  }
  return (void*)AWHitAllocator->MallocSingle();
}

inline void AWHit::operator delete(void *aHit){
  AWHitAllocator->FreeSingle((AWHit*) aHit);
}

#endif
