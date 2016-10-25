// Helix class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: May 2014

#include "TFitHelix.hh"
#include "TDigi.hh"
#include "TSpacePoint.hh"
#include "TMath.h"
#include "TPCBase.hh"

#include <iostream>
#include <iomanip>

#include <TMinuit.h>

#include <TMatrixDSym.h>
#include <TMatrixD.h>

static TMinuit* rfitter=0;
void RadFunc(int&, double*, double& chi2, double* p, int)
{
  TFitHelix* fitObj = (TFitHelix*) rfitter->GetObjectFit();
  const TObjArray* PointsColl = fitObj->GetPointsArray();
  if(PointsColl->GetEntries()==0) return;

  TSpacePoint* apnt=0;
  double r,tx,ty,d2;
  chi2=0.;
  for(int i=0; i<PointsColl->GetEntries(); ++i)
    {
      apnt=(TSpacePoint*) PointsColl->At(i);
      r=apnt->GetR();
      TVector2 f = fitObj->Evaluate(r*r, p[0], p[1], p[2]);
      tx = ( apnt->GetX() - f.X() ) / apnt->GetErrX();
      ty = ( apnt->GetY() - f.Y() ) / apnt->GetErrY();
      d2 = tx*tx + ty*ty;
      //      d2 = tx*tx + ty*ty - tx*ty;
      chi2+=d2;
    }
  apnt=0;
  return;
}

static TMinuit* rfitter_=0;
void RadFunc_(int&, double*, double& chi2, double* p, int)
{
  TFitHelix* fitObj = (TFitHelix*) rfitter_->GetObjectFit();
  const TObjArray* PointsColl = fitObj->GetPointsArray();
  if(PointsColl->GetEntries()==0) return;

  TSpacePoint* apnt=0;
  double r,tx,ty,d2;
  chi2=0.;
  for(int i=0; i<PointsColl->GetEntries(); ++i)
    {
      apnt=(TSpacePoint*) PointsColl->At(i);
      r=apnt->GetR();
      TVector2 f = fitObj->Evaluate_(r*r, p[0], p[1], p[2]);
      tx = ( apnt->GetX() - f.X() ) / apnt->GetErrX();
      ty = ( apnt->GetY() - f.Y() ) / apnt->GetErrY();
      d2 = tx*tx + ty*ty;
      //      d2 = tx*tx + ty*ty - tx*ty;
      chi2+=d2;
    }
  apnt=0;
  return;
}

static TMinuit* zfitter=0;
void ZedFunc(int&, double*, double& chi2, double* p, int)
{
  TFitHelix* fitObj = (TFitHelix*) zfitter->GetObjectFit();
  const TObjArray* PointsColl = fitObj->GetPointsArray();
  if(PointsColl->GetEntries()==0) return;

  TSpacePoint* apnt=0;
  double r,tz,s;
  chi2=0.;
  for(int i=0; i<PointsColl->GetEntries(); ++i)
    {
      apnt=(TSpacePoint*) PointsColl->At(i);
      r = apnt->GetR();
      s = fitObj->GetArcLength(r*r);
      double f = fitObj->Evaluate(s, p[0], p[1]);
      tz = (apnt->GetZ() - f )/ apnt->GetErrZ();
      chi2+=tz*tz;
    }
  apnt=0;
  return;
}

TFitHelix::TFitHelix(double B): gMagneticField(B),fDigi(0),fNdigi(0),
		       fPoints(0),fNpoints(0),
		       fBranch(0),fParticle(0),
		       fchi2R(0.),fStatR(-1),
		       fchi2Z(0.),fStatZ(-1),
		       fChi2RCut(15.),fChi2ZCut(8.),
		       fChi2RMin(1.),fChi2ZMin(0.5),
		       fcCut(16.e-3),fDCut(40.),
		       fpCut(15.),
		       fHelix(0),fStatus(-1),
                       faPoint(0)
{
  fResiduals.SetXYZ(0.0,0.0,0.0);
  fMomentum.SetXYZ(0.0,0.0,0.0);
  fMomentumError.SetXYZ(0.0,0.0,0.0);
}

TFitHelix::~TFitHelix()
{
  fDigi.Delete();
  fPoints.Delete();
  if(fHelix) delete fHelix;
}

int TFitHelix::AddDigi(TDigi* aDigi)
{
  fDigi.AddLast(aDigi);
  return ++fNdigi;
}

