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

#include "TPCSD.hh"
#include "TPCHit.hh"
#include "ChamberHit.hh"
#include "G4Step.hh"
#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4VProcess.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"

TPCSD::TPCSD(G4String name):G4VSensitiveDetector(name)
{
  collectionName.insert("TPCCollection");
  collectionName.insert("ChamberCollection");
}

TPCSD::~TPCSD(){;}

void TPCSD::Initialize(G4HCofThisEvent* HCE)
{
  static int TPCHCID = -1;
  TPCCollection = new TPCHitsCollection(SensitiveDetectorName,collectionName[0]); 
  if(TPCHCID<0)
    { 
      TPCHCID = GetCollectionID(0); 
    }
  HCE->AddHitsCollection(TPCHCID,TPCCollection);

  static int ChamberHCID = -1; 
  ChamberCollection = new ChamberHitsCollection(SensitiveDetectorName,collectionName[1]);
  if(ChamberHCID<0)
    { 
      ChamberHCID = GetCollectionID(1); 
    }
  HCE->AddHitsCollection(ChamberHCID,ChamberCollection);
}

G4bool TPCSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
  G4double edep = aStep->GetTotalEnergyDeposit();  
  if(edep==0.) return false;

  G4ThreeVector position = aStep->GetTrack()->GetPosition();
  
  TPCHit* newHit = new TPCHit();
  newHit->SetEdep    ( edep );
  newHit->SetParentID( aStep->GetTrack()->GetParentID() );
  newHit->SetTrackID ( aStep->GetTrack()->GetTrackID() );
  newHit->SetPDGcode ( aStep->GetTrack()->GetParticleDefinition()->GetPDGEncoding() );
  newHit->SetPosition( position );
  newHit->SetTime    ( aStep->GetTrack()->GetGlobalTime() );
  TPCCollection->insert( newHit );

  return true;
}
