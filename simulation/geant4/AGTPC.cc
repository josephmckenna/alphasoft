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
// $Id: G4Hbar :: Simulation of Antihydrogen Gravity Experiment$
// 05-Mar-2014  A. Capra
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"

#include "G4SystemOfUnits.hh"

#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
// #include "PrimaryGeneratorAction.hh"
// #include "RunAction.hh"
// #include "EventAction.hh"
#include "UserActionInitialization.hh"

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#endif

#ifdef G4UI_USE
#include "G4UIExecutive.hh"
#endif

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc,char** argv)
{
  G4double MagneticField=1.*tesla, QuencherFraction=0.3;
  G4String run_name="";

  // Choose the Random engine
  CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine);

// #ifdef G4MULTITHREADED
//   G4MTRunManager* runManager = new G4MTRunManager();
//   //  runManager->SetNumberOfThreads(2);
// #else
//   G4RunManager* runManager = new G4RunManager();
// #endif
  // Construct the default run manager
  G4RunManager * runManager = new G4RunManager;

  // Set mandatory initialization classes
  DetectorConstruction* detector = new DetectorConstruction;
  detector->SetMagneticFieldValue( MagneticField );
  detector->SetQuencherFraction( QuencherFraction );
  runManager->SetUserInitialization( detector );

  // My Physics List
  runManager->SetUserInitialization( new PhysicsList );

  // Set user action classes
  runManager->SetUserInitialization(new UserActionInitialization(detector));

  // // Set user action classes
  // PrimaryGeneratorAction* primgen = new PrimaryGeneratorAction( detector );
  
  // RunAction* run_action = new RunAction( detector );
  // run_action->SetRunName( run_name );
  // primgen->SetRunAction( run_action );
  
  // EventAction* event_action = new EventAction( run_action );

  // runManager->SetUserAction( primgen );
  // runManager->SetUserAction( run_action );
  // runManager->SetUserAction( event_action );
 
  // Initialize G4 kernel
  // runManager->Initialize();
  // G4 kernel initialization occurs in the macro

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  if(argc!=1)   // batch mode
    {
      G4String command = "/control/execute ";
      G4String fileName = argv[1];
      UImanager->ApplyCommand(command+fileName);
    }
  else  // interactive mode : define visualization and UI terminal
    {
#ifdef G4VIS_USE
      G4VisManager* visManager = new G4VisExecutive;
      visManager->Initialize();
#ifdef G4UI_USE
      G4UIExecutive* ui = new G4UIExecutive(argc, argv);
#endif
      UImanager->ApplyCommand("/control/execute vis.mac");
#endif

#ifdef G4UI_USE
      ui->SessionStart();
      delete ui;
#endif

#ifdef G4VIS_USE
      delete visManager;
#endif
    }

  // Job termination
  delete runManager;

  return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
