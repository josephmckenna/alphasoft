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
/// \file src/EventAction.cc
/// \brief Implementation of the EventAction class
//
//
// $Id$
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "G4Event.hh"
#include "EventAction.hh"
#include "RunAction.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include "TPCHit.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"
#include "G4SDManager.hh"

#include "G4GlobalFastSimulationManager.hh"
#include "HeedInterfaceModel.hh"
#include "HeedOnlyModel.hh"

#include "TH1D.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "TWaveform.hh"
#include "TMChit.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction(RunAction* theRunAction):fPrintModulo(100),
						  fRunAction(theRunAction),
						  fEvtNb(-1)
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::~EventAction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event* evt)
{  
  G4int evtNb = evt->GetEventID();
  if (evtNb%fPrintModulo == 0) 
    G4cout << "\n---> Begin of event: " << evtNb << G4endl;

  fRunAction->GetTPCHitsArray()->Clear();
  fRunAction->GetGarfieldHitsArray()->Clear();
  fRunAction->GetAWSignals()->Clear();
  fEvtNb=evtNb;

  HeedOnlyModel *hom = (HeedOnlyModel*)((G4GlobalFastSimulationManager::GetInstance())->GetFastSimulationModel("HeedOnlyModel"));
  if( hom ) hom->Reset();
  HeedInterfaceModel *him = (HeedInterfaceModel*)((G4GlobalFastSimulationManager::GetInstance())->GetFastSimulationModel("HeedInterfaceModel"));
  if( him ) him->Reset();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* evt)
{
  G4cout << "EventAction::EndOfEventAction" << G4endl;
  G4HCofThisEvent* HCE = evt->GetHCofThisEvent();
  if(!HCE) // custom function to retrive TPC hits
    return;
  G4SDManager * SDman = G4SDManager::GetSDMpointer();

  // ------------------ TPC ------------------
  G4int TPCCollID=-1;
  TPCCollID = SDman->GetCollectionID("TPCCollection");
  if(TPCCollID<0) return;
  AddTPCHits( (TPCHitsCollection*)(HCE->GetHC(TPCCollID)) );
  //  FillHisto( (TPCHitsCollection*)(HCE->GetHC(TPCCollID)) );

  G4int ChamberCollID=-1;
  ChamberCollID = SDman->GetCollectionID("ChamberCollection");
  if(ChamberCollID<0) return;
  AddChamberHits( (ChamberHitsCollection*)(HCE->GetHC(ChamberCollID)) );

  G4int AWCollID=-1;
  AWCollID = SDman->GetCollectionID("AnodeWiresCollection");
  if(AWCollID<0) return;
  AddSignals( (AWHitsCollection*)(HCE->GetHC(AWCollID)) );
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventAction::AddTPCHits(TPCHitsCollection* THC)
{
  TClonesArray& hitarray = *(fRunAction->GetTPCHitsArray());
  for(int i=0;i<THC->entries();++i)
    {
      TPCHit* aHit = (*THC)[i];
      new(hitarray[i]) TMChit(aHit->GetTrackID(),aHit->GetPDGcode(),
			      aHit->GetPosition().x()/mm,aHit->GetPosition().y()/mm,
			      aHit->GetPosition().z()/mm,
			      aHit->GetPosition().perp()/mm,aHit->GetPosition().phi(),
			      aHit->GetTime()/ns,
			      aHit->GetEdep()/eV);
    }
  fRunAction->GetMCinfoTree()->Fill();
}

void EventAction::FillHisto(TPCHitsCollection*/* THC*/)
{
  //  fRunAction->GetNhitsHisto()->Fill(double(THC->entries())); 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventAction::AddChamberHits(ChamberHitsCollection* CHC)
{
  TClonesArray& hitarray = *(fRunAction->GetGarfieldHitsArray());
  for(int i=0;i<CHC->entries();++i)
    {
      ChamberHit* aHit = (*CHC)[i];
      new(hitarray[i]) TMChit(aHit->GetTrackID(),-11,
			      aHit->GetPos().x()/mm,aHit->GetPos().y()/mm,
			      aHit->GetPos().z()/mm,
			      aHit->GetPos().perp()/mm,aHit->GetPos().phi(),
			      aHit->GetTime()/ns,
			      aHit->GetEnergy()/eV);
    }
  fRunAction->GetGarfieldTree()->Fill();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventAction::AddSignals(AWHitsCollection* AWHC)
{
  TClonesArray& sig = *(fRunAction->GetAWSignals());
  for(int i=0;i<AWHC->entries();++i)
    {
      AWHit* hit = (*AWHC)[i];
      std::string hname = "a" + std::to_string( hit->GetAnode() );
      new(sig[i]) TWaveform(hname,hit->GetWaveform(),hit->GetModelName());
    }
  fRunAction->GetSignalsTree()->Fill();
}
