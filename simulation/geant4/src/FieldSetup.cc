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
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "FieldSetup.hh"

#include "G4UniformMagField.hh"

#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"

FieldSetup* FieldSetup::fpFieldSetup=0;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

FieldSetup::FieldSetup():fGlobalField(0), fLocalField(0)
{
  fBz=1.*tesla;
  fGlobalFieldManager = G4TransportationManager::GetTransportationManager()->GetFieldManager();
  fLocalFieldManager = new G4FieldManager();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

FieldSetup::~FieldSetup()
{
  if (fGlobalField)      delete fGlobalField;
  if (fLocalField)       delete fLocalField;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void FieldSetup::UpdateFieldSetup()
{
  fGlobalField = new G4UniformMagField(G4ThreeVector(0.,0.,fBz));
  fGlobalFieldManager->SetDetectorField(fGlobalField);
  fGlobalFieldManager->CreateChordFinder(fGlobalField);
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void FieldSetup::LocalMagneticFieldSetup()
{
  fGlobalField = new G4UniformMagField(G4ThreeVector(0.,0.,0.));
  fGlobalFieldManager->SetDetectorField(0);
  fGlobalFieldManager->CreateChordFinder(fGlobalField);

  fLocalField       = new G4UniformMagField(G4ThreeVector(0.,0.,fBz));
  fLocalFieldManager->SetDetectorField(fLocalField);  
  fLocalFieldManager->CreateChordFinder( fLocalField );
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

FieldSetup* FieldSetup::GetFieldSetup()
{
  if(0 == fpFieldSetup)
    {
      static FieldSetup theInstance;
      fpFieldSetup = &theInstance;
    }
   
  return fpFieldSetup;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......




/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
