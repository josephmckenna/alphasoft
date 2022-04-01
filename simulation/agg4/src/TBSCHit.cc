// Scintillating Bars Digitization Class Implementation
//-----------------------------------------------------
// Author: A.Capra   Jan 2016
//-----------------------------------------------------

#include "TBSCHit.hh"

#include <iostream>
#include <cmath>
#include <cassert>

#include <TMath.h>
#include <TRandom3.h>

extern int gNbars;
extern double gBarRadius; // mm

TBSCHit::TBSCHit(int id, int pdg, double phi, double t):
                    fID(id),
                    fPDG(pdg),
					ft(t),
                    ft_in(nan("")),
                    ft_out(nan("")),
					fPos(-1.),fBar(-1),fNhits(0),fMultiTrack(0),
                    fMotherID(-999),
                    fpx_in(nan("")),
                    fpy_in(nan("")),
                    fpz_in(nan("")),
                    fpx_out(nan("")),
                    fpy_out(nan("")),
                    fpz_out(nan("")),
                    fCentreR(gBarRadius),
					fEnergy(0.),
					fMultiplicity(0)
{
  // since atan2 returns in [-pi,pi], rotate to get [0,2pi)
  if(phi<0.)
    phi+=TMath::TwoPi();
  fp=phi;

  // angular resolution: each bar covers a fraction (sigma) of 2pi
  //fSigmaPhi = TMath::TwoPi()/(double) gNbars; 
}
  
// TBSCHit::TBSCHit(int id, int pdg,
// 		       double phi, double t, 
// 		       double r, double z):fID(id),fPDG(pdg),
// 					   ft(t),
// 					   ft_in(nan("")),
//              ft_out(nan("")),
// 					   fPos(-1.),fBar(-1),fNhits(0),fMultiTrack(0),
//              fMotherID(-999),
//               fpx_in(nan("")),
//               fpy_in(nan("")),
//               fpz_in(nan("")),
//               fpx_out(nan("")),
//               fpy_out(nan("")),
//               fpz_out(nan("")),
//               fz(z),fr(r),
// 					   fSmearZ(-99999.),fSigmaZ(3.),
// 					   fCentreR(gBarRadius),
// 					   fEnergy(0.),
// 					   fMultiplicity(0)
// {  
//   // since atan2 returns in [-pi,pi], rotate to get [0,2pi)
//   if(phi<0.)
//     phi+=TMath::TwoPi();
//   fp=phi;  

//   // angular resolution: each bar covers a fraction (sigma) of 2pi
//   //fSigmaPhi = TMath::TwoPi()/(double) gNbars; 
// }

TBSCHit::TBSCHit(int id, int pdg,
		       double phi, double t, 
		       double Edep):
                fID(id),
                fPDG(pdg),
			    ft(t),
                ft_in(nan("")),
                ft_out(nan("")),
				fPos(-1.),fBar(-1),fNhits(0),fMultiTrack(0),
                fMotherID(-999),
                fpx_in(nan("")),
                fpy_in(nan("")),
                fpz_in(nan("")),
                fpx_out(nan("")),
                fpy_out(nan("")),
                fpz_out(nan("")),
                fCentreR(gBarRadius),
				fEnergy(Edep),
				fMultiplicity(0)
{  
  // since atan2 returns in [-pi,pi], rotate to get [0,2pi)
  if(phi<0.)
    phi+=TMath::TwoPi();
  fp=phi;  

  // angular resolution: each bar covers a fraction (sigma) of 2pi
  //fSigmaPhi = TMath::TwoPi()/(double) gNbars; 
}


void TBSCHit::PrintChannel()
{
  std::cout<<"*** TBSCHit ***   "<<fID<<" "<<fPDG<<std::endl;
  std::cout<<"ch: "<<fBar
	   <<" --  Mutliplicity "<<fMultiplicity<<std::endl;
  std::cout<<"pos = ("<<fCentreR<<", "<<fPos<<")"<<std::endl;
}

ClassImp(TBSCHit)
