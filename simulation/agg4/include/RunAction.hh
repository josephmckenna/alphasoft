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

#include "TPCreadout.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class G4Run;
class TClonesArray;
class TTree;
class TH1D;
class TH1I;
class TFile;
class TPCreadout;

class RunActionMessenger;

class RunAction : public G4UserRunAction
{
public:
  RunAction();
  virtual ~RunAction();

  void BeginOfRunAction(const G4Run*);
  void   EndOfRunAction(const G4Run*);
  
  inline TClonesArray* GetMCvertexArray() { return fMCvertexArray; }
  inline TClonesArray* GetMCpionsArray()  { return fMCpionsArray;  }
  inline TClonesArray* GetMCpi0Array()    { return fMCpi0Array; }
  inline TClonesArray* GetMCpositronsArray() { return fMCpositronsArray; }
  inline TClonesArray* GetMCpositronVtxArray() { return fMCpositronVtxArray; }
  inline TClonesArray* GetMCHitArray()    { return fMCHitArray; }
  inline TTree* GetMCinfoTree()           { return fMCinfoTree; }

  inline TPCreadout* GetTPCreadout()         { return fReadout; }

  inline TTree* GetTPCHitTree()               { return fTPCtree; }
  inline TClonesArray* GetTPCHitsArrayCheat() { return fTPCHitsArrayCheat; }
  inline TClonesArray* GetTPCHitsArray()      { return fTPCHitsArray; }
  inline TClonesArray* GetAnodesArray() { return fAnodesArray; }
  inline TClonesArray* GetPadsArray()   { return fPadsArray; }

  inline TClonesArray* GetAWSignals()     { return fAWsignals; }
  inline TClonesArray* GetPADSignals()    { return fPADsignals; }
  inline TTree* GetSignalsTree()          { return fSignalsTree; }

  inline TClonesArray* GetScintBarsHitsArray()   { return fBarsHitsArray; }
  inline TClonesArray* GetScintBarsMCHitsArray() { return fBarsMCHitsArray; }
  inline TTree* GetScintBarsHitTree()            { return fBarsTree; }

  inline TH1D* GetSecondHisto() {return fhSecond;}
  inline TH1I* GetHitsHisto()   {return fhNhits;}
  inline TH1I* GetNpi0Histo()   {return fhNpi0;}
  inline TH1I* GetNpositronsHisto()   {return fhNpositrons;}
  inline TH1D* GetRpositronsHisto()   {return fhRpositrons;}
  inline TH1D* GetPpositronsHisto()   {return fhPpositrons;}
  inline TH1D* GetPelectronsHisto()   {return fhPelectrons;}
  inline TH1I* GetNgammaHisto() {return fhNgamma;}
  inline TH1D* GetRgammaHisto() {return fhRgamma;}
  inline TH1D* GetPgammaHisto() {return fhPgamma;}
  inline TH1D* GetDCAgammaHisto() {return fhDCAgamma;}
  inline TH1I* GetNpairsHisto() {return fhNpairs;}

  inline TH1D* GetPadOccHisto() { return fhPadOcc;}
  inline TH1D* GetAwOccHisto() { return fhAwOcc;}

  inline TH1D* GetEdepChargedHistoBar() {return hdEChargedBar;}
  inline TH1D* GetEdepNeutralHistoBar() {return hdENeutralBar;}

  inline void SetRunName(G4String tag) {fTag=tag;}

private:

  TClonesArray* fMCvertexArray; // store MC vertex
  TClonesArray* fMCpositronVtxArray; //store positron creation vertices
  TClonesArray* fMCphotonsVtxArray; //store photon creation vertices
  TClonesArray* fMCpionsArray;  // store 4-momentum of generated charged pions in MeV
  TClonesArray* fMCpi0Array;    // store 4-momentum of generated neutral pions in MeV
  TClonesArray* fMCpositronsArray; // store 4-momentum of generated positrons in MeV
  TClonesArray* fMCHitArray;    // store ionization (not used)
  TTree* fMCinfoTree;

  TPCreadout* fReadout;      // readout manager to mimic ADC
  TClonesArray* fAnodesArray; // array containing TAnode obj containing signals
  TClonesArray* fPadsArray;   // array containing TPads obj containing signals
  TClonesArray* fTPCHitsArrayCheat; // TPC pad hits (the "digi") not time ordered
  TClonesArray* fTPCHitsArray;      // TPC pad hits (the "digi") time ordered
  TTree* fTPCtree;

  TClonesArray* fAWsignals;
  TClonesArray* fPADsignals;
  TTree* fSignalsTree;

  TClonesArray* fBarsHitsArray; // Scintillator Bars hits
  TClonesArray* fBarsMCHitsArray; 
  TTree* fBarsTree;
  

  TH1I*  fhNhits;
  TH1I*  fhNpi0;
  TH1I*  fhNpositrons;
  TH1D*  fhRpositrons;
  TH1D*  fhPpositrons;
  TH1D*  fhPelectrons;
  TH1I*  fhNgamma;
  TH1I*  fhNpairs;
  TH1D*  fhRgamma;
  TH1D*  fhPgamma;
  TH1D*  fhDCAgamma;
  TH1D*  fhSecond;

  TH1D* fhPadOcc;
  TH1D* fhAwOcc;

  TH1D* hdEChargedBar;
  TH1D* hdENeutralBar;

  TFile* fRoot;

  RunActionMessenger* fRunMessenger;
  G4String fTag;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif

