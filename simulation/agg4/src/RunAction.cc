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
#include "RunActionMessenger.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"

#include <TFile.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TLine.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TMath.h>
#include <TString.h>

#include "TPCreadout.hh"

extern double gAnodeTime;
extern double gPadTime;

extern bool kProto;
extern double gMagneticField; // T
extern double gQuencherFraction;

extern int gmchit;
extern int gdigi;
extern int gdigicheat;

extern int gmcbars;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction():fMCinfoTree(0),fTPCtree(0),fBarsTree(0),
		       fhNhits(0), fhNpi0(0),
		       fhNpositrons(0),
		       fhRpositrons(0), fhPpositrons(0), fhPelectrons(0),
		       fhNgamma(0), fhNpairs(0), fhRgamma(0), fhPgamma(0), fhDCAgamma(0),
		       fhSecond(0), 		       
		       fhPadOcc(0),fhAwOcc(0),
		       hdEChargedBar(0), hdENeutralBar(0),
		       fRoot(0)
{
  fMCvertexArray = new TClonesArray("TVector3");
  fMCpositronVtxArray = new TClonesArray("TVector3");
  fMCpionsArray = new TClonesArray("TLorentzVector");
  fMCpi0Array = new TClonesArray("TLorentzVector");
  fMCpositronsArray = new TClonesArray("TLorentzVector");
  fMCHitArray = new TClonesArray("TMChit");

  fTPCHitsArrayCheat = new TClonesArray("TDigi");
  fTPCHitsArray = new TClonesArray("TDigi");

  fAnodesArray = new TClonesArray("TAnode");
  fPadsArray   = new TClonesArray("TPads");

  fAWsignals = new TClonesArray("TWaveform");
  fPADsignals = new TClonesArray("TWaveform");

  fReadout = new TPCreadout;

  fBarsHitsArray = new TClonesArray("TScintDigi");

  fBarsMCHitsArray = new TClonesArray("TMChit");

  fRunMessenger = new RunActionMessenger(this);
  fTag = "";
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::~RunAction()
{
  delete fMCvertexArray;
  delete fMCpositronVtxArray;
  delete fMCpionsArray;
  delete fMCpi0Array;
  delete fMCpositronsArray;
  delete fMCHitArray;

  delete fTPCHitsArrayCheat;
  delete fTPCHitsArray;

  delete fReadout;
  delete fAnodesArray;
  delete fPadsArray;

  delete fAWsignals;
  delete fPADsignals;

  delete fBarsHitsArray;
  delete fBarsMCHitsArray;

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
      fileName = TString::Format("%s/outAgTPC_%s_AWtime%1.0fns_PADtime%1.0fns_B%1.2fT_Q%1.0f.root",
				 getenv("DATADIR"),
				 (kProto?"proto":"det"),
				 gAnodeTime,gPadTime,
				 gMagneticField,
				 gQuencherFraction*1.e2);
    }
  else
    {
      G4cout << "Output file tagged with _" << fTag.data() << G4endl;
      fileName = TString::Format("%s/outAgTPC_%s_AWtime%1.0fns_PADtime%1.0fns_B%1.2fT_Q%1.0f_%s.root",
				getenv("DATADIR"),
				(kProto?"proto":"det"),
				gAnodeTime,gPadTime,
				gMagneticField,
				gQuencherFraction*1.e2,
				fTag.data());
    }

  fRoot = new TFile(fileName.Data(),"RECREATE");
  G4cout<<" Save Output to "<<fRoot->GetName()<<G4endl;

  fMCinfoTree = new TTree("MCinfo","MCinfo");
  fMCinfoTree->Branch("MCvertex",&fMCvertexArray,32000,0);
  //  fMCinfoTree->Branch("MCpositronVtx",&fMCpositronVtxArray,32000,0);
  fMCinfoTree->Branch("MCpions",&fMCpionsArray,32000,0);
  //  fMCinfoTree->Branch("MCpi0s",&fMCpi0Array,32000,0);
  // fMCinfoTree->Branch("MCpositrons",&fMCpositronsArray,32000,0);
 
  fTPCtree = new TTree("TPCMCdata","TPCMCdata");
  if( gmchit ) fTPCtree->Branch("TPCMCHits",&fMCHitArray,32000,0);
  if( gdigicheat ) fTPCtree->Branch("TPCHitsCheat",&fTPCHitsArrayCheat,32000,0);
  if( gdigi ) fTPCtree->Branch("TPCHits",&fTPCHitsArray,32000,0);
  fTPCtree->Branch("Anodes",&fAnodesArray,32000,0);
  fTPCtree->Branch("Pads",&fPadsArray,32000,0);

  fSignalsTree = new TTree("Signals","Signals");
  fSignalsTree->Branch("AW",&fAWsignals,32000,0);
  fSignalsTree->Branch("PAD",&fPADsignals,32000,0);

  fBarsTree = new TTree("ScintBarsMCdata","ScintBarsMCdata");
  fBarsTree->Branch("ScintBarHits",&fBarsHitsArray,32000,0);
  if( gmcbars ) fBarsTree->Branch("ScintMCHits",&fBarsMCHitsArray,32000,0);

  fhSecond = new TH1D("fhSecond",
		      "Number of Electrons per Charged Pion;e^{-}/#pi^{#pm};events",
		      1000,0.,1000.);

  fhNhits = new TH1I("fhNhits","Number of Hits per Event;N of Hits;events",5000,0,5000);

  hdEChargedBar = new TH1D("hdEChargedBar",
			   "Energy Deposit by Charged Particles in the Barrell;dE [keV];Hits",
			   70000,0.,7000.);
  hdENeutralBar = new TH1D("hdENeutralBar",
			   "Energy Deposit by Neutral Particles in the Barrell;dE [keV];Hits",
			   70000,0.,7000.);

  fhPadOcc = new TH1D("fhPadOcc","Number of Pads Channels per Event",2500,0.,2500.);
  fhAwOcc = new TH1D("fhAwOcc","Number of Anode Channels per Event",256,0.,256.);
  
  fhNpi0 = new TH1I("fhNpi0","Number of pi0 generated per event;N pi0;events",11,-0.5,10.5);

  fhNpositrons = new TH1I("fhNpositrons","Number of e+ per event;N e+;events",12,-0.5,11.5);

  fhRpositrons = new TH1D("fhRpositrons","Radius of e+ production from pi0-originating photons;R[mm];events",1000,0,200);

  fhNgamma = new TH1I("fhNgamma","Number of photons per event;N gamma;events",12,-0.5,11.5);

  fhNpairs = new TH1I("fhNpairs","Number of e+ e- pairs per event;N pairs;events",12,-0.5,11.5);

  fhRgamma = new TH1D("fhRgamma","pi0 decay photon vertex radius;R[mm];events",1000,22.275-0.0002,22.275+0.0002);

  fhDCAgamma = new TH1D("fhDCAgamma","DCA of pi0 decay photons to MC vertex;DCA[mm];events",500,0,0.00002);

  fhPgamma = new TH1D("fhPgamma","3Momentum magnitude of pi0 decay photons;magnitude[eV];events",1000,0,1000);

  fhPpositrons = new TH1D("fhPpositrons","3Momentum magnitude of pi0 decay photon positrons;magnitude[eV];events",1000,0,1000);

  fhPelectrons = new TH1D("fhPelectrons","3Momentum magnitude of pi0 decay photon electrons;magnitude[eV];events",1000,0,1000);

  fReadout->Reset();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* aRun)
{
  G4int NbOfEvents = aRun->GetNumberOfEvent();
  if (NbOfEvents == 0) return;

  G4cout << "RunAction::EndOfRunAction Number of Simulated Events: " 
	 << fSignalsTree->GetEntriesFast() << G4endl;

  fRoot->Write();
  fRoot->Close();

  fReadout->Reset();

  fMCvertexArray->Clear("C");
  fMCpositronVtxArray->Clear("C");
  fMCpionsArray->Clear("C");
  fMCpi0Array->Clear("C");
  fMCpositronsArray->Clear("C");
  fMCHitArray->Clear("C");

  fTPCHitsArrayCheat->Clear("C");
  fTPCHitsArray->Clear("C");
  fAnodesArray->Clear("C");
  fPadsArray->Clear("C");

  fBarsHitsArray->Clear("C");
  fBarsMCHitsArray->Clear("C");

  G4cout << "### Run " << aRun->GetRunID() << " end." << G4endl;

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
