// Scintillating Bars Digitization Class Implementation
//-----------------------------------------------------
// Author: A.Capra   Jan 2016
//-----------------------------------------------------

#include "TScintDigi.hh"

#include <iostream>
#include <cmath>
#include <cassert>

#include <TMath.h>
#include <TRandom3.h>

extern int gNbars;
extern double gBarRadius; // mm

TScintDigi::TScintDigi(int id, int pdg, double phi, double t):fID(id),fPDG(pdg),
							      ft(t),
							      fPos(-1.),fBar(-1),
							      fz(-99999.),fr(-99999.),
							      fSmearZ(-99999.),fSigmaZ(10.),
							      fCentreR(gBarRadius),
							      fEnergy(0.),
							      fMultiplicity(0)
{
  // since atan2 returns in [-pi,pi], rotate to get [0,2pi)
  if(phi<0.)
    phi+=TMath::TwoPi();
  fp=phi;

  // angular resolution: each bar covers a fraction (sigma) of 2pi
  fSigmaPhi = TMath::TwoPi()/(double) gNbars; 
}
  
TScintDigi::TScintDigi(int id, int pdg,
		       double phi, double t, 
		       double r, double z):fID(id),fPDG(pdg),
					   ft(t),
					   fPos(-1.),fBar(-1),
					   fz(z),fr(r),
					   fSmearZ(-99999.),fSigmaZ(3.),
					   fCentreR(gBarRadius),
					   fEnergy(0.),
					   fMultiplicity(0)
{  
  // since atan2 returns in [-pi,pi], rotate to get [0,2pi)
  if(phi<0.)
    phi+=TMath::TwoPi();
  fp=phi;  

  // angular resolution: each bar covers a fraction (sigma) of 2pi
  fSigmaPhi = TMath::TwoPi()/(double) gNbars; 
}

TScintDigi::TScintDigi(int id, int pdg,
		       double phi, double t, 
		       double r, double z,
		       double Edep):fID(id),fPDG(pdg),
				    ft(t),
				    fPos(-1.),fBar(-1),
				    fz(z),fr(r),
				    fSmearZ(-99999.),fSigmaZ(3.),
				    fCentreR(gBarRadius),
				    fEnergy(Edep),
				    fMultiplicity(0)
{  
  // since atan2 returns in [-pi,pi], rotate to get [0,2pi)
  if(phi<0.)
    phi+=TMath::TwoPi();
  fp=phi;  

  // angular resolution: each bar covers a fraction (sigma) of 2pi
  fSigmaPhi = TMath::TwoPi()/(double) gNbars; 
}

// determine the bar hit from the MC phi position
void TScintDigi::Digi()
{
  // the first bar (bar0) is between -0.5*sigma and 0.5*sigma
  double bar0 = fSigmaPhi*0.5;
  
  // calculate the bar number as multiple of the angular resolution
  double digi = (fp+bar0)/fSigmaPhi;
  double chan;
  modf(digi, &chan);
  if( chan == gNbars ) chan=0.; // bar64 = bar0
  fBar = (int) chan;
  fPos = chan*fSigmaPhi;

  TRandom3 rndm;
  rndm.SetSeed(2016021616);
  fSmearZ=rndm.Gaus(fz,fSigmaZ);

  fMultiplicity=1;
}

bool TScintDigi::IsSameDigi(TScintDigi* right)
{
  assert(fMultiplicity>=1);
  if( fBar == right->fBar &&
      TMath::Abs( (right->fSmearZ - fSmearZ) / fSigmaZ ) < 3.0  )
    {
      ++fMultiplicity;
      fSmearZ = 0.5*(fSmearZ+right->fSmearZ);
      fEnergy += right->fEnergy;
      return true;
    }
  else
    return false;
}

void TScintDigi::PrintChannel()
{
  std::cout<<"*** TScintDigi ***   "<<fID<<" "<<fPDG<<std::endl;
  std::cout<<"ch: "<<fBar
	   <<" --  Mutliplicity "<<fMultiplicity<<std::endl;
  std::cout<<"pos = ("<<fCentreR<<", "<<fPos<<", "<<fSmearZ<<")"<<std::endl;
}

ClassImp(TScintDigi)
