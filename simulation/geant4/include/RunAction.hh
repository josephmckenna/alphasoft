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
/// \file analysis/shared/include/RunAction.hh
/// \brief Definition of the RunAction class
//
//
// $Id$
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef RunAction_h
#define RunAction_h 1

#include "G4String.hh"
#include "G4UserRunAction.hh"

#include "RunActionMessenger.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class G4Run;
class TClonesArray;
class TTree;
class TH1D;
class TFile;
class DetectorConstruction;

class RunAction : public G4UserRunAction
{
public:
  RunAction(DetectorConstruction*);
  virtual ~RunAction();

  void BeginOfRunAction(const G4Run*);
  void   EndOfRunAction(const G4Run*);
  
  inline TClonesArray* GetMCvertexArray() { return fMCvertexArray; }
  inline TClonesArray* GetMCpionsArray()  { return fMCpionsArray;  }
  inline TClonesArray* GetTPCHitsArray()  { return fTPCHitsArray; }
  inline TTree* GetMCinfoTree()           { return fMCinfoTree; }

  inline TClonesArray* GetGarfieldHitsArray()  { return fGarfieldHitsArray; }
  inline TClonesArray* GetAnodeHitsArray()     { return fAnodeHitsArray; }
  inline TTree* GetGarfieldTree()              { return fGarfieldTree; }

  inline TClonesArray* GetAWSignals()     { return fAWsignals; }
  inline TClonesArray* GetPADSignals()    { return fPADsignals; }
  inline TTree* GetSignalsTree()          { return fSignalsTree; }

  inline TH1D* GetSecondariesHisto() { return fhSecond; }
  inline TH1D* GetNhitsHisto()       { return fhNhits; }
  inline TH1D* GetPadOccHisto()      { return fhPadOcc;}
  inline TH1D* GetAwOccHisto()       { return fhAwOcc;}

  inline void SetRunName(G4String tag) {fTag=tag;}

private:

  DetectorConstruction* fDetector;

  TClonesArray* fMCvertexArray; // store MC vertex
  TClonesArray* fMCpionsArray;  // store 4-momentum of generated charged pions in MeV
  TClonesArray* fTPCHitsArray;  // TPC hits
  TTree* fMCinfoTree;

  TClonesArray* fGarfieldHitsArray;
  TClonesArray* fAnodeHitsArray;
  TTree* fGarfieldTree;

  TClonesArray* fAWsignals;
  TClonesArray* fPADsignals;
  TTree* fSignalsTree;

  TH1D* fhNhits;
  TH1D* fhSecond;

  TH1D* fhPadOcc;
  TH1D* fhAwOcc;

  TFile* fRoot;

  RunActionMessenger* fRunMessenger;
  G4String fTag;
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
