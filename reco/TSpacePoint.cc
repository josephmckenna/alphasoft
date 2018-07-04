// SpacePoint class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Nov 2014

#include <iostream>
#include <iomanip>

#include "TSpacePoint.hh"
#include <TMath.h>

#include "TPCconstants.hh"

TSpacePoint::TSpacePoint():fw(-1),fp(-1),ft(kUnknown),fH(kUnknown),
			   fx(kUnknown),fy(kUnknown),fz(kUnknown),
			   fr(kUnknown),fphi(kUnknown),
			   ferrx(kUnknown),ferry(kUnknown),ferrz(kUnknown),
			   ferrr(kUnknown),ferrphi(kUnknown)
{}

TSpacePoint::TSpacePoint(int w, int s, int i, 
			 double t,
			 double r, double phi, double z,
			 double er, double ep, double ez,
			 double H):fw(w),ft(t),fH(H),
				   fz(z),fr(r),
				   ferrz(ez)
{
  fp = s+i*_padcol; // pad uniq index

  double pos = _anodepitch * ( double(w) + 0.5 ); // point position = anode position
  fphi = pos - phi; // lorentz correction
  
  fx = fr*TMath::Cos( fphi );
  fy = fr*TMath::Sin( fphi );

  // sigma_t=time_bin/sqrt(12)
  double errt = _sq12*_timebin;
  // sigma_r=(dr/dt)*sigma_t
  ferrr = TMath::Abs(er)*errt;
  // sigma_phi = sqrt( anode_pitch**2/12 + sigma_lorentz**2 );
  // sigma_lorentz=(dlorentz/dt)*sigma_t
  ferrphi = TMath::Sqrt(_anodepitch*_anodepitch/12. + ep*ep*errt*errt);  

  // sigma_x and simg_y in quadrature
  double x2=fx*fx, y2=fy*fy, r2=fr*fr,
    err2r=ferrr*ferrr, err2phi=ferrphi*ferrphi;
  ferrx = TMath::Sqrt(x2*err2r/r2+y2*err2phi);
  ferry = TMath::Sqrt(y2*err2r/r2+x2*err2phi);
}

// TSpacePoint::TSpacePoint(int w, int p, double t,
// 			 double r, double phi,
// 			 double er,
// 			 double H):fw(w),fp(p), 
// 				   ft(t),fH(H),
// 				   fr(r)
// {
//   double pos = _anodepitch * ( double(w) + 0.5 );
//   fphi = pos - phi;
  
//   fx = fr*TMath::Cos( fphi );
//   fy = fr*TMath::Sin( fphi );

//   double z = ( double(p) + 0.5 ) * _padpitch;
//   fz = z - _halflength;

//   ferrr = TMath::Abs(er);//*_sq12*_timebin;
  
//   ferrphi = _sq12*_anodepitch;
//   ferrz = _sq12*_padpitch;

//   double x2=fx*fx, y2=fy*fy, r2=fr*fr,
//     err2r=ferrr*ferrr, err2phi=ferrphi*ferrphi;
//   ferrx = TMath::Sqrt(x2*err2r/r2+y2*err2phi);
//   ferry = TMath::Sqrt(y2*err2r/r2+x2*err2phi);
// }

// TSpacePoint::TSpacePoint(int w, int s, int i, double t,
// 			 double r, double phi,
// 			 double er,
// 			 double H):fw(w), 
// 				   ft(t),fH(H),
// 				   fr(r)
// {
//   double pos = _anodepitch * ( double(w) + 0.5 );
//   fphi = pos - phi;
  
//   fx = fr*TMath::Cos( fphi );
//   fy = fr*TMath::Sin( fphi );

//   fp = s+i*_padcol;

//   double z = ( double(i) + 0.5 ) * _padpitch;
//   fz = z - _halflength;

//   ferrr = TMath::Abs(er);//*_sq12*_timebin;
  
//   ferrphi = _sq12*_anodepitch;
//   ferrz = _sq12*_padpitch;

//   double x2=fx*fx, y2=fy*fy, r2=fr*fr,
//     err2r=ferrr*ferrr, err2phi=ferrphi*ferrphi;
//   ferrx = TMath::Sqrt(x2*err2r/r2+y2*err2phi);
//   ferry = TMath::Sqrt(y2*err2r/r2+x2*err2phi);
// }