int TFitHelix::AddPoint(TSpacePoint* aPoint)
{
  fPoints.AddLast(aPoint);
  return ++fNpoints;
}

void TFitHelix::Fit()
{
  if(fNpoints<=fNpar) return;
  fStatus=0;

  // Set starting values for parameters
  static double vstart[fNpar];
  Initialization(vstart);

  //  char parName[fNpar][7] = {"c   ","phi0","D   ","lambda","z0  "};
  //  for(int i=0; i<fNpar; ++i)
  //    printf("-- i%s\t%lf\n",parName[i],vstart[i]);

  // Set step sizes for parameters
  static double step[fNpar] = {0.00001 , 0.001 , 0.001, 0.001 , 0.01};

  // ================ R FIT 1 ================

  rfitter = new TMinuit(fRNpar);
  rfitter->SetObjectFit(this);
  rfitter->SetFCN( RadFunc );

  double arglist[10];
  int ierflg = 0;

  rfitter->SetPrintLevel(-1);

  arglist[0] = 1;
  rfitter->mnexcm("SET ERR", arglist , 1, ierflg);

  rfitter->mnparm(0, "c",    vstart[0], step[0], 0,0,ierflg);
  rfitter->mnparm(1, "phi0", vstart[1], step[1], 0,0,ierflg);
  rfitter->mnparm(2, "D",    vstart[2], step[2], 0,0,ierflg);

  rfitter->mnexcm("CALL FCN", arglist, 1, ierflg);

  // Now ready for minimization step
  arglist[0] = 500;
  arglist[1] = 1.;
  rfitter->mnexcm("MIGRAD", arglist, 2, ierflg);
  //  rfitter->mnexcm("IMPROVE", arglist, 1, ierflg);
  //  rfitter->mnimpr();
  //  rfitter->mnmnos();

  double nused0,nused1,chi2;
  int npar, stat;
  // status integer indicating how good is the covariance
  //   0= not calculated at all
  //   1= approximation only, not accurate
  //   2= full matrix, but forced positive-definite
  //   3= full accurate covariance matrix
  rfitter->mnstat(chi2,nused0,nused1,npar,npar,stat);

  // ================ R FIT 2 ================

  rfitter_ = new TMinuit(fRNpar);
  rfitter_->SetObjectFit(this);
  rfitter_->SetFCN( RadFunc_ );

  rfitter_->SetPrintLevel(-1);

  arglist[0] = 1;
  rfitter_->mnexcm("SET ERR", arglist , 1, ierflg);

  rfitter_->mnparm(0, "c",    vstart[0], step[0], 0,0,ierflg);
  rfitter_->mnparm(1, "phi0", vstart[1], step[1], 0,0,ierflg);
  rfitter_->mnparm(2, "D",    vstart[2], step[2], 0,0,ierflg);

  rfitter_->mnexcm("CALL FCN", arglist, 1, ierflg);

  // Now ready for minimization step
  arglist[0] = 500;
  arglist[1] = 1.;
  rfitter_->mnexcm("MIGRAD", arglist, 2, ierflg);
  //  rfitter_->mnexcm("IMPROVE", arglist, 1, ierflg);
  //  rfitter_->mnimpr();
  //  rfitter_->mnmnos();

  double chi2_;
  int stat_;
  rfitter_->mnstat(chi2_,nused0,nused1,npar,npar,stat_);

  double errc,errphi0,errD;

  // ======== R FIT 1 or 2 ? ========
  if(chi2<chi2_)
    {
      rfitter->GetParameter(0,fc,     errc);
      rfitter->GetParameter(1,fphi0, errphi0);
      rfitter->GetParameter(2,fD,     errD);
      fStatR = stat;
      fchi2R = chi2;
      fBranch = 1;
      //      std::cout<<"RFIT1"<<std::endl;
    }
  else
    {
      rfitter_->GetParameter(0,fc,     errc);
      rfitter_->GetParameter(1,fphi0, errphi0);
      rfitter_->GetParameter(2,fD,     errD);
      fStatR = stat_;
      fchi2R = chi2_;
      fBranch = -1;
      //     std::cout<<"RFIT2"<<std::endl;
    }

  delete rfitter;
  delete rfitter_;

  ferr2c = errc*errc;
  ferr2phi0 = errphi0*errphi0;
  ferr2D = errD*errD;
  //  if(fphi0>=TMath::TwoPi())
  //    fphi0=TMath::TwoPi()-fphi0;
  fx0=-fD*TMath::Sin(fphi0);
  fy0=fD*TMath::Cos(fphi0);

  fa=-0.299792458*TMath::Sign(1.,fc)*gMagneticField;

  // ================ Z FIT  ================

  zfitter = new TMinuit(fZNpar);
  zfitter->SetObjectFit(this);
  zfitter->SetFCN( ZedFunc );

  zfitter->SetPrintLevel(-1);

  arglist[0] = 1;
  zfitter->mnexcm("SET ERR", arglist , 1, ierflg);

  zfitter->mnparm(0, "lambda", vstart[3], step[3], 0,0,ierflg);
  zfitter->mnparm(1, "z0",     vstart[4], step[4], 0,0,ierflg);

  zfitter->mnexcm("CALL FCN", arglist, 1, ierflg);

  // Now ready for minimization step
  arglist[0] = 500;
  arglist[1] = 1.;
  zfitter->mnexcm("MIGRAD", arglist, 2, ierflg);
  //     zfitter->mnimpr();
  //     zfitter->mnmnos();

  zfitter->mnstat(chi2,nused0,nused1,npar,npar,stat);

  double errlambda,errz0;
  zfitter->GetParameter(0,flambda, errlambda);
  zfitter->GetParameter(1,fz0,     errz0);
  fStatZ = stat;
  fchi2Z = chi2;
  delete zfitter;

  ferr2lambda = errlambda*errlambda;
  ferr2z0 = errz0*errz0;
}

