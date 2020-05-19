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

#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

#include "Randomize.hh"

#include "TRandom3.h"

#include <fstream>
#include <vector>
#include <array>

class G4Event;
class SecondaryProducer;

class G4ParticleGun;
class CRYGenerator;
class CRYParticle;

class PrimaryGeneratorMessenger;
class RunAction;
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction();    
  virtual ~PrimaryGeneratorAction();

  inline void SetType(G4int t)       {fType=t;}
  inline void SetGrav(G4int g)       {fGravDir=g;}
  inline void SetZcenter(G4double z) {fZcenter=z;}
  inline void SetZlength(G4double z) {fZlength=z;}

  void SetRunAction(RunAction* run_action);

  void GeneratePrimaries(G4Event*);

private:
  
  G4int    fType;
  G4int    fGravDir;
  G4double fZcenter;
  G4double fZlength;

  std::ifstream      fin;               // get annihilation position
  SecondaryProducer* fHbarAnnihilation; // create annihilation products 

  G4ParticleGun*             fParticleGun; // pointer a to G4 particle generator
  CRYGenerator*              fCosmicGen;   // cosmic generator
  std::vector<CRYParticle*>* fvect;        // vector of generated cosmic particles

  G4RandGauss* fRandGaus;

  PrimaryGeneratorMessenger* fMessenger; //messenger of this class

  RunAction* fRunAction; // to save MC vertex in default generator case

  std::ifstream anni_file;                            // get annihilation positions
  std::string up_anni_file;                           // up annihilation file path
  std::string down_anni_file;                         // down annihilation file path
  std::array<std::vector<double>, 10000> anni_pos;    // store annihilation positions
  std::array<std::vector<double>, 10000> temp_pos;    // temporary positions storage
  int loaded_anni_file;                               // store which gravity direction is loaded
  TRandom3* rand_gen;                                 // random number generator
  int num_anni_pos;                                   // store number of available annihilation positions
  int num_elim;                                       // count number of eliminated positions

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif


