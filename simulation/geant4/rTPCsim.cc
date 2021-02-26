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


#include "G4RunManagerFactory.hh"
#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"

#include "G4SystemOfUnits.hh"

#include "GasModelParameters.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "UserActionInitialization.hh"

#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

//Header for Garfield random engine
#include "Garfield/Random.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// void PrintUsage()
// {
// 	printf("#############################################################\n");
// 	printf("\t--GarSeed 12345\t Force the seed for garfield\n");
// 	printf("#############################################################\n");
// 	exit(0);
// }

int main(int argc,char** argv)
{
  // Detect interactive mode (if no arguments) and define UI session
  //
  G4UIExecutive* ui = 0;
  if ( argc == 1 ) {
    ui = new G4UIExecutive(argc, argv);
  }

  int GarfieldSeed=18061985;
  // for (int i = 1; i < argc; i++) {
  //   int length=strlen(argv[i]);
  //   if (strncmp(argv[i],"--GarSeed",6)==0) {
  //     GarfieldSeed = atoi(argv[i + 1]);
  //     i++;
  //   //Ignore .mac files here
  //   } else if (strcmp(argv[i]+length-4,".mac")==0) {
  //     continue;
  //   } else {
  //     PrintUsage();
  //   }
  // }
  // //Choose seed for Garfield random engine
  // if (GarfieldSeed>0)
  Garfield::randomEngine.Seed(GarfieldSeed);
  // Choose the Random engine
  CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine);

  // // Construct the default run manager
  // G4RunManager * runManager = new G4RunManager;
  // Construct the default run manager
  auto* runManager =
    G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

  GasModelParameters* GasParam = new GasModelParameters();

  // Set mandatory initialization classes
  DetectorConstruction* detector = new DetectorConstruction(GasParam);
  runManager->SetUserInitialization( detector );

  // My Physics List
  //  runManager->SetUserInitialization( new PhysicsList(GasParam) );
  runManager->SetUserInitialization( new PhysicsList );

  // Set user action classes
  runManager->SetUserInitialization(new UserActionInitialization(detector));

  // G4 kernel initialization occurs in the macro

  // Initialize visualization
  //
  G4VisManager* visManager = new G4VisExecutive;
  // G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
  // G4VisManager* visManager = new G4VisExecutive("Quiet");
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  // Process macro or start UI session
  //
  if ( ! ui ) { 
    // batch mode
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command+fileName);
  }
  else { 
    // interactive mode
    UImanager->ApplyCommand("/control/execute init_vis.mac");
    ui->SessionStart();
    delete ui;
  }

  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted 
  // in the main() program !
  
  delete visManager;
  delete runManager;

  return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
