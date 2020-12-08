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

#include "EventAction.hh"
#include "G4Event.hh"
#include "RunAction.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include "TPCHit.hh"
#include "ScintBarHit.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"
#include "G4SDManager.hh"

#include <TMath.h>
#include <TTree.h>
#include <TH1.h>
#include <TClonesArray.h>
#include <TVector3.h>
#include <TVector.h>

#include "TMChit.hh"
#include "TDigi.hh"
#include "ElectronDrift.hh"
#include "TScintDigi.hh"

#include "TWaveform.hh"

#include "StackingAction.hh"

#include <vector>
#include <algorithm>


extern int gmchit;
extern int gdigi;
extern int gdigicheat;

extern int gmcbars;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction(RunAction* theRunAction):fPrintModulo(100),
						  fRunAction(theRunAction),
						  fNhits(0),
						  fNprim(0),
						  fNelect(0),
						  fNpairs(0),
						  fNpi0(0),
                                                  fNpositrons(0),
						  fNgamma(0), 
						  fEvtNb(-1)
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::~EventAction()
{ 
  fRpositrons.clear();
  fPpositrons.clear();
  fIDpositronParent.clear();

  fPelectrons.clear();
  fIDelectronParent.clear();

  fRgamma.clear();
  fPgamma.clear();
  
  fDCAgamma.clear();  

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event* evt)
{  
  G4int evtNb = evt->GetEventID();
  if (evtNb%fPrintModulo == 0) 
    G4cout << "\n---> Begin of event: " << evtNb << G4endl;

  fRunAction->GetMCHitArray()->Clear();

  fRunAction->GetTPCHitsArrayCheat()->Clear();
  fRunAction->GetTPCHitsArray()->Clear();

  fRunAction->GetPadsArray()->Delete();
  fRunAction->GetAnodesArray()->Delete();

  fRunAction->GetScintBarsHitsArray()->Clear();
  fRunAction->GetScintBarsMCHitsArray()->Clear();

  fRunAction->GetTPCreadout()->Reset();
  fRunAction->GetTPCreadout()->SetEventNumber(evtNb);

  fRunAction->GetAWSignals()->Clear();
  fRunAction->GetPADSignals()->Clear();
  
  fNprim=fNelect=fNpi0=fNpositrons=fNgamma=fNpairs=0;

  fEvtNb=evtNb;
  //  G4cout << " EventAction::BeginOfEventAction  Wires after reset: " << fRunAction->GetTPCreadout()->GetNumberOfWiresHit() << G4endl;
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
      AddTPCHits( (TPCHitsCollection*)(HCE->GetHC(TPCCollID)));
    }
  else 
    return;

  if( gdigi )
    {
      TClonesArray* TPCdigi = fRunAction->GetTPCHitsArray();
      if(TPCdigi) 
	{
	  RemoveDuplicate(TPCdigi);
	  TPCdigi->Sort();
	}
    }

  if( gdigicheat )
    {
      TClonesArray* TPCdigiCheat = fRunAction->GetTPCHitsArrayCheat();
      if(TPCdigiCheat) 
	{
	  RemoveDuplicate(TPCdigiCheat);
	}
    }

  fRunAction->GetTPCHitTree()->Fill();
  //  fRunAction->GetTPCreadout()->Reset();

  if(HCE) 
    FillHisto( (TPCHitsCollection*)(HCE->GetHC(TPCCollID)) );
  
  
  // ------------ Scintillating Bars ------------
  G4int BarsCollID=-1;
  BarsCollID = SDman->GetCollectionID("ScintBarCollection");
  if(BarsCollID<0) return;
  
  if(HCE)
    AddScintBarsHits( (ScintBarHitsCollection*)(HCE->GetHC(BarsCollID)) );
  else 
    return;
  fRunAction->GetScintBarsHitTree()->Fill();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventAction::AddTPCHits(TPCHitsCollection* THC)
{
  TClonesArray& mchitarray = *(fRunAction->GetMCHitArray());
  TClonesArray& hitarraycheat = *(fRunAction->GetTPCHitsArrayCheat());
  TClonesArray& hitarray = *(fRunAction->GetTPCHitsArray());
  
  G4double r,t_d,phi,dphi,alpha,angle,rphi,z;
  int j=0;
  for(int i=0;i<THC->entries();++i)
    {
      TPCHit* aHit = (*THC)[i];
      //      aHit->PrintPolar();
      r    = aHit->GetPosition().perp()/mm; 
      z    = aHit->GetPosition().z()/mm;
      phi  = aHit->GetPosition().phi();

      if( r < TPCBase::TPCBaseInstance()->GetCathodeRadius(true) || 
	  r > TPCBase::TPCBaseInstance()->GetROradius(true) ) // failsafe mode
	continue;

      if( phi < 0. )
	phi += TMath::TwoPi();

      alpha = ElectronDrift::ElectronDriftInstance()->GetAzimuth(r);
      dphi=alpha+phi;

      if( dphi >= TMath::TwoPi() )
	angle = dphi - TMath::TwoPi();
      else
	angle = dphi;

      rphi = angle*TPCBase::TPCBaseInstance()->GetROradius(true);

      t_d = ElectronDrift::ElectronDriftInstance()->GetTime(r);
      if(TMath::IsNaN(t_d)) // another fail mode
	continue;
      
      // // hard debug
      // G4cout<<i<<"\t"<<r<<"\t"<<t_d<<"\t"<<phi<<"\t"<<dphi<<"\t"<<z<<G4endl;
      // //----------------------------------------------------------------------------

      const G4int id=aHit->GetTrackID(),
       pdg = aHit->GetPDGcode();


      rphi=rphi;
      //----------------------------------------------------------------------------
      // Digitization old fashion - with cheats and MC hits
      
      if( gmchit )
	new(mchitarray[j]) TMChit(id,pdg,
				  aHit->GetPosition().x()/mm,
				  aHit->GetPosition().y()/mm,
				  z,r,phi);

      if( gdigicheat )
	{
	  new(hitarraycheat[j]) TDigi(aHit->GetTrackID(),aHit->GetPDGcode(),
				      z,rphi,t_d);
	  ((TDigi*) hitarraycheat.Last())->Digitization();
	}

      if( gdigi )
	{
	  new(hitarray[j]) TDigi(aHit->GetTrackID(),aHit->GetPDGcode(),
				 z,rphi,t_d);
	  ((TDigi*) hitarray.Last())->Digitization();
	}
      //---------------------------------------------------------------------------- 
      ++j;

      fRunAction->GetTPCreadout()->AddHit( t_d, z, angle ); // correct pad signal assignment
    }

  // G4cout << "EventAction::AddTPCHits Event # " << fRunAction->GetTPCreadout()->GetEventNumber() << "\t";
  // G4cout << " Wires to readout: " << fRunAction->GetTPCreadout()->GetNumberOfWiresHit() << "\n";

 
  //=========================================
  // ADD Electrodes Signals (from Garfield++)
  AddTPCreadout( fRunAction->GetTPCreadout() );
  AddSignals( );
  //=========================================
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventAction::RemoveDuplicate(TClonesArray* DigiArray)
{
  int entries = DigiArray->GetEntries(), j=0;
  while( j < entries )
    {
      TDigi* digi_temp = (TDigi*) DigiArray->ConstructedAt(j);
      for(int i=j+1; i<DigiArray->GetEntries(); ++i)
	{
	  TDigi* digi_new = (TDigi*) DigiArray->ConstructedAt(i);
	  if(digi_temp->IsSamePad(digi_new)) 
	    {
	      DigiArray->RemoveAt(i);
	      --entries;
	    }
	  else break;
	}
      DigiArray->Compress();
      ++j;
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventAction::AddSignals()
{
  TPCreadout* ro = fRunAction->GetTPCreadout();
  double mV2ADC(8.);
  TClonesArray& awsig = *(fRunAction->GetAWSignals());
  //  G4cout << "EventAction::AddSignals Event # " << fEvtNb << G4endl;
  int i=0;
  for(int aw = 0; aw<int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires()); ++aw)
   {
     TAnode* awire = ro->GetWire(aw);
     double Q = awire->GetCharge();
     if( !(Q > 0.) ) continue;

     //G4cout << "i=" << i << " aw=" << aw <<"  LE: "<<awire->GetLeadingEdgeDriftTime()<<"ns ";
     std::string hname = "a" + std::to_string( aw );
     //G4cout << hname << " charge: " << Q << " (size: " << awire->GetSignal().size() << ")" << G4endl;

     std::vector<int> temp(411,0);
     for(unsigned int n = 0; n < awire->GetSignal().size(); ++n)
        temp[ n ] = int(awire->GetSignal().at(n)*mV2ADC); 
      
     //G4cout << "tempwf size:" << temp.size() << G4endl;
     new(awsig[i]) TWaveform(hname,&temp,"0xAC");
     ++i;
   }

  TClonesArray& padsig = *(fRunAction->GetPADSignals());
  i=0;
  for(int ip = 0; ip<TPCBase::TPCBaseInstance()->GetNumberOfPads(); ++ip)
    {
      TPads* apad = ro->GetPad(ip);
      double Q = apad->GetCharge();
      if( !(Q > 0.) ) continue;

      int sec=ip%32;
      int row=ip/32;
      std::string hname = "p_" + std::to_string( sec ) + 
	"_" + std::to_string( row );
      //G4cout << i << "\t" << ip << "\t" << hname << G4endl;

      std::vector<int> temp(411,0);
      for(unsigned int n = 0; n < apad->GetSignal().size(); ++n)
        temp[ n ] = -1*int(apad->GetSignal().at(n)*mV2ADC); 
      new(padsig[i]) TWaveform(hname, &temp,"0xAC");
      ++i;
    }
  
  //  G4cout << "EventAction::AddSignals Filling SignalsTree" << G4endl;
  //  G4AutoLock lock(&aMutex);
  fRunAction->GetSignalsTree()->Fill();
}  

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventAction::AddTPCreadout(TPCreadout* ro)
{
  //  std::cout<<"@@@ Event # "<<ro->GetEventNumber()<<" @@@"<<std::endl;

  TClonesArray& padarray = *(fRunAction->GetPadsArray());
  padarray.Delete();
  int jp=0;
  double PadOcc=0.;
  for(int ip = 0; ip<TPCBase::TPCBaseInstance()->GetNumberOfPads(); ++ip)
    {
      TPads* apad = ro->GetPad(ip);
      double Q = apad->GetCharge();
      if( Q == 0. ) continue;
      else 
	{
	  // //	  std::cout<<"EventAction::AddTPCreadout(TPCreadout* ro) ip: "<<ip
	  // std::cout<<"ip: "<<ip
	  // 	   <<" [pad: "<<apad->GetPad()
	  // 	   <<" z: "<<apad->GetZ()
	  // 	   <<" rp: "<<apad->GetRphi()<<"]";
	  TPads* p = (TPads*) padarray.ConstructedAt(jp);
	  //	  std::cout<<"\tj: "<<jp;
	  p->Reset();

	  p->Locate( apad->GetPad() );
	  // std::cout<<"\tpad: "<<p->GetPad();
	  // std::cout<<"\tz: "<<p->GetZ()
	  // 	   <<"\trphi: "<<p->GetRphi();

	  p->SetCharge( Q );
	  //	  std::cout<<"\tQ: "<<p->GetCharge();
	  PadOcc+=Q;

	  p->SetSignal( apad->GetSignal() );

	  p->SetDriftTimes( apad->GetDriftTimes() );
	  // if( p->GetDriftTimes().size() > 0 )
	  //   std::cout<<"\tt (LE): "<<p->GetLeadingEdgeDriftTime()<<" ns\n";
	  // else
	  //   std::cout<<"\n";
	  ++jp;
	}
    }
  fRunAction->GetPadOccHisto()->Fill( PadOcc );

  TClonesArray& anodearray = *(fRunAction->GetAnodesArray());
  anodearray.Delete();
  int ja=0;
  double AwOcc=0.;
  for(int aw = 0; aw<int(TPCBase::TPCBaseInstance()->GetNumberOfAnodeWires()); ++aw)
    {
      TAnode* awire = ro->GetWire(aw);
      double Q = awire->GetCharge();
      if( Q == 0. ) continue;
      else 
	{
	  //	  std::cout<<"EventAction::AddTPCreadout(TPCreadout* ro) aw: "<<aw
	  // std::cout<<"aw: "<<aw
	  // 	   <<" [anode: "<<awire->GetWire()
	  // 	   <<" pos: "<<awire->GetPosition()<<"]";
	  TAnode* a = (TAnode*) anodearray.ConstructedAt(ja);
	  //	  std::cout<<"\tj: "<<ja;
	  a->Reset();

	  a->Locate( awire->GetWire() );
	  // std::cout<<"\tanode: "<<a->GetWire();
	  // std::cout<<"\tpos: "<<a->GetPosition();

	  a->SetCharge( Q );
	  //	  std::cout<<"\tQ: "<<a->GetCharge();
	  AwOcc+=Q;	  

	  a->SetSignal( awire->GetSignal() );

	  a->SetDriftTimes( awire->GetDriftTimes() );
	  // if( a->GetDriftTimes().size() > 0 )
	  //   std::cout<<"\tt (LE): "<<a->GetLeadingEdgeDriftTime()<<" ns\n";
	  // else
	  //   std::cout<<"\n";

	  a->SetZed( awire->GetZed() );
	  ++ja;
	}
    }
  fRunAction->GetAwOccHisto()->Fill( AwOcc );
  //  std::cout<<"EventAction::AddTPCreadout(TPCreadout* ro) finished\n";
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventAction::FillHisto(TPCHitsCollection* THC)
{
  fNhits=THC->entries();
  fRunAction->GetHitsHisto()->Fill(fNhits);
  fRunAction->GetNpi0Histo()->Fill(fNpi0);
  fRunAction->GetNpositronsHisto()->Fill(fNpositrons);
  fRunAction->GetNgammaHisto()->Fill(fNgamma);
  double ratio = ( (double) fNelect ) / ( (double) fNprim );
  fRunAction->GetSecondHisto()->Fill(ratio);
  
  for (unsigned int i=0;i<fRpositrons.size();++i)
    {
      fRunAction->GetRpositronsHisto()->Fill( (fRpositrons.at(i))->perp() );
    }
  for (unsigned int j=0;j<fRgamma.size();++j)
    {
      fRunAction->GetRgammaHisto()->Fill( (fRgamma.at(j))->perp() );
    }

  for (unsigned int k=0;k<fPpositrons.size();++k)
    {
      fRunAction->GetPpositronsHisto()->Fill( fPpositrons.at(k)->mag() );
    }

  for (unsigned int l=0;l<fPelectrons.size();++l)
    {
      fRunAction->GetPelectronsHisto()->Fill( fPelectrons.at(l)->mag() );
    }

  if ( fPpositrons.size()>0 )
      {
        G4ThreeVector* Pp;
        G4ThreeVector* Pe;
        G4ThreeVector* Vp;
        G4ThreeVector* mcvtx = new G4ThreeVector();
        G4int* Ppid;
        G4int* Peid;
	//        double* DCA; // unused  -- AC
        for (unsigned int i=0;i<fPpositrons.size();++i)
          {
            Pp = fPpositrons.at(i);
            Vp = fRpositrons.at(i);
            Ppid = GetIDPositronParent().at(i);
            Pe = GetPElectrons().at(i);
            Peid = GetIDElectronParent().at(i);
            if ( *Ppid == *Peid )
              {
                // std::cout<<"Pp = ( "<<Pp->x()<<", "<<Pp->y()<<", "<<Pp->z()<<")"<<std::endl;
                // std::cout<<"Pe = ( "<<Pe->x()<<", "<<Pe->y()<<", "<<Pe->z()<<")"<<std::endl;
                // std::cout<<"Pgamma = ( "<<Pe->x()+Pp->x()<<", "<<Pe->y()+Pe->y()<<", "<<Pe->z()+Pe->z()<<")"<<std::endl;
                ++fNpairs;
                PushBackPgamma( new G4ThreeVector( 
                  (Pp->x() + Pe->x()), (Pp->y() + Pe->y()), (Pp->z() + Pe->z()) ) );
                // VECTOR IDENTITY: DCA = |MCvtx X (Pgamma - MCvtx)| / |Pgamma|
                
		//((TVector3*)(fRunAction->GetMCvertexArray())->AddrAt( GetEvtNb() ))->Print();
                mcvtx->set( ((TVector3*)(fRunAction->GetMCvertexArray())->AddrAt( GetEvtNb() ))->X(),
                            ((TVector3*)(fRunAction->GetMCvertexArray())->AddrAt( GetEvtNb() ))->Y(),
                            ((TVector3*)(fRunAction->GetMCvertexArray())->AddrAt( GetEvtNb() ))->Z() );
                // std::cout<<"In EventAction.cc: *mcvtx = "<<*mcvtx<<std::endl;
                // std::cout<<"*fPgamma.back() = "<<*fPgamma.back()<<std::endl;
                //std::cout<<"DCA method 1 = "<<( (fPgamma.back())->cross( (*mcvtx) ).mag()/(fPgamma.back())->mag())<<" mm"<<std::endl;
                //std::cout<<"DCA method 2 = "<<( (*mcvtx) - (*Vp) - (*Pp)/( Pp->mag() ) ).cross( (*mcvtx) - (*Vp) ).mag()<<std::endl;
		// std::cout<<"DCA method 3 = "<<( (*mcvtx)-(*Vp) ).cross( (*mcvtx)-(*Vp)-( (*fPgamma.back())/(fPgamma.back())->mag() ) ).mag()<<std::endl;
                PushBackDCAgamma( new double( ( (*mcvtx)-(*Vp) ).cross( (*mcvtx)-(*Vp)-( (*fPgamma.back())/(fPgamma.back())->mag() ) ).mag() ));
              }
          }
        fRunAction->GetNpairsHisto()->Fill(fNpairs);
      }

  for (unsigned int m=0;m<fPgamma.size();++m)
    {
      fRunAction->GetPgammaHisto()->Fill( fPgamma.at(m)->mag() );
      fRunAction->GetDCAgammaHisto()->Fill( *(fDCAgamma.at(m)) );
    }

  fRpositrons.clear();
  fPpositrons.clear();
  fIDpositronParent.clear();

  fPelectrons.clear();
  fIDelectronParent.clear();

  fRgamma.clear();
  fPgamma.clear();

  fDCAgamma.clear();
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void EventAction::AddScintBarsHits(ScintBarHitsCollection* BHC)
{
  TClonesArray& hitarray   = *(fRunAction->GetScintBarsHitsArray());
  TClonesArray& mchitarray = *(fRunAction->GetScintBarsMCHitsArray());

  // Add Scintillators Hits -- Digitization
  for(int i=0;i<BHC->entries();++i)
    {
      ScintBarHit* aHit = (*BHC)[i];
      //      aHit->PrintPolar();
      new(hitarray[i]) TScintDigi(aHit->GetTrackID(),
				  aHit->GetPDGcode(),
				  aHit->GetPosition().phi(),
				  aHit->GetTime()/ns,
				  aHit->GetPosition().perp()/mm,
				  aHit->GetPosition().z()/mm,
				  aHit->GetEdep()/MeV);
      
      // G4cout<<"\t\t"<<i<<"\t"
      // 	    <<((TScintDigi*) hitarray.Last())->GetR()<<"\t"
      // 	    <<((TScintDigi*) hitarray.Last())->GetPhi()<<"\t";
      ((TScintDigi*) hitarray.Last())->Digi();
      // G4cout<<((TScintDigi*) hitarray.Last())->GetBar()<<"\t"
      // 	    <<((TScintDigi*) hitarray.Last())->GetPos()<<"\t"
      // 	    <<((TScintDigi*) hitarray.Last())->GetTime()<<"\t"
      // 	    <<((TScintDigi*) hitarray.Last())->GetZ()
      // 	    <<G4endl;

      if( gmcbars )
	new(mchitarray[i]) TMChit(aHit->GetTrackID(),
				  aHit->GetPDGcode(),
				  aHit->GetPosition().perp()/mm,
				  aHit->GetPosition().phi(),
				  aHit->GetPosition().z()/mm,
				  aHit->GetTime()/ns);
    }

  // Remove duplicated digitized Scintillators Hits
  int entries = hitarray.GetEntries(), j=0;
  while( j < entries )
    {
      TScintDigi* digi_temp = (TScintDigi*) hitarray.ConstructedAt(j);
      for(int i=j+1; i<hitarray.GetEntries(); ++i)
	{
	  TScintDigi* digi_new = (TScintDigi*) hitarray.ConstructedAt(i);
	  if(digi_temp->IsSameDigi(digi_new)) 
	    {
	      // digi_temp->PrintChannel();
	      // G4cout<<"EventAction::AddScintBarsHits() Removing:"<<G4endl;
	      // digi_new->PrintChannel();
	      hitarray.RemoveAt(i);
	      --entries;
	    }
	  else break;
	}
      hitarray.Compress();
      ++j;
    }
  
}


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
