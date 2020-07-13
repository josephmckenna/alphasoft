// SpacePoint class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Nov 2014

#include <iostream>
#include <iomanip>

#include "TSpacePoint.hh"
#include <TMath.h>

#include "TPCconstants.hh"

TSpacePoint::TSpacePoint():TObject(),
			   fw(-1),fp(-1),ft(agUnknown),fH(agUnknown),
			   fx(agUnknown),fy(agUnknown),fz(agUnknown),
			   fr(agUnknown),fphi(agUnknown),
			   ferrx(agUnknown),ferry(agUnknown),ferrz(agUnknown),
			   ferrr(agUnknown),ferrphi(agUnknown)
{}

TSpacePoint::TSpacePoint(const TSpacePoint &p):TObject(p)
{
  fw=p.fw;
  fp=p.fp;
  ft=p.ft;
  fH=p.fH;
  fx=p.fx;
  fy=p.fy;
  fz=p.fz;
  fr=p.fr;
  fphi=p.fphi;

  ferrx=p.ferrx;
  ferry=p.ferry;
  ferrz=p.ferrz;
  ferrr=p.ferrr;
  ferrphi=p.ferrphi;
}

TSpacePoint::TSpacePoint(int w, int s, int i, 
			 double t,
			 double r, double phi, double z,
			 double er, double ep, double ez,
			 double H):TObject(),
				   fw(w),ft(t),fH(H),
				   fz(z),fr(r)
{
  fp = s+i*32; // pad uniq index
  // if( ez == agUnknown )
  //   ferrz = _sq12*_padpitch;
  // else
  ferrz = ez;

  double pos = _anodepitch * ( double(fw) + 0.5 ); // point position = anode position
  fphi = pos - phi; // lorentz correction
  if( fphi < 0. ) fphi += TMath::TwoPi();
  if( fphi >= TMath::TwoPi() )
    fphi = fmod(fphi,TMath::TwoPi());

  //If available, calculate sin and cos in the same instruction:
  #ifdef _GNU_SOURCE
    sincos(fphi,&fy,&fx);
    fy=fr*fy;
    fx=fr*fx;
  #else
    fy = fr*TMath::Sin( fphi );
    fx = fr*TMath::Cos( fphi );
  #endif
  double errt = _sq12*_timebin;
  ferrr = TMath::Abs(er)*errt;
  ferrphi = TMath::Sqrt(_anodepitch*_anodepitch/12. + ep*ep*errt*errt);  

  // sigma_x and simg_y in quadrature
  double x2=fx*fx, y2=fy*fy, r2=fr*fr,
    err2r=ferrr*ferrr, err2phi=ferrphi*ferrphi;
  ferrx = TMath::Sqrt(x2*err2r/r2+y2*err2phi);
  ferry = TMath::Sqrt(y2*err2r/r2+x2*err2phi);
}

void TSpacePoint::Setup(int w, int s, int i, 
			double t,
			double r, double phi, double z,
			double er, double ep, double ez,
			double H)
{
  fw=w;
  ft=t;
  fH=H;
  fz=z;
  fr=r;
  fp = s+i*32; // pad uniq index
  // if( ez == agUnknown )
  //   ferrz = _sq12*_padpitch;
  // else
  ferrz = ez;

  double pos = _anodepitch * ( double(fw) + 0.5 ); // point position = anode position
  fphi = pos - phi; // lorentz correction
  if( fphi < 0. ) fphi += TMath::TwoPi();
  if( fphi >= TMath::TwoPi() )
    fphi = fmod(fphi,TMath::TwoPi());

  //If available, calculate sin and cos in the same instruction:
  #ifdef _GNU_SOURCE
    sincos(fphi,&fy,&fx);
    fy=fr*fy;
    fx=fr*fx;
  #else
    fy = fr*TMath::Sin( fphi );
    fx = fr*TMath::Cos( fphi );
  #endif
  double errt = _sq12*_timebin;
  ferrr = TMath::Abs(er)*errt;
  ferrphi = TMath::Sqrt(_anodepitch*_anodepitch/12. + ep*ep*errt*errt);  

  // sigma_x and simg_y in quadrature
  double x2=fx*fx, y2=fy*fy, r2=fr*fr,
    err2r=ferrr*ferrr, err2phi=ferrphi*ferrphi;
  ferrx = TMath::Sqrt(x2*err2r/r2+y2*err2phi);
  ferry = TMath::Sqrt(y2*err2r/r2+x2*err2phi);
}

