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

#include "TH1D.h"
#include "TTree.h"
#include "TClonesArray.h"

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
  fEvtNb=evtNb;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* evt)
{
  // ------------------ TPC ------------------
  G4int TPCCollID=-1;
  G4SDManager * SDman = G4SDManager::GetSDMpointer();
  TPCCollID = SDman->GetCollectionID("TPCCollection");
  if(TPCCollID<0) return;

  G4HCofThisEvent* HCE = evt->GetHCofThisEvent();
  if(HCE) // custom function to retrive TPC hits
    {
      AddTPCHits( (TPCHitsCollection*)(HCE->GetHC(TPCCollID)) );
    }
  else 
    return;


  if(HCE) 
    FillHisto( (TPCHitsCollection*)(HCE->GetHC(TPCCollID)) );
 
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

void EventAction::FillHisto(TPCHitsCollection* THC)
{
  fRunAction->GetNhitsHisto()->Fill(double(THC->entries())); 
}