TSpacePoint::TSpacePoint(double x, double y, double z,
			 double ex, double ey, double ez):fw(-1), fp(-1), 
							  ft(kUnknown),fH(999999.),
							  fx(x), fy(y), fz(z),
							  ferrx(ex), ferry(ey), ferrz(ez)
{
    fphi = TMath::ATan2(fy,fx);
    //    if(fphi<0.) fphi+=TMath::TwoPi();
    fr = TMath::Sqrt(fx*fx+fy*fy);
}

double TSpacePoint::Distance(TSpacePoint* aPoint) const
{
  double dx = fx-aPoint->fx,
    dy = fy-aPoint->fy,
    dz = fz-aPoint->fz;
  return TMath::Sqrt(dx*dx+dy*dy+dz*dz);
}


double TSpacePoint::MeasureRad(TSpacePoint* aPoint) const
{
  return TMath::Abs(fr-aPoint->fr);
}

double TSpacePoint::MeasurePhi(TSpacePoint* aPoint) const
{
  double dist=kUnknown,
    phi1=fphi,phi2=aPoint->fphi;
  if( phi1 < 0. )
    phi1+=TMath::TwoPi();
  if( phi2 < 0. )
    phi2+=TMath::TwoPi();

  if( phi1 >= 0. && phi2 >=0. ) dist = TMath::Abs(phi1-phi2);

  return dist;
}

double TSpacePoint::MeasureZed(TSpacePoint* aPoint) const
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

bool TSpacePoint::IsGood(const double& rmin, const double& rmax) const
{
  if( fw<0 || fw>255 ) return false;
  // fp;
  else if( ft < 0. ) return false;
  // fH;
  else if( TMath::IsNaN(fx) || TMath::IsNaN(fy)) return false;
  else if( fz == kUnknown ) return false;
  else if( fr < rmin || fr > rmax ) return false;
  // fphi;
  else if( TMath::IsNaN(ferrx) || TMath::IsNaN(ferry) || TMath::IsNaN(ferrr) )
    return false;
  else if( ferrz == kUnknown ) return false;
  // ferrphi;
  else return true;

  return true;
}

int TSpacePoint::Check(const double& rmin, const double& rmax) const
{
  if( fw<0 || fw>255 ) return -1;
  // fp;
  else if( ft < 0. ) return -2;
  // fH;
  else if( TMath::IsNaN(fx) || TMath::IsNaN(fy)) return -3;
  else if( fz == kUnknown ) return -4; // no z value
  else if( fr < rmin || fr > rmax ) return -5; // r outside detector volume
  // fphi;
  else if( TMath::IsNaN(ferrx) || TMath::IsNaN(ferry) || TMath::IsNaN(ferrr) )
    return -6;
  else if( ferrz == kUnknown ) return -7;
  // ferrphi;
  else return 1;

  return 1;
}

void TSpacePoint::Print(Option_t* opt) const
{
  std::cout<<"TSpacePoint @ t = "<<std::setw(5)<<std::left<<ft<<" ns "
	   <<"on anode: "<<std::setw(5)<<std::left<<fw
	   <<" V = "<<std::setw(5)<<std::left<<fH
	   <<" pad "<<std::setw(5)<<std::left<<fp<<std::endl;
  if( !strcmp(opt,"xy") )
    {
      std::cout<<"\t(x,y,z) = ("
	       <<std::setw(5)<<std::left<<fx<<", "
	       <<std::setw(5)<<std::left<<fy<<", "
	       <<std::setw(5)<<std::left<<fz<<")";
      std::cout<<"\t(Ex,Ey,Ez) = ("
	       <<std::setw(5)<<std::left<<ferrx<<", "
	       <<std::setw(5)<<std::left<<ferry<<", "
	       <<std::setw(5)<<std::left<<ferrz<<")"<<std::endl;
    }
  else if( !strcmp(opt,"rphi") )
    {
      std::cout<<"\t(r,phi,z) = ("
	       <<std::setw(5)<<std::left<<fr<<", "
	       <<std::setw(5)<<std::left<<fphi<<", "
	       <<std::setw(5)<<std::left<<fz<<")";
      std::cout<<"\t(Er,Ephi,Ez) = ("
	       <<std::setw(5)<<std::left<<ferrr<<", "
	       <<std::setw(5)<<std::left<<ferrphi<<", "
	       <<std::setw(5)<<std::left<<ferrz<<")"<<std::endl;
    }
  else std::cout<<"Unknown coordinate system"<<std::endl;
}

ClassImp(TSpacePoint)