// use analytical straight line through first and last digi
// to initialize helix canonical form
void TFitHelix::Initialization(double* Ipar)
{
  //  TSpacePoint* FirstPoint = (TSpacePoint*) fPoints.First();
  TSpacePoint* LastPoint = (TSpacePoint*) fPoints.Last();
  double x1 = LastPoint->GetX(),
    y1 = LastPoint->GetY(),
    z1 = LastPoint->GetZ(),
    phi1= LastPoint->GetPhi();

  //  TSpacePoint* LastPoint = (TSpacePoint*) fPoints.Last();
  TSpacePoint* FirstPoint = (TSpacePoint*) fPoints.First();
  double x2 = FirstPoint->GetX(),
    y2 = FirstPoint->GetY(),
    z2 = FirstPoint->GetZ();

  double dx = x2-x1, dy = y2-y1, dz=z2-z1,
         vr2=dx*dx+dy*dy, t = -(x1*dx+y1*dy)/vr2;
  double x0=dx*t+x1, y0=dy*t+y1, z0=dz*t+z1;
  //  std::cout<<"(x0,y0,z0) = ("<<x0<<","<<y0<<","<<z0<<") mm"<<std::endl;

  double r0 = TMath::Sqrt(x0*x0+y0*y0),
    phi0 = TMath::ATan2(-x0,y0) + TMath::Pi();

  double D = phi1>phi0?r0:-r0,
         l = dz/TMath::Sqrt(vr2);

  double curv=0.000418711,//(2R)^-1[mm^-1] = 0.5*10^-9*c*B[T]/p[MeV]
    pos=1.+2.*curv*D, ic;
  if(pos>0)
    ic=curv;
  else
    ic=-1.*curv;

  Ipar[0]=ic;
  Ipar[1]=phi0;
  Ipar[2]=D;
  Ipar[3]=l;
  Ipar[4]=z0;
}

// internal helix parameter
double TFitHelix::GetBeta(double r2, double c, double D)
{
  double num = r2-D*D,
    den = 1.+2.*c*D,
    arg=num/den;
  if(num>=0.)
    {
      double beta = TMath::Sqrt(arg)*c;
      return beta;
    }
  else return 0.;
}

double TFitHelix::GetArcLength(double r2, double c, double D)
{
  return TMath::ASin( GetBeta(r2,c,D) ) / c;
}

double TFitHelix::GetArcLength_(double r2, double c, double D)
{
  return ( TMath::Pi() - TMath::ASin( GetBeta(r2,c,D) ) ) / c;
}

// FitHelix Axial and FitVertex::FindSeed and FitVertex::Improve
double TFitHelix::GetArcLength(double r2)
{
  if(fBranch==1)
    return GetArcLength(r2,fc,fD);
  else if(fBranch==-1)
    return GetArcLength_(r2,fc,fD);
  else
    return 0;
}