void TSpacePoint::Setup(int w, int s, int i, 
			double t, double pos,
			double r, double phi, double z,
			double epos,
			double er, double ep, double ez,
			double H)
{
  fw=w;
  ft=t;
  fH=H;
  fz=z;
  fr=r;
  fp = s+i*32; // pad uniq index
 
  ferrz = ez;

  fphi = pos - phi; // lorentz correction
  if( fphi < 0. ) fphi += TMath::TwoPi();
  if( fphi >= TMath::TwoPi() )
    fphi = fmod(fphi,TMath::TwoPi());

  //If available, calculate sin and cos in the same instruction:
  #ifdef _GNU_SOURCE
    sincos(fphi,&fy,&fx);
    fy=fr*fy;
    fx=fr*fx;
  #else
    fy = fr*TMath::Sin( fphi );
    fx = fr*TMath::Cos( fphi );
  #endif
  double errt = _sq12*_timebin;
  ferrr = TMath::Abs(er)*errt;
  ferrphi = TMath::Sqrt( epos*epos + ep*ep*errt*errt);  

  // sigma_x and simg_y in quadrature
  double x2=fx*fx, y2=fy*fy, r2=fr*fr,
    err2r=ferrr*ferrr, err2phi=ferrphi*ferrphi;
  ferrx = TMath::Sqrt(x2*err2r/r2+y2*err2phi);
  ferry = TMath::Sqrt(y2*err2r/r2+x2*err2phi);
}

TSpacePoint::TSpacePoint(double x, double y, double z,
			 double ex, double ey, double ez):fw(-1), fp(-1), 
							  ft(agUnknown),fH(999999.),
							  fx(x), fy(y), fz(z),
							  ferrx(ex), ferry(ey), ferrz(ez)
{
    fphi = TMath::ATan2(fy,fx);
    //    if(fphi<0.) fphi+=TMath::TwoPi();
    fr = TMath::Sqrt(fx*fx+fy*fy);
}

double TSpacePoint::MeasureRad(TSpacePoint* aPoint) const
{
  return TMath::Abs(fr-aPoint->fr);
}

double TSpacePoint::MeasurePhi(TSpacePoint* aPoint) const
{
  double dist=agUnknown,
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

bool TSpacePoint::RadiusOrder(TSpacePoint* ip, TSpacePoint* jp)
{
  return ip->fr < jp->fr;
}

bool TSpacePoint::IsGood(const double& rmin, const double& rmax) const
{
  if( fw<0 || fw>255 ) return false;
  // fp;
  else if( ft < 0. ) return false;
  // fH;
  else if( TMath::IsNaN(fx) || TMath::IsNaN(fy)) return false;
  else if( fz == agUnknown ) return false;
  else if( fr < rmin || fr > rmax ) return false; // r outside fiducial volume
  // fphi;
  else if( TMath::IsNaN(ferrx) || TMath::IsNaN(ferry) || TMath::IsNaN(ferrr) )
    return false;
  else if( ferrz == agUnknown ) return false;
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
  else if( fz == agUnknown ) return -4; // no z value
  else if( fr < rmin || fr > rmax ) return -5; // r outside fiducial volume
  // fphi;
  else if( TMath::IsNaN(ferrx) || TMath::IsNaN(ferry) || TMath::IsNaN(ferrr) )
    return -6;
  else if( ferrz == agUnknown ) return -7;
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
