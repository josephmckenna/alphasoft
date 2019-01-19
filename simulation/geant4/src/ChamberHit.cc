#include "ChamberHit.hh"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

G4ThreadLocal G4Allocator<ChamberHit>* ChamberHitAllocator;

ChamberHit::ChamberHit() : G4VHit(), fTime(-1),
			   fPos(G4ThreeVector()){}

ChamberHit::~ChamberHit(){}

ChamberHit::ChamberHit(const ChamberHit& rhs) : G4VHit() 
{
  fPos       = rhs.fPos;
  fTime      = rhs.fTime;
  fModelName = rhs.fModelName;
}

const ChamberHit& ChamberHit::operator=(const ChamberHit& rhs)
{
  fPos       = rhs.fPos;
  fTime      = rhs.fTime;
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
      circle.SetScreenSize(1.);
      circle.SetFillStyle(G4Circle::filled);
      G4Colour colour(0.,1.,0.);
      G4VisAttributes attribs(colour);
      circle.SetVisAttributes(attribs);
      pVVisManager->Draw(circle);
    
      //G4cout<<"DRAWING "<<fPos.getY()<<G4endl;
    }
}

void ChamberHit::Print(){
  G4cout << "Printing hits" << G4endl;
}
