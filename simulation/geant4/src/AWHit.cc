#include "AWHit.hh"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

G4ThreadLocal G4Allocator<AWHit>* AWHitAllocator;

AWHit::AWHit() : G4VHit(), fAW(-1)
{}

AWHit::~AWHit(){}

AWHit::AWHit(const AWHit& rhs) : G4VHit() 
{
  fAW   = rhs.fAW;
  fData = rhs.fData;  
  fModelName = rhs.fModelName;
}

const AWHit& AWHit::operator=(const AWHit& rhs)
{
  fAW   = rhs.fAW;
  fData = rhs.fData;  
  fModelName = rhs.fModelName;
  return *this;
}

G4int AWHit::operator==(const AWHit& rhs) const{
  return (this==&rhs) ? 1 : 0;
}

// void AWHit::Draw()
// {
//   G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
//   if(pVVisManager)
//     {
//       G4Circle circle(fPos);
//       circle.SetScreenSize(1.);
//       circle.SetFillStyle(G4Circle::filled);
//       G4Colour colour(0.,1.,0.);
//       G4VisAttributes attribs(colour);
//       circle.SetVisAttributes(attribs);
//       pVVisManager->Draw(circle);
    
//       //G4cout<<"DRAWING "<<fPos.getY()<<G4endl;
//     }
// }

// void AWHit::Print(){
//   G4cout << "Printing hits" << G4endl;
// }
