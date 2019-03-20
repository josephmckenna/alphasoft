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

#include "PrimaryGeneratorMessenger.hh"

#include "PrimaryGeneratorAction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorMessenger::PrimaryGeneratorMessenger(PrimaryGeneratorAction* Gun)
:PrimaryAction(Gun)
{
  // AGTPCDir = new G4UIdirectory("/AGTPC/");
  // AGTPCDir->SetGuidance("UI commands for ALPHA-g TPC");

  ZedCCmd = new G4UIcmdWithADoubleAndUnit("/AGTPC/setZcenter",this);
  ZedCCmd->SetGuidance("Set centre of Hbar cloud");
  ZedCCmd->SetParameterName("Z",true);
  ZedCCmd->SetUnitCategory("Length");  
  ZedCCmd->SetDefaultValue(50.0);
  ZedCCmd->SetDefaultUnit("cm");
  ZedCCmd->AvailableForStates(G4State_Idle);  

  ZedLCmd = new G4UIcmdWithADoubleAndUnit("/AGTPC/setZlength",this);
  ZedLCmd->SetGuidance("Set length of Hbar cloud");
  ZedLCmd->SetParameterName("Z",true);
  ZedLCmd->SetUnitCategory("Length");  
  ZedLCmd->SetDefaultValue(2.0);
  ZedLCmd->SetDefaultUnit("cm");
  ZedLCmd->AvailableForStates(G4State_Idle);    

  TypeCmd = new G4UIcmdWithAnInteger("/AGTPC/setRunType",this);
  TypeCmd->SetGuidance("0: Hbar Annihilation  1: Hbar Annihilation (Berkeley) 2: cosmic 10: Realistic Randomized Annihilation");
  TypeCmd->SetParameterName("Run Type",false);
  TypeCmd->SetDefaultValue(0);

  GravCmd = new G4UIcmdWithAnInteger("/AGTPC/setGravDir", this);
  GravCmd->SetGuidance("1: Up -1: Down");
  GravCmd->SetParameterName("Direction",false);
  GravCmd->SetDefaultValue(-1);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger()
{
  delete ZedCCmd;
  delete ZedLCmd;
  delete TypeCmd;
  delete GravCmd;
  //  delete AGTPCDir;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{ 
  if( command == ZedCCmd ) 
    {
      G4double zed = ZedCCmd->GetNewDoubleValue(newValue);
      PrimaryAction->SetZcenter(zed);
    }  

  if( command == ZedLCmd ) 
    {
      G4double zed = ZedLCmd->GetNewDoubleValue(newValue);
      PrimaryAction->SetZlength(zed);
    }

  if( command == TypeCmd)
    {
      G4int type = TypeCmd->GetNewIntValue(newValue);
      PrimaryAction->SetType(type);
    }
  
  if( command == GravCmd)
    {
      G4int grav = TypeCmd->GetNewIntValue(newValue);
      PrimaryAction->SetGrav(grav);
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

