// TPC Digitization Class
// simple method via arithmetics calculations
// saved to ROOT file (output of the simulation)
// used in the analysis
//------------------------------------------------
// Author: A.Capra   Nov 2014
//------------------------------------------------

#include "TDigi.hh"

#include <iostream>
#include <math.h>
#include <TMath.h>

#include "TPCBase.hh"

// extern double gROradius;
// extern double gTPCLength;
extern double gPadZed;
extern double gPadRphi;
extern double gPadTime;

TDigi::TDigi():fPadZed(0.),fPadRphi(0.),fPadTime(0.),
	       fCharge(1),fID(0),fPDG(0),
	       fChannelZed(-999),fChannelRphi(-999),fChannelTime(-999),
	       fHitZed(0),fHitRphi(0),fHitTime(0),
	       fDigiZed(-999.),fDigiRphi(-999.),fDigiTime(-999.)
{
  fROradius=TPCBase::TPCBaseInstance()->GetROradius(true);
  fChannelZedOffset  = (int) ( TPCBase::TPCBaseInstance()->GetFullLengthZ(true) / gPadZed );
  fChannelRphiOffset = (int) ( fROradius * TMath::Pi() / gPadRphi );
}

TDigi::TDigi(int id, int pdg, 
	     double zh, double hp, double ht):fPadZed(gPadZed),fPadRphi(gPadRphi),fPadTime(gPadTime),
					      fCharge(1),fID(id),fPDG(pdg),
					      fChannelZed(-999),fChannelRphi(-999),fChannelTime(-999),
					      fHitZed(zh),fHitRphi(hp),fHitTime(ht),
					      fDigiZed(-999.),fDigiRphi(-999.),fDigiTime(-999.)
{ 
  fROradius=TPCBase::TPCBaseInstance()->GetROradius(true);
  fChannelZedOffset  = (int) ( TPCBase::TPCBaseInstance()->GetFullLengthZ(true) / fPadZed );
  fChannelRphiOffset = (int) ( fROradius * TMath::Pi() / fPadRphi );  
}

TDigi::TDigi(double zh, double hp, double ht):fPadZed(gPadZed),fPadRphi(gPadRphi),fPadTime(gPadTime),
					      fCharge(1),fID(0),fPDG(0),
					      fChannelZed(-999),fChannelRphi(-999),fChannelTime(-999),
					      fHitZed(zh),fHitRphi(hp),fHitTime(ht),
					      fDigiZed(-999.),fDigiRphi(-999.),fDigiTime(-999.)
{ 
  fROradius=TPCBase::TPCBaseInstance()->GetROradius(true);
  fChannelZedOffset  = (int) ( TPCBase::TPCBaseInstance()->GetFullLengthZ(true) / fPadZed );
  fChannelRphiOffset = (int) ( fROradius * TMath::Pi() / fPadRphi );  
}

void TDigi::Digitization()
{
  if( fPadZed==0. || fPadRphi==0. || fPadTime==0.) return;

  double digi = fHitZed/fPadZed;
  double chan;
  double man  = modf(digi, &chan);
  if( TMath::Abs(man) < 0.5 )
    fChannelZed = TMath::FloorNint(digi) + fChannelZedOffset;
  else
    fChannelZed = TMath::CeilNint(digi) + fChannelZedOffset;
  
  digi = fHitRphi/fPadRphi; 
  man  = modf(digi, &chan);
  if( TMath::Abs(man) < 0.5 )
    fChannelRphi = TMath::FloorNint(digi) + fChannelRphiOffset;
  else
    fChannelRphi = TMath::CeilNint(digi) + fChannelRphiOffset;

  digi = fHitTime/fPadTime;
  man  = modf(digi, &chan);
  if( TMath::Abs(man) < 0.5 )
    fChannelTime = TMath::FloorNint(digi);
  else
    fChannelTime = TMath::CeilNint(digi);

  // given the channel, determine the position
  DigiPosition();
}

void TDigi::DigiPosition()
{
  if( fChannelZed==-999 || fChannelRphi==-999 || fChannelTime==-999) return;
  fDigiZed  = fPadZed  * (double) ( fChannelZed  - fChannelZedOffset  );
  fDigiRphi = fPadRphi * (double) ( fChannelRphi - fChannelRphiOffset );
  fDigiTime = fPadTime * (double) fChannelTime;
}

bool TDigi::IsSameDigi(TDigi* right)
{
  if( fID == right->fID )
    return IsSamePad( right->fChannelZed,
		      right->fChannelRphi,
		      right->fChannelTime );
  else return false;
}

bool TDigi::IsSamePad(int cz, int cp, int ct)
{
  if(fChannelZed==cz && fChannelRphi==cp && fChannelTime==ct)
    {
      ++fCharge;
      return true;
    }
  else return false;  
}

bool TDigi::IsSamePad(TDigi* right)
{
  if(fChannelZed  == right->fChannelZed  && 
     fChannelRphi == right->fChannelRphi &&
     fChannelTime == right->fChannelTime )
    {
      ++fCharge;
      return true;
    }
  else return false;  
}

void TDigi::PrintChannel()
{
  std::cout<<"*** TDigi ***   "<<fID<<" "<<fPDG<<std::endl;
  std::cout<<"ch: "<<fChannelZed
	   <<"    "<<fChannelRphi
	   <<"    "<<fChannelTime
	   <<" --  Charge "<<fCharge<<std::endl;
  std::cout<<"pos = ("<<fDigiZed<<", "<<fDigiRphi<<", "<<fDigiTime<<")"<<std::endl;
}

int TDigi::Compare(const TObject* aDigi) const
{
  if ( fDigiTime < ((TDigi*) aDigi)->fDigiTime ) return -1;
  else if ( fDigiTime > ((TDigi*) aDigi)->fDigiTime ) return 1;
  else return 0;
}

ClassImp(TDigi)