// FitHelix Radial for +1 Branch
TVector2 TFitHelix::Evaluate(double r2, double c, double phi, double D)
{
  double u0 = TMath::Cos(phi),
    v0 = TMath::Sin(phi);
  double x0 = -D*v0,
    y0 = D*u0,
    beta = GetBeta(r2, c, D);
  double beta2 = beta*beta;
  TVector2 p( x0 + u0 * beta * TMath::Sqrt(1.-beta2) / c - v0 * beta2 / c,
	      y0 + v0 * beta * TMath::Sqrt(1.-beta2) / c + u0 * beta2 / c);
  return p;
}

// FitHelix Radial for -1 Branch
TVector2 TFitHelix::Evaluate_(double r2, double c, double phi, double D)
{
  double u0 = TMath::Cos(phi),
    v0 = TMath::Sin(phi);
  double x0 = -D*v0,
    y0 = D*u0,
    beta = GetBeta(r2, c, D);
  double beta2 = beta*beta;
  TVector2 p( x0 - u0 * beta * TMath::Sqrt(1.-beta2) / c - v0 * beta2 / c,
	      y0 - v0 * beta * TMath::Sqrt(1.-beta2) / c + u0 * beta2 / c);
  return p;
}

// FitHelix Axial
double TFitHelix::Evaluate(double s, double l, double z0)
{
  return z0 + l * s;
}

// FitHelix for +1 Branch
TVector3 TFitHelix::Evaluate(double r2, double c, double phi, double D, double l, double z0)
{
  double u0 = TMath::Cos(phi),
    v0 = TMath::Sin(phi);
  double x0 = -D*v0,
    y0 = D*u0,
    beta = GetBeta(r2, c, D);
  double beta2 = beta*beta;
  TVector3 p( x0 + u0 * beta * TMath::Sqrt(1.-beta2) / c - v0 * beta2 / c,
	      y0 + v0 * beta * TMath::Sqrt(1.-beta2) / c + u0 * beta2 / c,
	      z0 + l * GetArcLength(r2, c, D) );
  return p;
}

// FitHelix for -1 Branch
TVector3 TFitHelix::Evaluate_(double r2, double c, double phi, double D, double l, double z0)
{
  double u0 = TMath::Cos(phi),
    v0 = TMath::Sin(phi);
  double x0 = -D*v0,
    y0 = D*u0,
    beta = GetBeta(r2, c, D);
  double beta2 = beta*beta;
  TVector3 p( x0 - u0 * beta * TMath::Sqrt(1.-beta2) / c - v0 * beta2 / c,
	      y0 - v0 * beta * TMath::Sqrt(1.-beta2) / c + u0 * beta2 / c,
	      z0 + l * GetArcLength_(r2, c, D) );
  return p;
}

// Draw routine
TVector3 TFitHelix::Evaluate(double r2)
{
  double s= GetArcLength(r2);
  TVector2 r;
  if(fBranch==1)
    r.Set(Evaluate(r2, fc, fphi0, fD));
  else if(fBranch==-1)
    r.Set(Evaluate_(r2, fc, fphi0, fD));

  TVector3 p( r.X(), r.Y(), Evaluate(s,flambda,fz0) );
  return p;
}

// FitVertex
TVector3 TFitHelix::EvaluateErrors2(double r2)
{
  double beta = GetBeta(r2,fc,fD),
    beta2 = beta*beta,
    bb = beta*TMath::Sqrt(1.-beta2),
    cp=TMath::Cos(fphi0),
    sp=TMath::Sin(fphi0),
    c2=fc*fc,
    eps=(double) fBranch;

  double dxdc = -eps*bb*cp/c2 +beta2*sp/c2 ,
    dxdphi = -fD*cp -beta2*cp/fc -eps*bb*sp/fc,
    dxdD = -sp,
    dydc = -eps*bb*sp/c2 -beta2*cp/c2,
    dydphi = -fD*sp -beta2*sp/fc +eps*bb*cp/fc,
    dydD = cp,
    dzdl = GetArcLength(r2);

  TVector3 sigma2(dxdc*dxdc*ferr2c + dxdphi*dxdphi*ferr2phi0 + dxdD*dxdD*ferr2D,
		  dydc*dydc*ferr2c + dydphi*dydphi*ferr2phi0 + dydD*dydD*ferr2D,
		  dzdl*dzdl*ferr2lambda + ferr2z0);
  return sigma2;
}

