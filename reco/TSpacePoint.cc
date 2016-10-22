// SpacePoint class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Nov 2014

#include "TSpacePoint.hh"
#include "TDigi.hh"
#include "TLookUpTable.hh"

#include <TMath.h>

extern double gROradius;
extern double gPadTime;
extern double gPadZed;
extern double gAnglePitch;

TSpacePoint::TSpacePoint():fw(-1),fp(-1),ft(-99999.),
			   fx(0),fy(0),fz(0),fr(0),fphi(0),
			   ferrx(0),ferry(0),ferrz(0),ferrr(0),ferrphi(0),
			   fMCid(0),fPDG(0)
{}

TSpacePoint::TSpacePoint(TDigi* aDigi)
{
  ft = aDigi->GetDigiTime();
  fr = TLookUpTable::LookUpTableInstance()->GetRadius(ft);
  double dphi = TLookUpTable::LookUpTableInstance()->GetAzimuth(ft); // Lorentz Angle
  fphi = ( aDigi->GetDigiRphi() / gROradius ) - dphi;

  fx = fr*TMath::Cos( fphi );
  fy = fr*TMath::Sin( fphi );
  fz = aDigi->GetDigiZed();

  double sq12=1./TMath::Sqrt(12.);
  double errt = sq12*aDigi->GetPadTime();

  ferrr = TLookUpTable::LookUpTableInstance()->GetdRdt(ft)*errt;

  ferrphi = sq12*aDigi->GetPadRPhi() / gROradius;
  ferrz = sq12*aDigi->GetPadZ();

  double x2=fx*fx, y2=fy*fy, r2=fr*fr,
    err2r=ferrr*ferrr, err2phi=ferrphi*ferrphi;
  ferrx = TMath::Sqrt(x2*err2r/r2+y2*err2phi);
  ferry = TMath::Sqrt(y2*err2r/r2+x2*err2phi);

  fw = aDigi->GetChannelRphi();
  fp = aDigi->GetChannelZed();

  fMCid = aDigi->GetTrackID();
  fPDG = aDigi->GetTrackPDG();
}

TSpacePoint::TSpacePoint(int wire, int pad,
			 double t, double phi, double z):fw(wire),fp(pad),ft(t),
							 fMCid(0),fPDG(0)
{
  fr = TLookUpTable::LookUpTableInstance()->GetRadius(ft);
  double dphi = TLookUpTable::LookUpTableInstance()->GetAzimuth(ft); // Lorentz Angle
  fphi = phi - dphi;

  fx = fr*TMath::Cos( fphi );
  fy = fr*TMath::Sin( fphi );
  fz = z;

  double sq12=1./TMath::Sqrt(12.);
  double errt = gPadTime*sq12;

  ferrr = TLookUpTable::LookUpTableInstance()->GetdRdt(ft)*errt;

  ferrphi = sq12*gAnglePitch;
  ferrz = sq12*gPadZed;

  double x2=fx*fx, y2=fy*fy, r2=fr*fr,
    err2r=ferrr*ferrr, err2phi=ferrphi*ferrphi;
  ferrx = TMath::Sqrt(x2*err2r/r2+y2*err2phi);
  ferry = TMath::Sqrt(y2*err2r/r2+x2*err2phi);
}

TSpacePoint::TSpacePoint(int wire, int pad,
			 int id, int pdg,
			 double t, double phi, double z):fw(wire),fp(pad),ft(t),
							 fMCid(id),fPDG(pdg)
{
  fr = TLookUpTable::LookUpTableInstance()->GetRadius(ft);
  double dphi = TLookUpTable::LookUpTableInstance()->GetAzimuth(ft); // Lorentz Angle
  fphi = phi - dphi;

  fx = fr*TMath::Cos( fphi );
  fy = fr*TMath::Sin( fphi );
  fz = z;

  double sq12=1./TMath::Sqrt(12.);
  double errt = gPadTime*sq12;

  ferrr = TLookUpTable::LookUpTableInstance()->GetdRdt(ft)*errt;

  ferrphi = sq12*gAnglePitch;
  ferrz = sq12*gPadZed;

  double x2=fx*fx, y2=fy*fy, r2=fr*fr,
    err2r=ferrr*ferrr, err2phi=ferrphi*ferrphi;
  ferrx = TMath::Sqrt(x2*err2r/r2+y2*err2phi);
  ferry = TMath::Sqrt(y2*err2r/r2+x2*err2phi);
}

TSpacePoint::TSpacePoint(int w, int p, double t,
			 double x, double y, double z,
			 double ex, double ey, double ez,
			 double H):fw(w), fp(p), ft(t),
				   fx(x), fy(y), fz(z),
				   ferrx(ex), ferry(ey), ferrz(ez),
				   fH(H),
				   fMCid(0),fPDG(0)
{
    fphi = TMath::ATan2(fy,fx);
    fr = TMath::Sqrt(fx*fx+fy*fy);
}

TSpacePoint::TSpacePoint(double t,
			 double x, double y, double z,
			 double ex, double ey, double ez,
			 double H):fw(99999), fp(99999), ft(t),
				   fx(x), fy(y), fz(z),
				   ferrx(ex), ferry(ey), ferrz(ez),
				   fH(H),
				   fMCid(0),fPDG(0)
{
    fphi = TMath::ATan2(fy,fx);
    fr = TMath::Sqrt(fx*fx+fy*fy);
}

TSpacePoint::TSpacePoint(double x, double y, double z,
			 double ex, double ey, double ez,
			 double H):fw(99999), fp(99999), ft(-999999.),
				   fx(x), fy(y), fz(z),
				   ferrx(ex), ferry(ey), ferrz(ez),
				   fH(H),
				   fMCid(0),fPDG(0)
{
    fphi = TMath::ATan2(fy,fx);
    //    if(fphi<0.) fphi+=TMath::TwoPi();
    fr = TMath::Sqrt(fx*fx+fy*fy);
}

double TSpacePoint::Distance(TSpacePoint* aPoint)
{
  double dx = fx-aPoint->fx,
    dy = fy-aPoint->fy,
    dz = fz-aPoint->fz;
  return TMath::Sqrt(dx*dx+dy*dy+dz*dz);
}


double TSpacePoint::MeasureRad(TSpacePoint* aPoint)
{
  return TMath::Abs(fr-aPoint->fr);
}
double TSpacePoint::MeasurePhi(TSpacePoint* aPoint)
{
  double dist=-99999.,
    phi1=fphi,phi2=aPoint->fphi;
  if( phi1 < 0. )
    phi1+=TMath::TwoPi();
  if( phi2 < 0. )
    phi2+=TMath::TwoPi();

  if( phi1 >= 0. && phi2 >=0. ) dist = TMath::Abs(phi1-phi2);

  return dist;
}
double TSpacePoint::MeasureZed(TSpacePoint* aPoint)
{
  return TMath::Abs(fz-aPoint->fz);
}


int TSpacePoint::Compare(const TObject* aPoint) const
{
  if ( fr > ((TSpacePoint*) aPoint)->fr ) return -1;// smaller = large radius = earlier
  else if ( fr < ((TSpacePoint*) aPoint)->fr ) return 1;// larger = small radius = later
  else return 0;
}

// int TSpacePoint::Compare(const TObject* aPoint) const
// {
//   if ( ft < ((TSpacePoint*) aPoint)->ft ) return -1;
//   else if ( ft > ((TSpacePoint*) aPoint)->ft ) return 1;
//   else return 0;
// }

ClassImp(TSpacePoint)
