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
// $Id: TPCField.cc $
//
// --------------------------------------------------------------------

#include "TPCField.hh"
#include "G4RunManager.hh"
#include "DetectorConstruction.hh"

TPCField::TPCField(G4double Bz, G4double V): G4ElectroMagneticField()
{
  fSolenoid = Bz;

  detector = (DetectorConstruction*) G4RunManager::GetRunManager()->GetUserDetectorConstruction();
  G4double a = detector->GetTPCinnerRadius();
  G4double b = a + detector->GetDriftRadius();
  fBias=V/log(b/a);
  //  G4cout<<fBias<<G4endl;
}

TPCField::~TPCField()
{
}

void TPCField::GetFieldValue(const G4double* point, G4double* field) const
{
  field[0] = field[1] = field[2] = field[3] = field[4] = field[5] = 0.0;

  // protect against Geant4 bug that calls us with point[] NaN.
  if(point[0] != point[0]) return;

  field[2]=fSolenoid;
  
  G4double r2 = point[0]*point[0]+point[1]*point[1];
  G4double cof=fBias/r2;
  field[3]=cof*point[0];
  field[4]=cof*point[1];
}