// FitVertex
TVector3 TFitHelix::GetPosition(double s)
{
  double rho=2.*fc,
    cp=TMath::Cos(fphi0),sp=TMath::Sin(fphi0),
    crs=TMath::Cos(rho*s),srs=TMath::Sin(rho*s);

  TVector3 p(fx0+cp*srs/rho-sp*(1.-crs)/rho,
	     fy0+sp*srs/rho+cp*(1.-crs)/rho,
	     fz0+flambda*s);
  return p;
}

// FitVertex::EvaluateMeanPointError2
TVector3 TFitHelix::GetError2(double s)
{
  double rho=2.*fc,
    cp=TMath::Cos(fphi0),sp=TMath::Sin(fphi0),
    crs=TMath::Cos(rho*s),srs=TMath::Sin(rho*s);

  double dxdc = s*crs*cp/fc - cp*srs/rho/fc + (1.-crs)*sp/rho/fc - s*srs*sp/fc,
    dxdp = -fy0 - (1.-crs)*cp/rho -srs*sp/rho,
    dxdD = -sp,
    dydc = -(1.-crs)*cp/rho/fc + s*cp*srs/fc + s*crs*sp/fc - srs*sp/rho/fc,
    dydp = fx0 + cp*srs/rho - (1.-crs)*sp/rho,
    dydD = cp;
  TVector3 sigma2(dxdc*dxdc*ferr2c + dxdp*dxdp*ferr2phi0 + dxdD*dxdD*ferr2D,
		  dydc*dydc*ferr2c + dydp*dydp*ferr2phi0 + dydD*dydD*ferr2D,
		  s*s*ferr2lambda + ferr2z0);
  return sigma2;
}

double TFitHelix::Momentum()
{
  double coeff = 0.5*fa/fc,
    px=coeff*TMath::Cos(fphi0), // MeV/c
    py=coeff*TMath::Sin(fphi0),
    pz=coeff*flambda;
  fMomentum.SetXYZ(px,py,pz);
  double pT = fMomentum.Perp();
  double errc = TMath::Sqrt(ferr2c), errphi0 = TMath::Sqrt(ferr2phi0), errlambda = TMath::Sqrt(ferr2lambda);
  fMomentumError.SetXYZ(-px*errc/fc-py*errphi0,-py*errc/fc+px*errphi0,-pz*errc/fc+pT*errlambda);
  return pT;
}

double TFitHelix::GetApproxPathLength()
{
  double gInnerRadius = 10.*TPCBase::CathodeRadius;
  double gTrapRadius = 10.*TPCBase::TrapR;
  TVector3 r1(Evaluate(gInnerRadius*gInnerRadius));
  TVector3 r2(Evaluate(gTrapRadius*gTrapRadius));
  return TMath::Abs(r1.Mag()-r2.Mag());
}

double TFitHelix::VarMS()
{
  // Multiple scattering causes the particle to scatter in the two planes
  // perpendicular to its path without losing energy. The distribution of
  // each angle is gaussian with std.dev.
  //          0.0141         L
  // sigma = -------- sqrt( --- )
  //          p beta         X
  // where p is in GeV/c, beta its velocity, L path length and X radiation length

  double p2 = fMomentum.Mag2()*1.e-6, // (GeV/c)^2
     E2 = gChargedPionMass*gChargedPionMass+fMomentum.Mag2(),
    beta2 = fMomentum.Mag2()/E2,
    L=GetApproxPathLength();

  // return sigma^2 variance
  return 1.9881e-4*L/p2/beta2/gRadiationLength;
}

