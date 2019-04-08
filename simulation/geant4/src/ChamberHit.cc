#include "ChamberHit.hh"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

G4ThreadLocal G4Allocator<ChamberHit>* ChamberHitAllocator;

ChamberHit::ChamberHit() : G4VHit(), fTime(-1),
			   fPos(G4ThreeVector()),
			   fEnergy(-1.),
			   fTrackID(-1),
			   fModelName("")
{}

ChamberHit::~ChamberHit(){}

ChamberHit::ChamberHit(const ChamberHit& rhs) : G4VHit() 
{
  fPos       = rhs.fPos;
  fTime      = rhs.fTime;
  fEnergy    = rhs.fEnergy;
  fTrackID   = rhs.fTrackID;
  fModelName = rhs.fModelName;
}

const ChamberHit& ChamberHit::operator=(const ChamberHit& rhs)
{
  fPos       = rhs.fPos;
  fTime      = rhs.fTime;  
  fEnergy    = rhs.fEnergy;
  fTrackID   = rhs.fTrackID;
  fModelName = rhs.fModelName;
  return *this;
}

G4int ChamberHit::operator==(const ChamberHit& rhs) const{
  return (this==&rhs) ? 1 : 0;
}

void ChamberHit::Draw()
{
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if(pVVisManager)
    {
      G4Circle circle(fPos);
      //circle.SetScreenSize(1.);
      circle.SetScreenSize(0.04);
      circle.SetFillStyle(G4Circle::filled);
      G4Colour colour(1.,0.,0.);
      G4VisAttributes attribs(colour);
      circle.SetVisAttributes(attribs);
      pVVisManager->Draw(circle);
    }
}

void ChamberHit::Print(){
  G4cout << "Printing hits" << G4endl;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
