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
/// \file analysis/shared/src/RunAction.cc
/// \brief Implementation of the RunAction class
//
//
// $Id$
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "RunAction.hh"
#include "DetectorConstruction.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

#include <TFile.h>
#include <TH1D.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TString.h>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction(DetectorConstruction* det): fDetector(det),
						 fMCinfoTree(0),
						 fSignalsTree(0),
						 fhNhits(0),fhSecond(0),fhPadOcc(0),fhAwOcc(0),
						 fRoot(0)
{
  fMCvertexArray = new TClonesArray("TVector3");
  fMCpionsArray = new TClonesArray("TLorentzVector");
  fTPCHitsArray = new TClonesArray("TMChit");

  fGarfieldHitsArray = new TClonesArray("TMChit");
  fAnodeHitsArray = new TClonesArray("TMChit");

  fAWsignals = new TClonesArray("TWaveform");
  fPADsignals = new TClonesArray("TWaveform");

  fRunMessenger = new RunActionMessenger(this);
  fTag = "";
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::~RunAction()
{
  delete fMCvertexArray;
  delete fMCpionsArray;
  delete fTPCHitsArray;
  delete fGarfieldHitsArray;
  delete fAnodeHitsArray;
  delete fAWsignals;
  delete fPADsignals;

  if(fRunMessenger) delete fRunMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run* aRun)
{
  G4int RunNumber=aRun->GetRunID();
  G4cout << "### Run " << RunNumber << " start." << G4endl;

  TString fileName;
  if(fTag.isNull())
    {
      fileName = TString::Format("%s/outAgTPC_%s_B%1.2fT_Q%1.0f.root",
				 getenv("MCDATA"),
				 (fDetector->IsPrototype()?"proto":"det"),
				 fDetector->GetMagneticFieldValue()/tesla,
				 fDetector->GetQuencherFraction()*1.e2);
    }
  else
    {
      G4cout << "Output file tagged with _" << fTag.data() << G4endl;
      fileName = TString::Format("%s/outAgTPC_%s_B%1.2fT_Q%1.0f_%s.root",
				 getenv("MCDATA"),
				 (fDetector->IsPrototype()?"proto":"det"),
				 fDetector->GetMagneticFieldValue()/tesla,
				 fDetector->GetQuencherFraction()*1.e2,
				 fTag.data());
    }

  fRoot = new TFile(fileName.Data(),"RECREATE");
  G4cout<<" Save Output to "<<fRoot->GetName()<<G4endl;

  fMCinfoTree = new TTree("MCinfo","MCinfo");
  fMCinfoTree->Branch("MCvertex",&fMCvertexArray,32000,0);
  fMCinfoTree->Branch("MCpions",&fMCpionsArray,32000,0);
  fMCinfoTree->Branch("TPCHits",&fTPCHitsArray,32000,0);

  fGarfieldTree = new TTree("Garfield","Garfield");
  fGarfieldTree->Branch("GarfHits",&fGarfieldHitsArray,32000,0);
  fGarfieldTree->Branch("AnodeHits",&fAnodeHitsArray,32000,0);

  fSignalsTree = new TTree("Signals","Signals");
  fSignalsTree->Branch("AW",&fAWsignals,32000,0);
  fSignalsTree->Branch("PAD",&fPADsignals,32000,0);

  // fhSecond = new TH1D("fhSecond",
  // 		      "Number of Electrons per Charged Pion;e^{-}/#pi^{#pm};events",
  // 		      1000,0.,1000.);

  // fhNhits = new TH1D("fhNhits","Number of Hits per Event;N of Hits;events",5000,0.,5000.);

  // fhPadOcc = new TH1D("fhPadOcc","Number of Pads Channels per Event",2500,0.,2500.);
  // fhAwOcc = new TH1D("fhAwOcc","Number of Anode Channels per Event",256,0.,256.);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* aRun)
{
  G4int NbOfEvents = aRun->GetNumberOfEvent();
  if (NbOfEvents == 0) return;

  G4cout << "RunAction::EndOfRunAction Number of Events " << NbOfEvents << G4endl;
  G4cout << "RunAction::EndOfRunAction MCinfo: " << fMCinfoTree->GetEntriesFast() << G4endl;
  G4cout << "RunAction::EndOfRunAction Garf: " << fGarfieldTree->GetEntriesFast() << G4endl;
  G4cout << "RunAction::EndOfRunAction Sig: " << fSignalsTree->GetEntriesFast() << G4endl;

  fRoot->Write();
  fRoot->Close();

  // fMCvertexArray->Clear("C");
  // fMCpionsArray->Clear("C");
 
  // fTPCHitsArray->Clear("C");

  // //  fAWsignals->Clear("C");
 
  G4cout << "### Run " << aRun->GetRunID() << " end." << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
