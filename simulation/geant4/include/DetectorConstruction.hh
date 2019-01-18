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
#include "G4Material.hh"

#include "TPC.hh"

class G4VPhysicalVolume;
class FieldSetup;
class TPC;
// Detector construction class to define materials and geometry.
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  DetectorConstruction();
  virtual ~DetectorConstruction();
  
  virtual G4VPhysicalVolume* Construct();
  
  inline void SetMagneticFieldValue(G4double b) { fMagneticField = b; }
  inline G4double GetMagneticFieldValue() const { return fMagneticField; }

  inline void SetQuencherFraction(G4double q) { fQuencherFraction=q; }
  inline G4double GetQuencherFraction() const { return fQuencherFraction; }

  inline void TurnOffMaterial() { kMat   = false; }
  inline void SetPrototype()    { kProto = true; }
  
  inline G4bool IsPrototype() const { return kProto; }
  inline G4bool Cryostat()    const { return kMat; }

private:
  G4double fMagneticField;
  G4double fQuencherFraction;
  G4bool kMat;
  G4bool kProto;

  TPC fDriftCell;

  FieldSetup* fpFieldSetup;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

