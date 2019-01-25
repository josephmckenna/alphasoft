#ifndef AWHIT_HH
#define AWHIT_HH

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

#include <vector>

class AWHit : public G4VHit {
    
    
public:
  AWHit();
  virtual ~AWHit();
  AWHit(const AWHit &);
    
  const AWHit& operator=(const AWHit&);
  G4int operator==(const AWHit&) const;
    
  inline void* operator new(size_t);
  inline void  operator delete(void*);
	
  // virtual void Draw();
  // virtual void Print();
    
  G4int GetAnode()                 { return fAW; }
  std::vector<double> GetWaveform(){ return fData; }
  G4String GetModelName()          { return fModelName; }

  void SetAnode(G4int w)                   { fAW = w; }
  void SetWaveform(std::vector<double> wf) { fData = wf; }
  void SetModelName(G4String n)            { fModelName = n; }

private:
  G4int               fAW;
  std::vector<double> fData; 
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
