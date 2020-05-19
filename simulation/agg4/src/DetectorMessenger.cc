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
//
// $Id$
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "DetectorMessenger.hh"

#include "DetectorConstruction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithoutParameter.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorMessenger::DetectorMessenger(DetectorConstruction* Det)
:Detector(Det)
{ 
  AGTPCDir = new G4UIdirectory("/AGTPC/");
  AGTPCDir->SetGuidance("UI commands for ALPHA-g TPC");
         
  TPCradCmd = new G4UIcmdWithADoubleAndUnit("/AGTPC/setTPCinnerRadius",this);
  TPCradCmd->SetGuidance("Set Radius of TPC inner wall");
  TPCradCmd->SetParameterName("TPCrad",false);
  TPCradCmd->SetRange("TPCrad>=0.");
  TPCradCmd->SetUnitCategory("Length");
  TPCradCmd->AvailableForStates(G4State_PreInit,G4State_Idle);
  
  TPCthiCmd = new G4UIcmdWithADoubleAndUnit("/AGTPC/setDriftRegionRadius",this);
  TPCthiCmd->SetGuidance("Set Radius of the Drift Chamber");
  TPCthiCmd->SetParameterName("TPCthi",false);
  TPCthiCmd->SetUnitCategory("Length");  
  TPCthiCmd->AvailableForStates(G4State_PreInit,G4State_Idle);

  UpdateCmd = new G4UIcmdWithoutParameter("/AGTPC/update",this);
  UpdateCmd->SetGuidance("Update Trapping Paramenters.");
  UpdateCmd->SetGuidance("This command MUST be applied before \"beamOn\" ");
  UpdateCmd->SetGuidance("if you changed geometrical value(s).");
  UpdateCmd->AvailableForStates(G4State_Idle);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....

DetectorMessenger::~DetectorMessenger()
{
  delete TPCradCmd; 
  delete TPCthiCmd;
  delete UpdateCmd;
  delete AGTPCDir;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{ 
  if( command == TPCradCmd )
    Detector->SetTPCinnerRadius( TPCradCmd->GetNewDoubleValue(newValue) );
  
  if( command == TPCthiCmd )
    Detector->SetDriftRadius( TPCthiCmd->GetNewDoubleValue(newValue) );

  if( command == UpdateCmd ) Detector->UpdateGeometry();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
