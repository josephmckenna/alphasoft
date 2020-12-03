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
#include "G4UImanager.hh"

#include "G4SystemOfUnits.hh"

#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "StackingAction.hh"
#include "SteppingAction.hh"

#include <TMath.h>
#include <fstream>

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#endif

#ifdef G4UI_USE
#include "G4UIExecutive.hh"
#endif

#include "ElectronDrift.hh"
#include "TPCBase.hh"

int gmchit     = 1; // dis/en-able storing of MC hits (ionization points)
int gdigi      = 1; // dis/en-able storing of digi
int gdigicheat = 0; // dis/en-able storing of digi cheat

int gmcbars = 0;

double gMagneticField;
double gQuencherFraction;

double gPadZed;
double gPadRphi;
double gPadTime;

double gAnodeTime;

int gNbars;
double gBarRadius;
double gBarLength;

bool kMat;
bool kProto;

int gVerb;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc,char** argv)
{
  // Choose the Random engine
  CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine);

  //char setname[80];
  //sprintf(setname,"%s/settings.dat",getenv("RUN_TPC"));
  //std::ifstream fset(setname);
  std::ifstream fset("./settings.dat");
  fset>>gPadTime; //ns
  fset>>gAnodeTime; //ns
  fset>>gMagneticField; //T
  fset>>gQuencherFraction; //[0,1]
  fset>>kMat; // bool: 
  fset>>kProto; // bool:
  fset.close();

  G4cout<<"\n================================================="<<G4endl;
  G4cout<<"===== AGTPC main ====="<<G4endl;
  G4cout<<"Prototype set: "<<(kProto?"yes":"no")<<G4endl;
  TPCBase::TPCBaseInstance()->SetPrototype(kProto);
  if(TPCBase::TPCBaseInstance()->GetPrototype())
    G4cout<<"* simulation * PROTO-rTPC"<<G4endl;
  else
    G4cout<<"* simulation * ALPHA-g rTPC"<<G4endl;
  G4cout<<"TPC length = "<<TPCBase::TPCBaseInstance()->GetFullLengthZ()<<" cm"<<G4endl;
  G4cout<<"TPC inner radius = "<<TPCBase::TPCBaseInstance()->GetCathodeRadius()<<" cm\t\t";
  G4cout<<"TPC outer radius = "<<TPCBase::TPCBaseInstance()->GetROradius()<<" cm"<<G4endl;
  G4cout<<"AW radius = "<<TPCBase::TPCBaseInstance()->GetAnodeWiresRadius()<<" cm"<<G4endl;

  gPadZed=4.0;//mm
  gPadRphi=4.0;//mm

  G4cout<<"anode time bin: "<<gAnodeTime<<" ns"<<G4endl;
  G4cout<<"pad time bin: "<<gPadTime<<" ns"<<G4endl;
  G4cout<<"B = "<<gMagneticField<<" T"<<G4endl;
  G4cout<<"=================================================\n"<<G4endl;

  G4cout<<"Max Drift time: "<<ElectronDrift::ElectronDriftInstance()->GetTime( TPCBase::TPCBaseInstance()->GetCathodeRadius(true) )<<" ns"<<G4endl;

  gNbars = 64;
  gBarLength = 2.5*m;

  // Construct the default run manager
  G4RunManager * runManager = new G4RunManager;

  // Set mandatory initialization classes
  DetectorConstruction* detector = new DetectorConstruction;
  detector->SetQuencherFraction( gQuencherFraction );
  runManager->SetUserInitialization( detector );

  // My Physics List
  runManager->SetUserInitialization( new PhysicsList );


  // Set user action classes

  PrimaryGeneratorAction* primgen = new PrimaryGeneratorAction();
  runManager->SetUserAction( primgen );

  RunAction* run_action = new RunAction();
  runManager->SetUserAction( run_action );
  primgen->SetRunAction( run_action );

  EventAction* event_action = new EventAction( run_action );
  runManager->SetUserAction( event_action );

  runManager->SetUserAction( new StackingAction( event_action ) );

  //  runManager->SetUserAction( new SteppingAction() );
  runManager->SetUserAction( new SteppingAction( run_action ) );

  // Initialize G4 kernel
  runManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  if(argc!=1)   // batch mode
    {
      if(argc==3) { run_action->SetRunName(G4String(argv[2])); } //Modify output file name from command line
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
      //      UImanager->ApplyCommand("/control/execute EventDisplay.mac");
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
