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

#ifndef FieldSetup_h
#define FieldSetup_h 1

#include "G4UniformMagField.hh"
#include "G4SystemOfUnits.hh"
class G4FieldManager;
class FieldSetup
{
public:

  FieldSetup();
  ~FieldSetup();

  inline void SetUniformBField(G4double b) {fBz=b;}
  inline G4double GetUniformBField() {return fBz;}

  static FieldSetup* GetFieldSetup();
  
  G4FieldManager*  GetLocalFieldManager() { return fLocalFieldManager;}

  void UpdateFieldSetup();
  void LocalMagneticFieldSetup();
  
private:
  G4UniformMagField* fGlobalField;
  G4UniformMagField* fLocalField;

  G4double fBz;

  G4FieldManager*   fGlobalFieldManager;
  G4FieldManager*   fLocalFieldManager;

  static FieldSetup* fpFieldSetup;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