void TFitHelix::AddMSerror()
{
  TMatrixDSym W(4);
  double weights[]={
    1.,  0.,  0.5,   0.,
    0.,  1.,  0.,    0.5,
    0.5, 0.,  1./3., 0.,
    0.,  0.5, 0.,    1./3.};
  W.Use(4,weights);

  double r=10.*TPCBase::TrapR,
    r2=r*r,
    s=GetArcLength(r2),
    D2=fD*fD,
    cD=fc*fD,
    rho=2.*fc,
    lambda2=flambda*flambda,
    sintheta=1./TMath::Sqrt(1.+lambda2),
    costheta=flambda*sintheta;

  if(fMomentum.Mag()==0.0)
    Momentum();

  double pT=fMomentum.Perp(),
    HL = VarMS(),
    p=fMomentum.Mag(),
    T=pT*(1.+2.*cD),
    T2=T*T,
    pz=fMomentum.Z(),
    sina=r*fc-fD*(1.+cD)/r,
    cosa=TMath::Sqrt( (1.-D2/r2)*((1.+cD)*(1.+cD)-r2*fc*fc) );
  if( TMath::IsNaN(cosa) ) cosa=0.;
  else if(fBranch==-1) cosa*=-1.;

  TMatrixD M(5,4);
  double MSeffect[]={
    0.,                               0.5*fa*flambda/pT,         0.,                     0.,
    p*pT*(1.-rho*r*sina)/T2,          fa*flambda*pT*r*cosa/T2,   fa*p*r*cosa/T2,         fa*pz*sintheta*(1.-rho*r*sina)/T2,
    -p*r*cosa/T,                      -0.5*fa*flambda*(r2-D2)/T, p*(1.-rho*r*sina)/T,    fa*r*costheta*cosa/T,
    0.,                               -p*p/pT/pT,                0.,                     0.,
    -flambda*p*pT*(fD+fc*(D2+r2))/T2, s+lambda2*pT*pT*r*cosa/T2, fa*flambda*p*r*cosa/T2, sintheta*(-T2+pz*pz*(1.-rho*r*sina))/T2
  };

  M.Use(5,4,MSeffect);

  TMatrixD A(W,TMatrixD::kMultTranspose,M); // A = W*M^T
  TMatrixD V(M,TMatrixD::kMult,A); // V = M*A
  V*=HL; // V = (HL) M*W*M^T

  TMatrixDDiag Vdiag(V);
    ferr2c     +=Vdiag(0);
    ferr2phi0  +=Vdiag(1);
    ferr2D     +=Vdiag(2);
    ferr2lambda+=Vdiag(3);
    ferr2z0    +=Vdiag(4);
}

double TFitHelix::CalculateResiduals()
{
  TSpacePoint* aPoint=0;
  double r;
  for(int i=0; i<fPoints.GetEntries(); ++i)
    {
      aPoint = (TSpacePoint*) fPoints.At(i);

      TVector3 p(aPoint->GetX(),
		 aPoint->GetY(),
		 aPoint->GetZ());
      r=aPoint->GetR();
      fResiduals += p-Evaluate(r*r);
    }
  aPoint=0;
  return fResiduals.Mag();
}

int TFitHelix::TubeIntersection(TVector3& pos1, TVector3& pos2, double radius)
{
  if( TMath::Abs(fD) < radius )
    {
      double beta = GetBeta(radius*radius,fc,fD),
	s1, s2;
      if( fBranch == 1 )
	{
	  s1 = TMath::ASin(beta)/fc;
	  s2 = TMath::ASin(-beta)/fc;
	}
      else if( fBranch == -1 )
	{
	  s1 = (TMath::Pi()-TMath::ASin(beta))/fc;
	  s2 = (TMath::Pi()-TMath::ASin(-beta))/fc;
	}
      else
	{
	  std::cerr<<"TFitHelix::TubeIntersection FAIL type: Unknown Branch Type "<<fBranch<<std::endl;
	  return -1;
	}

      pos1 = GetPosition(s1);
      if( !TMath::AreEqualRel( pos1.Perp(), radius, 1.e-4) )
	{
	  double err = TMath::Abs(pos1.Perp()-radius),
	    avg = 0.5*(TMath::Abs(pos1.Perp())+TMath::Abs(radius));
	  double rel = err/avg;
	  std::cerr<<"TFitHelix::TubeIntersection FAIL type: position 1 radius: "<<pos1.Perp()<<" differs from parameter: "<<radius<<" by "<<rel<<std::endl;
	}
      pos2 = GetPosition(s2);
      if( !TMath::AreEqualRel( pos2.Perp(), radius, 1.e-4) )
	{
	  double err = TMath::Abs(pos2.Perp()-radius),
	    avg = 0.5*(TMath::Abs(pos2.Perp())+TMath::Abs(radius));
	  double rel = err/avg;
	  std::cerr<<"TFitHelix::TubeIntersection FAIL type: position 2 radius: "<<pos2.Perp()<<" differs from parameter: "<<radius<<" by "<<rel<<std::endl;
	}
      return 1;
    }
  else
    {
      pos1.SetXYZ(fx0,fy0,fz0);
      pos2.SetXYZ(fx0,fy0,fz0);
      return 0;
    }
}


