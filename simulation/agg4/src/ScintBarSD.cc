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

#include "ScintBarSD.hh"
#include "ScintBarHit.hh"
#include "G4Step.hh"
#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4VProcess.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"

ScintBarSD::ScintBarSD(G4String name):G4VSensitiveDetector(name)
{
  collectionName.insert("ScintBarCollection");
}

ScintBarSD::~ScintBarSD(){;}

void ScintBarSD::Initialize(G4HCofThisEvent* HCE)
{
  static int HCID = -1;
  ScintBarCollection = new ScintBarHitsCollection(SensitiveDetectorName,collectionName[0]); 
  if(HCID<0)
    { 
      HCID = GetCollectionID(0); 
    }
  HCE->AddHitsCollection(HCID,ScintBarCollection);
}

G4bool ScintBarSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
  G4double edep = aStep->GetTotalEnergyDeposit();  
  if( edep == 0.) return false;
  //  if( edep < 0.3*keV ) return false;

  G4int pdg = aStep->GetTrack()->GetParticleDefinition()->GetPDGEncoding();
  //  if( pdg == 22 ) return false;

  if(aStep->GetTrack()->GetCreatorProcess()!=0)
    {
      G4String proc = aStep->GetTrack()->GetCreatorProcess()->GetProcessName();
      if(proc=="hIoni" || proc=="eIoni" || proc=="muIoni") return false;
    }
 
  ScintBarHit* newHit = new ScintBarHit();
  newHit->SetEdep    ( edep );
  newHit->SetParentID( aStep->GetTrack()->GetParentID() );
  newHit->SetTrackID ( aStep->GetTrack()->GetTrackID() );
  newHit->SetPDGcode ( pdg );
  newHit->SetPosition( aStep->GetTrack()->GetPosition() );
  newHit->SetTime    ( aStep->GetTrack()->GetGlobalTime() );
  ScintBarCollection->insert( newHit );

  return true;
}
