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
/// \file electromagnetic/TestEm8/src/PhysicsList.cc
/// \brief Implementation of the PhysicsList class
//
// $Id$
//
//---------------------------------------------------------------------------
//
// ClassName:   PhysicsList
//
// Description: EM physics for ALPHA-G TPC
//
// Author:      V.Ivanchenko 01.09.2010
// Adapted:     A.Capra      04.03.2014
//
//----------------------------------------------------------------------------
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "PhysicsList.hh"
//#include "PhysicsListMessenger.hh"

#include "globals.hh"
#include "G4ExceptionSeverity.hh"
#include "G4RegionStore.hh"

#include "G4EmStandardPhysics.hh"
#include "G4DecayPhysics.hh"

#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4Proton.hh"

#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4LossTableManager.hh"
#include "G4ProductionCutsTable.hh"

#include "G4PAIModel.hh"
//#include "G4PAIPhotModel.hh"
#include "G4EmConfigurator.hh"

#include "G4ProcessManager.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleTable.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhysicsList::PhysicsList() : G4VModularPhysicsList()
{
  fConfig = G4LossTableManager::Instance()->EmConfigurator();
  G4LossTableManager::Instance()->SetVerbose(-1);
  defaultCutValue = 1.*mm;
  fCutForGamma     = defaultCutValue;
  fCutForElectron  = defaultCutValue;
  fCutForPositron  = defaultCutValue;

  // Decay Physics is always defined
  fDecayPhysicsList = new G4DecayPhysics();

  // EM physics
  fEmPhysicsList = new G4EmStandardPhysics(-1);

  SetVerboseLevel(-1);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhysicsList::~PhysicsList()
{
  delete fDecayPhysicsList;
  delete fEmPhysicsList;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::ConstructParticle()
{
  fDecayPhysicsList->ConstructParticle();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::ConstructProcess()
{
  AddTransportation();
  fEmPhysicsList->ConstructProcess();
  fDecayPhysicsList->ConstructProcess();

  AddPAIModel();
  fConfig->AddModels();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::SetCuts()
{  
  G4double new_cut=0.1*mm;
  G4double emin = 26.7*eV;
  //  G4double emin = 20.*eV;
  G4ProductionCutsTable::GetProductionCutsTable()->SetEnergyRange(emin,100.*GeV);
  if ( verboseLevel > 0 )
  {
    G4cout << "PhysicsList::SetCuts:";
    G4cout << "CutLength : " << G4BestUnit(defaultCutValue,"Length") << G4endl;
  }

  // set cut values for gamma at first and for e- second and next for e+,
  // because some processes for e+/e- need cut values for gamma
  SetCutValue(fCutForGamma, "gamma");
  SetCutValue(fCutForElectron, "e-");
  SetCutValue(fCutForPositron, "e+");

  // SetCutForGamma(fCutForGamma);
  // SetCutForElectron(fCutForElectron);
  // SetCutForPositron(fCutForPositron);

  G4Region* driftRegion = G4RegionStore::GetInstance()->GetRegion("DriftRegion",false);
  if (driftRegion) 
    {
      G4ProductionCuts* driftRegionCuts = new G4ProductionCuts();
      driftRegionCuts->SetProductionCut(new_cut, "gamma");
      driftRegionCuts->SetProductionCut(new_cut, "e-");
      driftRegionCuts->SetProductionCut(new_cut, "e+");
      //      driftRegionCuts->SetProductionCut(new_cut, "pi+");
      //      driftRegionCuts->SetProductionCut(new_cut, "pi-");
      driftRegion->SetProductionCuts(driftRegionCuts);
    } 
  else 
    {
      G4Exception("PhysicsList::SetCuts()","123",JustWarning,"Found no DriftRegion");
    }

  if ( verboseLevel > 0 ) { DumpCutValuesTable(); }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::SetCutForGamma(G4double cut)
{
  fCutForGamma = cut;
  SetParticleCuts(fCutForGamma, G4Gamma::Gamma());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::SetCutForElectron(G4double cut)
{
  fCutForElectron = cut;
  SetParticleCuts(fCutForElectron, G4Electron::Electron());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::SetCutForPositron(G4double cut)
{
  fCutForPositron = cut;
  SetParticleCuts(fCutForPositron, G4Positron::Positron());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PhysicsList::AddPAIModel()
{
  //  fConfig->SetVerbose(2);

  G4ParticleTable::G4PTblDicIterator* PartIter = 
    G4ParticleTable::GetParticleTable()->GetIterator(); 
  PartIter->reset();

  while( (*PartIter)() )
    {
      G4ParticleDefinition* particle = PartIter->value();
      G4String particleName = particle->GetParticleName();
      
      if(particleName == "e-" || particleName == "e+") 
	{
	  G4PAIModel* pai = new G4PAIModel(particle,"PAIModel");
	  fConfig->SetExtraEmModel(particleName,"eIoni",pai,"DriftRegion",
				   0.0,100.*GeV,pai);
	} 
      else if (particleName == "mu-" || particleName == "mu+")
	{
	  G4PAIModel* pai = new G4PAIModel(particle,"PAIModel");
	  fConfig->SetExtraEmModel(particleName,"muIoni",pai,"DriftRegion",
				   0.0,100.*GeV,pai);
	} 
      else if (particleName == "proton" ||
	       particleName == "pi+" ||
	       particleName == "pi-" )
	{
	  G4PAIModel* pai = new G4PAIModel(particle,"PAIModel");
	  fConfig->SetExtraEmModel(particleName,"hIoni",pai,"DriftRegion",
				   0.0,100.*GeV,pai);
	}
    }
}