static TMinuit* hel2pnt=0;
void Hel2PntFunc(int&, double*, double& chi2, double* p, int)
{
  TFitHelix* fitObj = (TFitHelix*) hel2pnt->GetObjectFit();
  TVector3* pnt  = fitObj->GetPoint();
  TVector3 h  = fitObj->GetPosition( p[0] );
  TVector3 e2 = fitObj->EvaluateErrors2( h.Perp2() );
  double tx=pnt->X()-h.X(), ty=pnt->Y()-h.Y(), tz=pnt->Z()-h.Z();
  chi2 = 0.0;
  chi2 += tx*tx/e2.X() + ty*ty/e2.Y() + tz*tz/e2.Z() ;
  return;
}

double TFitHelix::MinDistPoint(const TVector3* point, TVector3* minpoint)
{
  if(point)
    SetPoint((TVector3*)point);
  else
    return -9999999.;

  static double step = 1.e-9;
  hel2pnt = new TMinuit(1);
  hel2pnt->SetObjectFit(this);
  hel2pnt->SetFCN(Hel2PntFunc);

  double arglist[10];
  int ierflg = 0;

  hel2pnt->SetPrintLevel(-1);

  arglist[0] = 1;
  hel2pnt->mnexcm("SET ERR", arglist , 1, ierflg);

  double gTrapRadius = 10.*TPCBase::TrapR;
  double s_start = GetArcLength(gTrapRadius*gTrapRadius);
  hel2pnt->mnparm(0, "s", s_start, step, 0,0, ierflg);

  hel2pnt->mnexcm("CALL FCN", arglist, 1, ierflg);

  // Now ready for minimization step
  arglist[0] = 500;
  arglist[1] = 1.;
  hel2pnt->mnexcm("MIGRAD", arglist, 2, ierflg);

  double chi2,nused0, nused1;
  int npar, stat;
  // status integer indicating how good is the covariance
  //   0= not calculated at all
  //   1= approximation only, not accurate
  //   2= full matrix, but forced positive-definite
  //   3= full accurate covariance matrix
  hel2pnt->mnstat(chi2,nused0,nused1,npar,npar,stat);

  double s,es;
  hel2pnt->GetParameter(0,s,es);
  delete hel2pnt;

  TVector3 mpnt=GetPosition(s);
  minpoint->SetXYZ(mpnt.X(),mpnt.Y(),mpnt.Z());

  //  chi2*=0.5;
  return chi2;
}


bool TFitHelix::IsGood()
{
  // make sure that the fit succeeded
  if( fStatR <= 0 ) fStatus=-2;
  else if( fStatZ <= 0 ) fStatus=-3;
  // do not search for the vertex with bad helices
  //  if(chi2>fChi2RCut || chi2<=fChi2RMin) return 0;
  else if( (fchi2R/(double) GetRDoF()) > fChi2RCut ) fStatus=-4;
  //  if(chi2>fChi2ZCut || chi2<=fChi2ZMin) return 0;
  else if( (fchi2Z/(double) GetZDoF()) > fChi2ZCut ) fStatus=-5;
  else if( TMath::Abs(fD)  > fDCut && kDcut )        fStatus=-6;
  else if( TMath::Abs(fc)  > fcCut && kccut )        fStatus=-6;
  else if( fMomentum.Perp() < fpCut && kpcut )       fStatus=-6;
  //else if( fNpoints < 15 )                           fStatus=-7;
  else fStatus=1;

  if(fStatus>0)
    return true;
  else
    return false;
}

// bool TFitHelix::IsDuplicated(TFitHelix* right, double cut)
// {
//   //  int cnt=0;
//   for(int i=0; i<fPoints.GetEntries(); ++i)
//     {
//       TSpacePoint* pni = (TSpacePoint*) fPoints.At(i);
//       for(int j=0; j<right->fPoints.GetEntries(); ++j)
// 	{
// 	  TSpacePoint* pmj = (TSpacePoint*) right->fPoints.At(j);
// 	  if( pni->Distance(pmj) <= cut ) // it's a nearby point
// 	  // if( pni->GetTime() == pmj->GetTime() &&
// 	  //     pni->GetWire() == pmj->GetWire() )// && pni->GetPad()  == pmj->GetPad() )
// 	    return true;
// 	  //   ++cnt;

