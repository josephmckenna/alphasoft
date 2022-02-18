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

#include "ScintBarHit.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4UnitsTable.hh"
#include "G4AttValue.hh"
#include "G4AttDef.hh"
#include "G4AttCheck.hh"

G4Allocator<ScintBarHit> ScintBarHitAllocator;

ScintBarHit::ScintBarHit()
{;}

ScintBarHit::~ScintBarHit()
{;}

ScintBarHit::ScintBarHit(const ScintBarHit &right) : G4VHit()
{
  barID    = right.barID;
  edep     = right.edep;
  parentID = right.parentID;
  trackID  = right.trackID;
  PDGcode  = right.PDGcode;
  position = right.position;
  time     = right.time;
  IsWhere  = right.IsWhere;
}

const ScintBarHit& ScintBarHit::operator=(const ScintBarHit &right)
{
  barID    = right.barID;
  edep     = right.edep;
  parentID = right.parentID;
  trackID  = right.trackID;
  PDGcode  = right.PDGcode;
  position = right.position;
  time     = right.time;
  IsWhere  = right.IsWhere;
  return *this;
}

G4int ScintBarHit::operator==(const ScintBarHit &right) const
{
  return (this==&right) ? 1 : 0;
}

std::map<G4String,G4AttDef> ScintBarHit::fAttDefs;

void ScintBarHit::Draw()
{
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if(pVVisManager)
  {
    G4Circle circle(position);
    //    circle.SetScreenSize(0.04);
    circle.SetScreenDiameter(10.);
    circle.SetFillStyle(G4Circle::filled);
    G4Colour colour(0.,1.,1.);
    G4VisAttributes attribs(colour);
    circle.SetVisAttributes(attribs);
    pVVisManager->Draw(circle);
  }
}

const std::map<G4String,G4AttDef>* ScintBarHit::GetAttDefs() const
{
  // G4AttDefs have to have long life.  Use static member...
  if (fAttDefs.empty()) {
    fAttDefs["HitType"] =
      G4AttDef("HitType","Type of hit","Physics","","G4String");
  }
  return &fAttDefs;
}

std::vector<G4AttValue>* ScintBarHit::CreateAttValues() const
{
  // Create expendable G4AttsValues for picking...
  std::vector<G4AttValue>* attValues = new std::vector<G4AttValue>;
  attValues->push_back
    (G4AttValue("HitType","ScintBarHit",""));
  //G4cout << "Checking...\n" << G4AttCheck(attValues, GetAttDefs());
  return attValues;
}

void ScintBarHit::PrintPolar()
{
  G4cout<<parentID<<"\t"<<PDGcode<<"\t"
	<<G4BestUnit(position.perp(),"Length")<<"\t"
	<<G4BestUnit(position.phi(),"Angle")<<"\t"
	<<G4BestUnit(position.z(),"Length")<<"\t"
	<<G4BestUnit(time,"Time")<<"\t\t"
	<<G4BestUnit(edep,"Energy")<<"\t\t"
  <<G4BestUnit(barID,"Bar number")<<G4endl;
}

void ScintBarHit::Print()
{
  G4cout<<parentID<<"\t"<<PDGcode<<"\t"
	<<G4BestUnit(position.x(),"Length")<<"\t"
	<<G4BestUnit(position.y(),"Length")<<"\t"
	<<G4BestUnit(position.z(),"Length")<<"\t"
	<<G4BestUnit(time,"Time")<<"\t\t"
  <<G4BestUnit(barID,"Bar number")<<G4endl;
}
