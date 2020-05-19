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
// $Id$
//
/// \file DetectorConstruction.hh
/// \brief Definition of the DetectorConstruction class

#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
//#include "globals.hh"

#include "G4Material.hh"

class G4VPhysicalVolume;
class FieldSetup;
class DetectorMessenger;

// Detector construction class to define materials and geometry.
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  DetectorConstruction();
  virtual ~DetectorConstruction();
  
  virtual G4VPhysicalVolume* Construct();
  void UpdateGeometry();
  
  inline void SetTPCinnerRadius(G4double r) { fTPCrad=r; }
  inline G4double GetTPCinnerRadius() const { return fTPCrad; }

  inline void SetDriftRadius(G4double r) { fTPCthi=r; }
  inline G4double GetDriftRadius() const { return fTPCthi; }

  inline void SetTPCLength(G4double l) { fTPClen=l; }
  inline G4double GetTPCLength() const { return fTPClen; }

  inline void SetDriftGas(G4Material* gas) { fDriftGas=gas; }
  inline G4Material* GetDriftGas() const { return fDriftGas; }

  inline void SetQuencherFraction(G4double q) { fQuencherFraction=q; }
  inline G4double GetQuencherFraction() const { return fQuencherFraction; }

private:

  G4double fTPCrad;
  G4double fTPCthi;
  G4double fTPClen;
  G4Material* fDriftGas;
  G4double fQuencherFraction;

  FieldSetup* fpFieldSetup;
  DetectorMessenger* fpDetectorMessenger; 
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

