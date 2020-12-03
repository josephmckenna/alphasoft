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
/// \file src/StackingAction.cc
/// \brief Implementation of the StackingAction class
//
//
#include "StackingAction.hh"
#include "EventAction.hh"

#include "G4ParticleDefinition.hh"
#include "G4VProcess.hh"
#include "G4Track.hh"

#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4Gamma.hh"
#include "G4PionPlus.hh"
#include "G4PionMinus.hh"
#include "G4PionZero.hh"

#include "G4SystemOfUnits.hh"

//#include "G4String.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StackingAction::StackingAction(EventAction* evt):fEventAction(evt)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StackingAction::~StackingAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ClassificationOfNewTrack
StackingAction::ClassifyNewTrack(const G4Track * aTrack)
{
  G4ThreeVector r;
  G4ThreeVector p;
  G4int ID;
  if( aTrack->GetDefinition() == G4Electron::Definition() )
    {
      G4String procname;
      if(aTrack->GetCreatorProcess()!=0)
	procname = aTrack->GetCreatorProcess()->GetProcessName();
      else
	return fUrgent;
      // particle is primary ionization
      if(procname == "hIoni" && aTrack->GetVolume()->GetName() == "DriftChamber" )
	{
	  fEventAction->IncrementNelectrons();
	  return fKill;
	}
      else if(procname == "eIoni")
	return fKill;
      else if(procname == "conv" && aTrack->GetCreatorProcess()->GetProcessType() == fElectromagnetic )
        {
          p = aTrack->GetMomentum();
          fEventAction->PushBackElectronMomentum( new G4ThreeVector(p) );
          ID = aTrack->GetParentID();
          fEventAction->PushBackElectronParentID( new int(ID) );
          return fUrgent;
        }
      else 
	return fUrgent;
    }
  else if ( aTrack->GetDefinition() == G4PionPlus::Definition() || 
	    aTrack->GetDefinition() == G4PionMinus::Definition() )
    {
      fEventAction->IncrementNprimaries();
      return fUrgent;
    }
  else if ( aTrack->GetDefinition() == G4PionZero::Definition() )
    {
      //G4cout<<"pi0 Track ID: "<<aTrack->GetTrackID() <<G4endl;
      fEventAction->IncrementNpi0();
      return fUrgent;
    }
  else if ( aTrack->GetDefinition() == G4Positron::Definition() && aTrack->GetCreatorProcess()!=0 )
    {
      if ( aTrack->GetCreatorProcess()->GetProcessName() == "conv" &&
            aTrack->GetCreatorProcess()->GetProcessType() == fElectromagnetic )
        {
          r = aTrack->GetPosition();
          fEventAction->PushBackRpositron( new G4ThreeVector(r) );
          fEventAction->IncrementNpositrons();

          p = aTrack->GetMomentum();
          fEventAction->PushBackPositronMomentum( new G4ThreeVector(p) );
          ID = aTrack->GetParentID();
          fEventAction->PushBackPositronParentID( new int(ID) );
          return fUrgent;
        }
    }
  else if ( aTrack->GetDefinition() == G4Gamma::Definition() && aTrack->GetCreatorProcess()!=0)
    {
      if ( aTrack->GetCreatorProcess()->GetProcessName() == "Decay" )
        { 
          fEventAction->IncrementNgamma();
          r = aTrack->GetPosition();
          fEventAction->PushBackRgamma( new G4ThreeVector(r) );
          return fUrgent;
        }
      else 
        return fKill;
    }

  return fWaiting;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void StackingAction::NewStage()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void StackingAction::PrepareNewEvent()
{}