// 	  // if(cnt>=3) // three close points give duplicated helix
// 	    //	       return true;
// 	}// pnt j loop - hel m
//     }// pnt i loop - hel n

//   return false;
// }

bool TFitHelix::IsDuplicated(TFitHelix* right, double cut)
{
  if( TMath::AreEqualAbs(fMomentum.Phi(),right->GetMomentumV().Phi(),cut) &&
      TMath::AreEqualAbs(fMomentum.Theta(),right->GetMomentumV().Theta(),cut) )
    return true;
  else
    return false;
}

void TFitHelix::Print(Option_t*) const
{
  std::cout<<" *** TFitHelix ***"<<std::endl;
  std::cout<<" ("<<std::setw(5)<<std::left<<GetX0()
	   <<", "<<std::setw(5)<<std::left<<GetY0()
	   <<", "<<std::setw(5)<<std::left<<fz0<<")\n"
	   <<"    c = "<<std::setw(5)<<std::left<<fc
	   <<" Phi0 = "<<std::setw(5)<<std::left<<fphi0
	   <<"    D = "<<std::setw(5)<<std::left<<fD
	   <<"    L = "<<std::setw(5)<<std::left<<flambda
	   <<std::endl;
  std::cout<<"Branch : "<<fBranch<<std::endl;
  std::cout<<"Radial Chi2 = "<<fchi2R<<"\t ndf = "<<GetRDoF()<<"\t cov stat = "<<fStatR<<std::endl;
  std::cout<<" Axial Chi2 = "<<fchi2Z<<"\t ndf = "<<GetZDoF()<<"\t cov stat = "<<fStatZ<<std::endl;
  if(fMomentum.Mag()!=0.0)
    {
      std::cout<<" Momentum = ("
	       <<std::setw(5)<<std::left<<fMomentum.X()<<", "
	       <<std::setw(5)<<std::left<<fMomentum.Y()<<", "
	       <<std::setw(5)<<std::left<<fMomentum.Z()<<") MeV/c"<<std::endl;
      std::cout<<" |p| = "<<fMomentum.Mag()
	       <<" MeV/c\t pT = "<<fMomentum.Perp()<<" MeV/c"<<std::endl;
    }
  if(fResiduals.Mag()!=0.0)
    std::cout<<"  Residuals = ("
	     <<std::setw(5)<<std::left<<fResiduals.X()
	     <<", "<<std::setw(5)<<std::left<<fResiduals.Y()
	     <<", "<<std::setw(5)<<std::left<<fResiduals.Z()<<") mm"<<std::endl;
  if(fParticle!=0)
    std::cout<<"PDG code "<<fParticle<<std::endl;
  std::cout<<"Status: "<<fStatus<<std::endl;
  std::cout<<"--------------------------------------------------------------------------"<<std::endl;

}

void TFitHelix::Draw(Option_t*)
{
  //  if(fStatus<1) return;

  double gROradius = 10.*TPCBase::ROradius;
  double rho2i =0.,
    rho2f = (gROradius+1.)*(gROradius+1.),
    Npoints = 50.,
    rs = TMath::Abs(rho2f-rho2i)/Npoints;

  fHelix = new TPolyLine3D();
  for(double r2 = rho2i; r2 <= rho2f; r2 += rs)
    {
      TVector3 p = Evaluate(r2);
      fHelix->SetNextPoint(p.X(),p.Y(),p.Z());
    }

  if(fStatus==1) // good helix
    {
      fHelix->SetLineColor(kGreen);
      fHelix->SetLineWidth(2);
    }
  else if(fStatus==2) // seed
    {
      //fHelix->SetLineColor(kMagenta);
      fHelix->SetLineColor(9);
      fHelix->SetLineWidth(2);
    }
  else if(fStatus==3) // added
    {
      //fHelix->SetLineColor(kCyan);
      fHelix->SetLineColor(6);
      fHelix->SetLineWidth(2);
    }
  else // not good
    {
      //fHelix->SetLineColor(kGray);
      fHelix->SetLineColor(1);
      fHelix->SetLineWidth(2);
    }
}

int TFitHelix::Compare(const TObject* aHelix) const
{
  if(TMath::Abs(fc) < TMath::Abs(((TFitHelix*)aHelix)->fc)) return -1;
  else if(TMath::Abs(fc) > TMath::Abs(((TFitHelix*)aHelix)->fc)) return 1;
  else return 0;
}

ClassImp(TFitHelix)
