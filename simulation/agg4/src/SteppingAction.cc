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
/// \file src/SteppingAction.cc
/// \brief Implementation of the SteppingAction class
//
//
#include "SteppingAction.hh"

#include "G4ParticleDefinition.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "G4PionPlus.hh"
#include "G4PionMinus.hh"

#include "TH1D.h"

SteppingAction::SteppingAction()
{
  fkinE_thr = 1.*keV;
}

SteppingAction::SteppingAction(RunAction* ra):fRunAction(ra)
{
  fkinE_thr = 1.*keV;
}

void SteppingAction::UserSteppingAction(const G4Step* aStep)
{
  // while simulating no decay of pions, found that occassionally low E pions
  // would continue tracking while losing no energy and no transport. This stops
  // that from happening
  if(((aStep->GetTrack()->GetDefinition() == G4PionPlus::Definition()) ||
      (aStep->GetTrack()->GetDefinition() == G4PionMinus::Definition())) &&
     (aStep->GetTrack()->GetKineticEnergy() <= fkinE_thr)){
    aStep->GetTrack()->SetTrackStatus(fStopAndKill);
    return;
  }

  if( aStep->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName()
      == "ScintBars" )
    {
      G4double edep = aStep->GetTotalEnergyDeposit();   
      G4double charge = aStep->GetTrack()->GetDefinition()->GetPDGCharge();
      if( charge != 0. ) // pions, e+/e-, muons
	fRunAction->GetEdepChargedHistoBar()->Fill(edep/keV);
      else 
	{
	  // G4cout<<aStep->GetTrack()->GetDefinition()->GetParticleName()
	  // 	<<" dE: "<<G4BestUnit(edep,"Energy")<<G4endl;	  
	  fRunAction->GetEdepNeutralHistoBar()->Fill(edep/keV);
	}
    }

}
