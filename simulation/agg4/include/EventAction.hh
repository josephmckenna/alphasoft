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
/// \file include/EventAction.hh
/// \brief Definition of the EventAction class
//
//
// $Id$
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "G4ThreeVector.hh"
//#include "globals.hh"

#include "TPCHit.hh"
#include "ScintBarHit.hh"

#include "TPCreadout.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class RunAction;
class TClonesArray;
class EventAction : public G4UserEventAction
{
public:
  EventAction( RunAction* );
  virtual ~EventAction();

  void  BeginOfEventAction(const G4Event*);
  void    EndOfEventAction(const G4Event*);

  std::vector<G4ThreeVector*> GetRPositrons() {return fRpositrons;}
  std::vector<G4ThreeVector*> GetRGamma() {return fRgamma;}
  std::vector<G4ThreeVector*> GetPGamma() {return fPgamma;}
  std::vector<G4ThreeVector*> GetPPositrons() {return fPpositrons;}
  std::vector<G4ThreeVector*> GetPElectrons() {return fPelectrons;}
  std::vector<G4int*> GetIDElectronParent()   {return fIDelectronParent;}
  std::vector<G4int*> GetIDPositronParent()   {return fIDpositronParent;}

  inline G4int GetEvtNb() { return fEvtNb; }

  inline G4int GetNelectrons() { return fNelect; }  
  inline G4int GetNhits()      { return fNhits;  }
  inline G4int GetNprimary()   { return fNprim;  }
  inline G4int GetNpi0()       { return fNpi0;   }
  inline G4int GetNpositrons() { return fNpositrons; }
  inline G4int GetNgamma()     { return fNgamma; }
  inline G4int GetNpairs()     { return fNpairs; }

  inline void IncrementNprimaries() { ++fNprim;  }
  inline void IncrementNelectrons() { ++fNelect; }
  inline void IncrementNpi0()       { ++fNpi0; }
  inline void IncrementNpositrons() { ++fNpositrons; }
  inline void IncrementNgamma()     { ++fNgamma; }
  inline void IncrementNpairs()     { ++fNpairs; }

  inline void PushBackRpositron(G4ThreeVector* Rpos)   { fRpositrons.push_back(Rpos); }
  inline void PushBackRgamma(G4ThreeVector* Rgam)      { fRgamma.push_back(Rgam); }
  inline void PushBackPgamma(G4ThreeVector* Pgam)      { fPgamma.push_back(Pgam); }

  inline void PushBackDCAgamma(G4double* dca) { fDCAgamma.push_back(dca); }

  inline void PushBackElectronParentID(G4int* IDelectron)  { fIDelectronParent.push_back(IDelectron); }
  inline void PushBackPositronParentID(G4int* IDpositron)   { fIDpositronParent.push_back(IDpositron); }

  inline void PushBackElectronMomentum(G4ThreeVector* Pelectron)  { fPelectrons.push_back(Pelectron); }
  inline void PushBackPositronMomentum(G4ThreeVector* Ppositron)  { fPpositrons.push_back(Ppositron); }
  inline void PushBackPhotonMomentum(G4ThreeVector* Pgamma)       { fPgamma.push_back(Pgamma); }

  void AddTPCHits(TPCHitsCollection*);
  void RemoveDuplicate(TClonesArray*);
  void Compress(TClonesArray*, TClonesArray*);
  
  void AddTPCreadout(TPCreadout*);
  void AddSignals();

  void AddScintBarsHits(ScintBarHitsCollection*);
    
  void FillHisto(TPCHitsCollection*);

private:
  G4int fPrintModulo;
  RunAction* fRunAction;
  G4int fNhits;
  G4int fNprim;
  G4int fNelect;
  G4int fNpairs;
  G4int fNpi0;
  G4int fNpositrons;
  G4int fNgamma;
  G4int fEvtNb;
  std::vector<G4ThreeVector*> fRgamma;
  std::vector<G4ThreeVector*> fRpositrons;
  std::vector<G4ThreeVector*> fPgamma;
  std::vector<G4ThreeVector*> fPpositrons;
  std::vector<G4ThreeVector*> fPelectrons;
  std::vector<G4int*> fIDelectronParent;
  std::vector<G4int*> fIDpositronParent;
  std::vector<double*> fDCAgamma;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

    
