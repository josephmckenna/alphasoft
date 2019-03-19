// Helix class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: May 2014

#include "TFitHelix.hh"
#include "TSpacePoint.hh"

#include "TStoreHelix.hh"

#include <iostream>
#include <iomanip>

#include <TMath.h>
#include <TMinuit.h>

#include <TMatrixDSym.h>
#include <TMatrixD.h>

bool kDcut = true;
bool kccut = false;
bool kpcut = false;

static TMinuit* rfitter=0;
void RadFunc(int&, double*, double& chi2, double* p, int)
{
  TFitHelix* fitObj = (TFitHelix*) rfitter->GetObjectFit();
  const std::vector<TSpacePoint*>* PointsColl = fitObj->GetPointsArray();
  int pcol=PointsColl->size();
  if(pcol==0) return;

  TSpacePoint* apnt=0;
  double r,tx,ty,d2;
  chi2=0.;


  double u0 = TMath::Cos(p[1]),
    v0 = TMath::Sin(p[1]); //phi

  for(int i=0; i<pcol; ++i)
    {
      apnt=(TSpacePoint*) PointsColl->at(i);
      r=apnt->GetR();
      Vector2 f = fitObj->Evaluate(r*r, p[0], u0, v0, p[2]);
      tx = ( apnt->GetX() - f.X ) / apnt->GetErrX();
      ty = ( apnt->GetY() - f.Y ) / apnt->GetErrY();
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
  const std::vector<TSpacePoint*>* PointsColl = fitObj->GetPointsArray();
  int pcol=PointsColl->size();
  if(pcol==0) return;

  TSpacePoint* apnt=0;
  double r,tx,ty,d2;
  chi2=0.;
    double u0 = TMath::Cos(p[1]),
    v0 = TMath::Sin(p[1]); //phi

  for(int i=0; i<pcol; ++i)
    {
      apnt=(TSpacePoint*) PointsColl->at(i);
      r=apnt->GetR();
      Vector2 f = fitObj->Evaluate_(r*r, p[0], u0, v0, p[2]);
      tx = ( apnt->GetX() - f.X ) / apnt->GetErrX();
      ty = ( apnt->GetY() - f.Y ) / apnt->GetErrY();
      d2 = tx*tx + ty*ty;
      //      d2 = tx*tx + ty*ty - tx*ty;
      chi2+=d2;
    }
  apnt=0;
  return;
}

static TMinuit* rfitterPlus=0;
void RadFuncPlus(int&, double*, double& chi2, double* p, int)
{
  TFitHelix* fitObj = (TFitHelix*) rfitterPlus->GetObjectFit();
  const std::vector<TSpacePoint*>* PointsColl = fitObj->GetPointsArray();
  int pcol=PointsColl->size();
  if(pcol==0) return;
  
  TSpacePoint* apnt=0;
  double r,tx,ty,d2;
  chi2=0.;
      double u0 = TMath::Cos(p[1]),
    v0 = TMath::Sin(p[1]); //phi
  
  for(int i=0; i<pcol; ++i)
    {
      apnt=(TSpacePoint*) PointsColl->at(i);
      r=apnt->GetR();
      Vector2 f = fitObj->EvaluatePlus(r*r, p[0], u0, v0, p[2]);
      tx = ( apnt->GetX() - f.X ) / apnt->GetErrX(); 
      ty = ( apnt->GetY() - f.Y ) / apnt->GetErrY();
      d2 = tx*tx + ty*ty;
      //      d2 = tx*tx + ty*ty - tx*ty;
      chi2+=d2;
    }
  apnt=0;
  return;
}

static TMinuit* rfitterPlus_=0;
void RadFuncPlus_(int&, double*, double& chi2, double* p, int)
{
  TFitHelix* fitObj = (TFitHelix*) rfitterPlus_->GetObjectFit();
  const std::vector<TSpacePoint*>* PointsColl = fitObj->GetPointsArray();
  int pcol=PointsColl->size();
  if(pcol==0) return;

  TSpacePoint* apnt=0;
  double r,tx,ty,d2;
  chi2=0.;

  double u0 = TMath::Cos(p[1]),
    v0 = TMath::Sin(p[1]); //phi
  
  for(int i=0; i<pcol; ++i)
    {
      apnt=(TSpacePoint*) PointsColl->at(i);
      r=apnt->GetR();
      Vector2 f = fitObj->EvaluatePlus_(r*r, p[0], u0, v0, p[2]);
      tx = ( apnt->GetX() - f.X ) / apnt->GetErrX(); 
      ty = ( apnt->GetY() - f.Y ) / apnt->GetErrY();
      d2 = tx*tx + ty*ty;
      //      d2 = tx*tx + ty*ty - tx*ty;
      chi2+=d2;
    }
  apnt=0;
  return;
}

static TMinuit* rfitterMinus=0;
void RadFuncMinus(int&, double*, double& chi2, double* p, int)
{
  TFitHelix* fitObj = (TFitHelix*) rfitterMinus->GetObjectFit();
  const std::vector<TSpacePoint*>* PointsColl = fitObj->GetPointsArray();
  int pcol=PointsColl->size();
  if(pcol==0) return;
  
  TSpacePoint* apnt=0;
  double r,tx,ty,d2;
  chi2=0.;
  double u0 = TMath::Cos(p[1]),
    v0 = TMath::Sin(p[1]); //phi
  for(int i=0; i<pcol; ++i)
    {
      apnt=(TSpacePoint*) PointsColl->at(i);
      r=apnt->GetR();
      Vector2 f = fitObj->EvaluateMinus(r*r, p[0], u0, v0, p[2]);
      tx = ( apnt->GetX() - f.X ) / apnt->GetErrX(); 
      ty = ( apnt->GetY() - f.Y ) / apnt->GetErrY();
      d2 = tx*tx + ty*ty;
      //      d2 = tx*tx + ty*ty - tx*ty;
      chi2+=d2;
    }
  apnt=0;
  return;
}

static TMinuit* rfitterMinus_=0;
void RadFuncMinus_(int&, double*, double& chi2, double* p, int)
{
  TFitHelix* fitObj = (TFitHelix*) rfitterMinus_->GetObjectFit();
  const std::vector<TSpacePoint*>* PointsColl = fitObj->GetPointsArray();
  int pcol=PointsColl->size();
  if(pcol==0) return;

  TSpacePoint* apnt=0;
  double r,tx,ty,d2;
  chi2=0.;
  
  
  double u0 = TMath::Cos(p[1]),
    v0 = TMath::Sin(p[1]); //phi
  for(int i=0; i<pcol; ++i)
    {
      apnt=(TSpacePoint*) PointsColl->at(i);
      r=apnt->GetR();
      Vector2 f = fitObj->EvaluateMinus_(r*r, p[0], u0, v0, p[2]);
      tx = ( apnt->GetX() - f.X ) / apnt->GetErrX(); 
      ty = ( apnt->GetY() - f.Y ) / apnt->GetErrY();
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
  const std::vector<TSpacePoint*>* PointsColl = fitObj->GetPointsArray();
  int pcol=PointsColl->size();
  if(pcol==0) return;

  TSpacePoint* apnt=0;
  double r,tz,s;
  chi2=0.;

  for(int i=0; i<pcol; ++i)
    {
      apnt=(TSpacePoint*) PointsColl->at(i);
      r = apnt->GetR();
      s = fitObj->GetArcLength(r*r);
      double f = fitObj->Evaluate(s, p[0], p[1]);
      tz = (apnt->GetZ() - f )/ apnt->GetErrZ();
      chi2+=tz*tz;
    }
  apnt=0;
  return;
}

void ZedFuncB(int&, double*, double& chi2, double* p, int)
{
  TFitHelix* fitObj = (TFitHelix*) zfitter->GetObjectFit();
  const std::vector<TSpacePoint*>* PointsColl = fitObj->GetPointsArray();
  int pcol=PointsColl->size();
  if(pcol==0) return;

  TSpacePoint* apnt=0;
  double r,tz,s;
  chi2=0.;
  for(int i=0; i<pcol; ++i)
    {
      apnt=(TSpacePoint*) PointsColl->at(i);
      r = apnt->GetR();
      s = fitObj->GetArcLengthB(r*r);
      double f = fitObj->Evaluate(s, p[0], p[1]);
      tz = (apnt->GetZ() - f )/ apnt->GetErrZ();
      chi2+=tz*tz;
    }
  apnt=0;
  return;
}

TFitHelix::TFitHelix():TTrack(),
		       fc(kUnknown),fRc(kUnknown),
		       fphi0(kUnknown),fD(kUnknown),
		       flambda(kUnknown),fz0(kUnknown),
		       fa(kUnknown),
		       fx0(kUnknown),fy0(kUnknown),
		       ferr2c(kUnknown),ferr2Rc(kUnknown),
		       ferr2phi0(kUnknown),ferr2D(kUnknown),
		       ferr2lambda(kUnknown),ferr2z0(kUnknown),
		       fBranch(0),fBeta(0.),
		       fchi2R(0.),fStatR(-1),
		       fchi2Z(0.),fStatZ(-1),
		       fChi2RCut(15.),fChi2ZCut(8.),
		       fChi2RMin(1.),fChi2ZMin(0.1),
		       fcCut(16.e-3),fDCut(40.),
		       fpCut(15.)
{  
  fPointsCut = 10;
  fMomentum.SetXYZ(0.0,0.0,0.0);
  fMomentumError.SetXYZ(0.0,0.0,0.0);
}

TFitHelix::TFitHelix(const TTrack& atrack):TTrack(atrack),
					   fc(kUnknown),fRc(kUnknown),
					   fphi0(kUnknown),fD(kUnknown),
					   flambda(kUnknown),fz0(kUnknown),
					   fa(kUnknown),
					   fx0(kUnknown),fy0(kUnknown),
					   ferr2c(kUnknown),ferr2Rc(kUnknown),
					   ferr2phi0(kUnknown),ferr2D(kUnknown),
					   ferr2lambda(kUnknown),ferr2z0(kUnknown),
					   fBranch(0),fBeta(0.),
					   fchi2R(0.),fStatR(-1),
					   fchi2Z(0.),fStatZ(-1),
					   fChi2RCut(15.),fChi2ZCut(8.),
					   fChi2RMin(1.),fChi2ZMin(0.1),
					   fcCut(16.e-3),fDCut(40.),
					   fpCut(15.)
{
  fPointsCut = 10;
  fMomentum.SetXYZ(0.0,0.0,0.0);
  fMomentumError.SetXYZ(0.0,0.0,0.0);
}


TFitHelix::TFitHelix(TObjArray* points):TTrack(points),
					fc(kUnknown),fRc(kUnknown),
					fphi0(kUnknown),fD(kUnknown),
					flambda(kUnknown),fz0(kUnknown),
					fa(kUnknown),
					fx0(kUnknown),fy0(kUnknown),
					ferr2c(kUnknown),ferr2Rc(kUnknown),
					ferr2phi0(kUnknown),ferr2D(kUnknown),
					ferr2lambda(kUnknown),ferr2z0(kUnknown),
					fBranch(0),fBeta(0.),
					fchi2R(0.),fStatR(-1),
					fchi2Z(0.),fStatZ(-1),
					fChi2RCut(7.),fChi2ZCut(4.),
					fChi2RMin(0.1),fChi2ZMin(0.1),
					fcCut(0.001),fDCut(40.)
{ 
  fPointsCut = 10;
  fMomentum.SetXYZ(0.0,0.0,0.0);
  fMomentumError.SetXYZ(0.0,0.0,0.0);
}

TFitHelix::TFitHelix(TStoreHelix* h):TTrack(h->GetSpacePoints()),
				     fc(h->GetC()), fRc(h->GetRc()), 
				     fphi0(h->GetPhi0()), fD(h->GetD()),
				     flambda(h->GetLambda()), fz0(h->GetZ0()),
				     fx0( h->GetX0() ), fy0( h->GetY0() ),
				     ferr2c(h->GetErrC()), ferr2Rc(h->GetErrRc()), 
				     ferr2phi0(h->GetErrPhi0()), ferr2D(h->GetErrD()),
				     ferr2lambda(h->GetErrLambda()), ferr2z0(h->GetErrZ0()),
				     fBranch( h->GetBranch() ), fBeta( h->GetFBeta() ),
				     fMomentum(h->GetMomentumV()), fMomentumError(h->GetMomentumVerror()),
				     fchi2R(h->GetRchi2()), fchi2Z(h->GetZchi2())
{
  SetStatus( h->GetStatus() );
  
  SetResidual( h->GetResidual() );
  std::vector<double> res = h->GetResidualsVector();
  SetResidualsVector( res );
  SetResidualsSquared( h->GetResidualsSquared() );
}

TFitHelix::TFitHelix( const TFitHelix& right ):TTrack(right), 
					       fc(right.fc), fRc(right.fRc),
					       fphi0(right.fphi0), fD(right.fD),
					       flambda(right.flambda),fz0(right.fz0),
					       fa(right.fa),
					       fx0(right.fx0), fy0(right.fy0),
					       ferr2c(right.ferr2c), ferr2Rc(right.ferr2Rc),
					       ferr2phi0(right.ferr2phi0), ferr2D(right.ferr2D),
					       ferr2lambda(right.ferr2lambda), ferr2z0(right.ferr2z0),
					       fBranch(right.fBranch), fBeta(right.fBeta),
					       fMomentum(right.fMomentum), fMomentumError(right.fMomentumError),
					       fchi2R(right.fchi2R), fStatR(right.fStatR),fchi2Z(right.fchi2Z), fStatZ(right.fStatZ)
{ }

TFitHelix& TFitHelix::operator=( const TFitHelix& right )
{
  fPoints     = right.fPoints;
  fNpoints    = right.fNpoints;
  fStatus     = right.fStatus;
  fParticle   = right.fParticle;
  fResiduals2 = right.fResiduals2;
  fResidual   = right.fResidual;
  fResiduals  = right.fResiduals;
  #if USE_MAPS
  fResidualsRadii = right.fResidualsRadii;
  fResidualsXY = right.fResidualsXY;
  #endif
  fPoint      = right.fPoint;
  fc = right.fc; fRc = right.fRc;
  fphi0 = right.fphi0; fD = right.fD;
  flambda = right.flambda;fz0 = right.fz0;
  fa = right.fa;
  fx0 = right.fx0; fy0 = right.fy0;
  ferr2c = right.ferr2c; ferr2Rc = right.ferr2Rc;
  ferr2phi0 = right.ferr2phi0; ferr2D = right.ferr2D;
  ferr2lambda = right.ferr2lambda; ferr2z0 = right.ferr2z0;
  fBranch = right.fBranch; fBeta = right.fBeta;
  fMomentum = right.fMomentum; fMomentumError = right.fMomentumError;
  fchi2R = right.fchi2R; fStatR = right.fStatR;fchi2Z = right.fchi2Z; fStatZ = right.fStatZ;
  return *this;
}

void TFitHelix::Clear(Option_t *)
{
  fPoints.clear();
  if (fPoint) delete fPoint;
  fResiduals.clear();
}

TFitHelix::~TFitHelix()
{
  fPoints.clear();
  if (fPoint) delete fPoint;
  fResiduals.clear();
}

//==============================================================================================
void TFitHelix::RadialFit(double* vstart)
{
  //  std::cout<<"TFitHelix::RadialFit"<<std::endl;
  // Set step sizes for parameters
  //  static double step[fRNpar] = {0.00001 , 0.001 , 0.001};
  static double step[fRNpar] = {0.001 , 0.001 , 0.001};

  // double arglist[10];
  double up = 0.001,// UP = Minuit defines parameter errors as
  //  the change in parameter value required to change the function 
  //  value by UP
    max_calls=500.;// MAX CALLS
  // double iflag = 6.,// if (iflag > 5) Minuit assumes that a new
  // // problem is being redefined, and it forgets the previous best
  // // value of the function, covariance matrix, etc.
  //   tol=0.1;// TOLERANCE: the minimization will stop when the 
  // // estimated vertical distance to the minimum (EDM) is less than 
  // // 0.001*TOLERANCE*UP
  int ierflg = 0;
  int print_level = -1;

  // ================ R FIT 1 +ve ================ 
  //  std::cout<<"R FIT 1 +ve"<<std::endl;
  rfitterPlus = new TMinuit(fRNpar);
  rfitterPlus->SetObjectFit(this);
  rfitterPlus->SetFCN( RadFuncPlus );

  rfitterPlus->SetPrintLevel(print_level);

  rfitterPlus->SetErrorDef(up);
  
  rfitterPlus->mnparm(0, "Rc",   vstart[0], step[0], 0,0,ierflg);
  rfitterPlus->mnparm(1, "phi0", vstart[1], step[1], 0,0,ierflg);
  rfitterPlus->mnparm(2, "D",    vstart[2], step[2], 0,0,ierflg);

  rfitterPlus->SetMaxIterations( max_calls );
  rfitterPlus->Migrad();

 // ================ R FIT 1 -ve ================ 
 // std::cout<<"R FIT 1 -ve"<<std::endl;
  rfitterMinus = new TMinuit(fRNpar);
  rfitterMinus->SetObjectFit(this);
  rfitterMinus->SetFCN( RadFuncMinus );

  rfitterMinus->SetPrintLevel(print_level);

  rfitterMinus->SetErrorDef(up);
  
  rfitterMinus->mnparm(0, "Rc",   vstart[0], step[0], 0,0,ierflg);
  rfitterMinus->mnparm(1, "phi0", vstart[1], step[1], 0,0,ierflg);
  rfitterMinus->mnparm(2, "D",    vstart[2], step[2], 0,0,ierflg);

  rfitterMinus->SetMaxIterations( max_calls );
  rfitterMinus->Migrad();


  // ================ R FIT 2 +ve ================ 
  //  std::cout<<"R FIT 2 +ve"<<std::endl;
  rfitterPlus_ = new TMinuit(fRNpar);
  rfitterPlus_->SetObjectFit(this);
  rfitterPlus_->SetFCN( RadFuncPlus_ );

  rfitterPlus_->SetPrintLevel(print_level);

  rfitterPlus_->SetErrorDef(up);
  
  rfitterPlus_->mnparm(0, "Rc",   vstart[0], step[0], 0,0,ierflg);
  rfitterPlus_->mnparm(1, "phi0", vstart[1], step[1], 0,0,ierflg);
  rfitterPlus_->mnparm(2, "D",    vstart[2], step[2], 0,0,ierflg);

  rfitterPlus_->SetMaxIterations( max_calls );
  rfitterPlus_->Migrad();

  // ================ R FIT 2 -ve ================ 
  //  std::cout<<"R FIT 2 -ve"<<std::endl;
  rfitterMinus_ = new TMinuit(fRNpar);
  rfitterMinus_->SetObjectFit(this);
  rfitterMinus_->SetFCN( RadFuncMinus_ );

  rfitterMinus_->SetPrintLevel(print_level);

  rfitterMinus_->SetErrorDef(up);
  
  rfitterMinus_->mnparm(0, "Rc",   vstart[0], step[0], 0,0,ierflg);
  rfitterMinus_->mnparm(1, "phi0", vstart[1], step[1], 0,0,ierflg);
  rfitterMinus_->mnparm(2, "D",    vstart[2], step[2], 0,0,ierflg);

  rfitterMinus_->SetMaxIterations( max_calls );
  rfitterMinus_->Migrad();
  
  // ======== R FIT ? ========
  //  std::cout<<"FIT selector"<<std::endl;
  TMinuit* best_fit = SelectBestFit();

  if( !best_fit )
    {
      fStatR = -1;
      return;
    }

  //    std::cout<<" Branch : "<<fBranch<<"\t beta/|beta| = "<<fBeta<<std::endl;
  double nused0,nused1;
  int npar;
  best_fit->mnstat(fchi2R,nused0,nused1,npar,npar,fStatR);
  //std::cout<<"Best Fitter status: "<<fStatR<<"\tchi^2 = "<<fchi2R<<std::endl;

  double errR,errphi0,errD;
  best_fit->GetParameter(0,fRc,     errR);
  best_fit->GetParameter(1,fphi0, errphi0);
  best_fit->GetParameter(2,fD,     errD);

  delete rfitterPlus;
  delete rfitterMinus;
  delete rfitterPlus_;
  delete rfitterMinus_;

  fc = 0.5/fRc;
  ferr2Rc = errR*errR;
  ferr2c = 4.*TMath::Power(fc,4.)*ferr2Rc;  
  ferr2phi0 = errphi0*errphi0;
  ferr2D = errD*errD;
  fx0=-fD*TMath::Sin(fphi0);
  fy0=fD*TMath::Cos(fphi0);

  fa=-0.299792458*TMath::Sign(1.,fc)*fB;
}

void TFitHelix::AxialFit(double* vstart)
{
  //  std::cout<<"TFitHelix::AxialFit"<<std::endl;
  // Set step sizes for parameters
  static double step[fZNpar] = {0.001 , 0.001};

  double arglist[10];
  int ierflg = 0;

  // ================ Z FIT  ================ 
  zfitter = new TMinuit(fZNpar);
  zfitter->SetObjectFit(this);
  zfitter->SetFCN( ZedFuncB );
      
  zfitter->SetPrintLevel(-1);

  arglist[0] = 0.001;
  zfitter->mnexcm("SET ERR", arglist , 1, ierflg);
  
  zfitter->mnparm(0, "lambda", vstart[0], step[0], 0,0,ierflg);
  zfitter->mnparm(1, "z0",     vstart[1], step[1], 0,0,ierflg);
  
  zfitter->mnexcm("CALL FCN", arglist, 1, ierflg);
  
  // Now ready for minimization step
  arglist[0] = 500;
  arglist[1] = 0.1;
  zfitter->mnexcm("MIGRAD", arglist, 2, ierflg);

  double nused0,nused1,chi2;
  int npar, stat;
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

TMinuit* TFitHelix::SelectBestFit()
{
  if( rfitterPlus && rfitterPlus_ && rfitterMinus && rfitterMinus_ )
    {
      double chi2[4];
      int stat[4];
      double nused0,nused1;
      int npar;

      rfitterPlus->mnstat(chi2[0],nused0,nused1,npar,npar,stat[0]);

      rfitterMinus->mnstat(chi2[1],nused0,nused1,npar,npar,stat[1]);

      rfitterPlus_->mnstat(chi2[2],nused0,nused1,npar,npar,stat[2]);

      rfitterMinus_->mnstat(chi2[3],nused0,nused1,npar,npar,stat[3]);

      int fail,approx,forced,ok,dunno;
      fail=approx=forced=ok=dunno=0;
      for(int i=0; i<4; ++i)
	{
	  switch(stat[i])
	    {
	    case 0:
	      ++fail;
	      break;
	    case 1:
	      ++approx;
	      break;
	    case 2:
	      ++forced;
	      break;
	    case 3:
	      ++ok;
	      break;
	    default:
	      ++dunno;
	    }
	} 

      if(dunno > 0 )
	{
	  std::cerr<<"ERROR in TFitHelix::SelectBestFit() "<<dunno
		   <<" I don't know what happened"<<std::endl;
	  return 0;
	}

      if( fail == 4 )
	{
	  //	  std::cerr<<"ERROR in TFitHelix::SelectBestFit() "<<fail
	  //		   <<" Massive fitter fails"<<std::endl;
	  return 0;
	}

      if( ok < 4 && ok > 0 )
	{
	  for(int i=0; i<4; ++i)
	    {
	      if( stat[i]<3 ) 
		chi2[i] = 9.e+18;
	    }
	}

      int idx = -1;
      double c2 = 9.e+17;
      for(int i=0; i<4; ++i)
	{
	  if( chi2[i]<c2 ) 
	    {
	      c2  = chi2[i];
	      idx = i;
	    }
	}

      switch(idx)
	{
	case 0:
	  fBranch=1;
	  fBeta=1.;
	  return rfitterPlus;
	case 1:
	  fBranch=1;
	  fBeta=-1.;
	  return rfitterMinus;
	case 2:
	  fBranch=-1;
	  fBeta=1.;
	  return rfitterPlus_;
	case 3:
	  fBranch=-1;
	  fBeta=-1.;
	  return rfitterMinus_;
	case -1:
	  {
	    std::cerr<<"ERROR in TFitHelix::SelectBestFit() ok: "<<ok<<std::endl;
	    for(int i=0; i<4; ++i)
	      {
		std::cerr<<"\t\tfitter: "<<i<<"\tchi^2 = "<<chi2[i]<<"\tstatus: "<<stat[i]<<std::endl;
	      }
	    std::cerr<<"ERROR in TFitHelix::SelectBestFit() failed to find the best"<<std::endl;
	    return 0;
	  }
	}
    }
  else
    {
      std::cerr<<"ERROR in TFitHelix::SelectBestFit() fitter exixtence check failed"<<std::endl;
      return 0;
    }
  return 0;
}

void TFitHelix::Fit()
{
  if(fNpoints<=fNpar) return;
  fStatus=0;

  // Set starting values for parameters
  double* vstart = new double[fNpar];
  Initialization(vstart);

  char parName[fNpar][7] = {"Rc   ","phi0","D   ","lambda","z0  "};
  if( 0 )
    {
      for(int i=0; i<fNpar; ++i)
	printf("-- i%s\t%lf\n",parName[i],vstart[i]);
    }

#if BETA>0 // new functions

  RadialFit(vstart);
  AxialFit(vstart+3);

#else  
  // Set step sizes for parameters
  //  static double step[fNpar] = {0.00001 , 0.001 , 0.001, 0.001 , 0.01};
  static double step[fNpar] = {0.001 , 0.001 , 0.001, 0.001 , 0.01};

  // ================ R FIT 1 ================

  rfitter = new TMinuit(fRNpar);
  rfitter->SetObjectFit(this);
  rfitter->SetFCN( RadFunc );

  double arglist[10];
  int ierflg = 0;

  rfitter->SetPrintLevel(-1);

  arglist[0] = 0.001;
  rfitter->mnexcm("SET ERR", arglist , 1, ierflg);

  rfitter->mnparm(0, "Rc",   vstart[0], step[0], 0,0,ierflg);
  rfitter->mnparm(1, "phi0", vstart[1], step[1], 0,0,ierflg);
  rfitter->mnparm(2, "D",    vstart[2], step[2], 0,0,ierflg);

  rfitter->mnexcm("CALL FCN", arglist, 1, ierflg);

  // Now ready for minimization step
  arglist[0] = 500;
  arglist[1] = 0.1;
  rfitter->mnexcm("MIGRAD", arglist, 2, ierflg);

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

  arglist[0] = 0.001;
  rfitter_->mnexcm("SET ERR", arglist , 1, ierflg);

  rfitter_->mnparm(0, "Rc",   vstart[0], step[0], 0,0,ierflg);
  rfitter_->mnparm(1, "phi0", vstart[1], step[1], 0,0,ierflg);
  rfitter_->mnparm(2, "D",    vstart[2], step[2], 0,0,ierflg);

  rfitter_->mnexcm("CALL FCN", arglist, 1, ierflg);

  // Now ready for minimization step
  arglist[0] = 500;
  arglist[1] = 0.1;
  rfitter_->mnexcm("MIGRAD", arglist, 2, ierflg);

  double chi2_;
  int stat_;
  rfitter_->mnstat(chi2_,nused0,nused1,npar,npar,stat_);

  //  double errc,errphi0,errD;
  double errR,errphi0,errD;

  // ======== R FIT 1 or 2 ? ========
  if(chi2<chi2_)
    {
      rfitter->GetParameter(0,fRc,     errR);
      rfitter->GetParameter(1,fphi0, errphi0);
      rfitter->GetParameter(2,fD,     errD);
      fStatR = stat;
      fchi2R = chi2;
      fBranch = 1;
      //      std::cout<<"RFIT1"<<std::endl;
    }
  else
    {
      rfitter_->GetParameter(0,fRc,     errR);
      rfitter_->GetParameter(1,fphi0, errphi0);
      rfitter_->GetParameter(2,fD,     errD);      
      fStatR = stat_;
      fchi2R = chi2_;
      fBranch = -1;
      //     std::cout<<"RFIT2"<<std::endl;
    }

  delete rfitter;
  delete rfitter_;

  fc = 0.5/fRc;
  ferr2Rc = errR*errR;
  ferr2c = 4.*TMath::Power(fc,4.)*ferr2Rc;
  //  ferr2c = errc*errc;  
  ferr2phi0 = errphi0*errphi0;
  ferr2D = errD*errD;
  //  if(fphi0>=TMath::TwoPi())
  //    fphi0=TMath::TwoPi()-fphi0;
  fx0=-fD*TMath::Sin(fphi0);
  fy0=fD*TMath::Cos(fphi0);

  fa=-0.299792458*TMath::Sign(1.,fc)*fB;

  // ================ Z FIT  ================

  zfitter = new TMinuit(fZNpar);
  zfitter->SetObjectFit(this);
  zfitter->SetFCN( ZedFunc );

  zfitter->SetPrintLevel(-1);

  arglist[0] = 0.001;
  zfitter->mnexcm("SET ERR", arglist , 1, ierflg);

  zfitter->mnparm(0, "lambda", vstart[3], step[3], 0,0,ierflg);
  zfitter->mnparm(1, "z0",     vstart[4], step[4], 0,0,ierflg);

  zfitter->mnexcm("CALL FCN", arglist, 1, ierflg);

  // Now ready for minimization step
  arglist[0] = 500;
  arglist[1] = 0.1;
  zfitter->mnexcm("MIGRAD", arglist, 2, ierflg);

  zfitter->mnstat(chi2,nused0,nused1,npar,npar,stat);
  
  double errlambda,errz0;
  zfitter->GetParameter(0,flambda, errlambda);
  zfitter->GetParameter(1,fz0,     errz0);
  fStatZ = stat;
  fchi2Z = chi2;
  delete zfitter;

  ferr2lambda = errlambda*errlambda;
  ferr2z0 = errz0*errz0;
#endif
  delete[] vstart;
}


//==============================================================================================
// use analytical straight line through first and last spacepoint
// to initialize helix canonical form
void TFitHelix::Initialization(double* Ipar)
{
  TSpacePoint* LastPoint = (TSpacePoint*) GetPointsArray()->back();
  double x1 = LastPoint->GetX(),
    y1 = LastPoint->GetY(),
    z1 = LastPoint->GetZ(),
    phi1 = TMath::ATan2(y1,x1);

  TSpacePoint* FirstPoint = (TSpacePoint*) GetPointsArray()->front();
  double x2 = FirstPoint->GetX(),
    y2 = FirstPoint->GetY(),
    z2 = FirstPoint->GetZ();

  // straight line
  double dx = x2-x1, dy = y2-y1, dz=z2-z1,
    vr2=dx*dx+dy*dy;
  double t = -(x1*dx+y1*dy)/vr2; // intersection with z-axis
  //double t = -(x1*dx+y1*dy+z1*dz)/(vr2+dz*dz); // intersection with origin
  double x0=dx*t+x1, y0=dy*t+y1, z0=dz*t+z1;
  //  std::cout<<"(x0,y0,z0) = ("<<x0<<","<<y0<<","<<z0<<") mm"<<std::endl;

  double r0 = TMath::Sqrt(x0*x0+y0*y0),
    l = dz/TMath::Sqrt(vr2),
    phi0=TMath::ATan2(y0,x0);

  double p0, rc, D, 
    a = phi1 - phi0; // signed difference between angles [-180,180]
                     // https://stackoverflow.com/q/1878907
  a += (a>TMath::Pi()) ? -TMath::TwoPi() : (a<-TMath::Pi()) ? TMath::TwoPi() : 0.;
  
  if( a > 0. ) // negative curvature = positive charge?
    {
      p0 = TMath::ATan2(-x0,y0); // <-- not too shabby * good *
      //p0 = TMath::ATan2(x0,y0);  // <-- not too shabby 2
      rc = -1.e3; // Rc ~ -0.2998 B Q / pT
      D = r0;
    }
  else  // positive curvature = negative charge?
    {
      p0 = TMath::ATan2(x0,y0) + TMath::Pi(); // <-- not too shabby 1 and 2  * good *
      rc = 1.e3; // Rc ~ -0.2998 B Q / pT
      D = -r0;
    }

  //  p0 = phi0>=0.?phi0:phi0+TMath::TwoPi(); // very good 19036

  Ipar[0]=rc;
  Ipar[1]=p0;
  Ipar[2]=D;
  Ipar[3]=l;
  Ipar[4]=z0;
}

//==============================================================================================
// internal helix parameter
inline double TFitHelix::GetBeta(double r2, double Rc, double D)
{
   double num = r2-D*D;
   if (num<0) return 0.;
   double den = 1.+D/Rc,
   arg=num/den;
   return TMath::Sqrt(arg)*0.5/Rc;
}

inline double TFitHelix::GetBetaPlus(double r2, double Rc, double D)
{
  return GetBeta(r2, Rc, D);
}

inline double TFitHelix::GetBetaMinus(double r2, double Rc, double D)
{
   double num = r2-D*D;
   if (num<0) return 0.;
   double den = 1.+D/Rc,
   arg=num/den;
   return -TMath::Sqrt(arg)*0.5/Rc;
}

inline double TFitHelix::GetArcLength(double r2, double Rc, double D)
{
  return TMath::ASin( GetBeta(r2,Rc,D) ) * 2. * Rc;
}

inline double TFitHelix::GetArcLength_(double r2, double Rc, double D)
{
  return ( TMath::Pi() - TMath::ASin( GetBeta(r2,Rc,D) ) ) * 2. * Rc;
}

inline double TFitHelix::GetArcLengthPlus(double r2, double Rc, double D)
{
  return TMath::ASin( GetBetaPlus(r2,Rc,D) ) * 2. * Rc;
}

inline double TFitHelix::GetArcLengthPlus_(double r2, double Rc, double D)
{
  return ( TMath::Pi() - TMath::ASin( GetBetaPlus(r2,Rc,D) ) ) * 2. * Rc;
}

inline double TFitHelix::GetArcLengthMinus(double r2, double Rc, double D)
{
  return TMath::ASin( GetBetaMinus(r2,Rc,D) ) * 2. * Rc;
}

inline double TFitHelix::GetArcLengthMinus_(double r2, double Rc, double D)
{
  return ( TMath::Pi() - TMath::ASin( GetBetaMinus(r2,Rc,D) ) ) * 2. * Rc;
}

// FitHelix Axial and FitVertex::FindSeed and FitVertex::Improve
double TFitHelix::GetArcLength(double r2)
{
  if(fBranch==1)
    return GetArcLength(r2,fRc,fD);
  else if(fBranch==-1)
    return GetArcLength_(r2,fRc,fD);
  else
    return 0;
}

double TFitHelix::GetArcLengthB(double r2)
{
  if(fBranch==1 && fBeta > 0.)
    return GetArcLengthPlus(r2,fRc,fD);
  else if(fBranch==-1 && fBeta > 0.)
    return GetArcLengthPlus_(r2,fRc,fD);
  else if(fBranch==1 && fBeta < 0.)
    return GetArcLengthMinus(r2,fRc,fD);
  else if(fBranch==-1 && fBeta < 0.)
    return GetArcLengthMinus_(r2,fRc,fD);
  else 
    return 0;
}

//==============================================================================================
// FitHelix Radial for +1 Branch
inline Vector2 TFitHelix::Evaluate(double r2, double Rc, double phi, double D)
{
  double u0 = TMath::Cos(phi),
    v0 = TMath::Sin(phi);
  return Evaluate( r2, Rc, u0, v0, D);
}

//==============================================================================================
// FitHelix Radial for +1 Branch
inline Vector2 TFitHelix::Evaluate(double r2, double Rc, double u0, double v0, double D)
{
  double x0 = -D*v0,
    y0 = D*u0,
    beta = GetBeta(r2, Rc, D);
  double beta2 = beta*beta;
  return { x0 + u0 * beta * TMath::Sqrt(1.-beta2) * 2. * Rc - v0 * beta2 * 2. * Rc,
	      y0 + v0 * beta * TMath::Sqrt(1.-beta2) * 2. * Rc + u0 * beta2 * 2. * Rc};
}

// FitHelix Radial for -1 Branch
inline Vector2 TFitHelix::Evaluate_(double r2, double Rc, double phi, double D)
{
  double u0 = TMath::Cos(phi),
    v0 = TMath::Sin(phi);
  return Evaluate_( r2, Rc, u0, v0, D);
}
// FitHelix Radial for -1 Branch
inline Vector2 TFitHelix::Evaluate_(double r2, double Rc, double u0, double v0, double D)
{
  double x0 = -D*v0,
    y0 = D*u0,
    beta = GetBeta(r2, Rc, D);
  double beta2 = beta*beta;
  return { x0 - u0 * beta * TMath::Sqrt(1.-beta2) * 2. * Rc - v0 * beta2 * 2. * Rc,
	      y0 - v0 * beta * TMath::Sqrt(1.-beta2) * 2. * Rc + u0 * beta2 * 2. * Rc};
}

// FitHelix Radial for +1 Branch, beta +ve root
inline Vector2 TFitHelix::EvaluatePlus(double r2, double Rc, double u0, double v0, double D)
{
  return Evaluate(r2, Rc, u0, v0, D);
}
// FitHelix Radial for +1 Branch, beta +ve root
inline Vector2 TFitHelix::EvaluatePlus(double r2, double Rc, double phi, double D)
{
  return Evaluate(r2, Rc, phi, D);
}

// FitHelix Radial for -1 Branch, beta +ve root
inline Vector2 TFitHelix::EvaluatePlus_(double r2, double Rc, double u0, double v0, double D)
{
  return Evaluate_(r2, Rc, u0, v0, D);
}
// FitHelix Radial for -1 Branch, beta +ve root
inline Vector2 TFitHelix::EvaluatePlus_(double r2, double Rc, double phi, double D)
{
  return Evaluate_(r2, Rc, phi, D);
}

// FitHelix Radial for +1 Branch, beta -ve root
inline Vector2 TFitHelix::EvaluateMinus(double r2, double Rc, double u0, double v0, double D)
{
  double x0 = -D*v0,
    y0 = D*u0,
    beta = GetBetaMinus(r2, Rc, D);
  double beta2 = beta*beta;
  return { x0 + u0 * beta * TMath::Sqrt(1.-beta2) * 2. * Rc - v0 * beta2 * 2. * Rc,
                   y0 + v0 * beta * TMath::Sqrt(1.-beta2) * 2. * Rc + u0 * beta2 * 2. * Rc};
}
// FitHelix Radial for +1 Branch, beta -ve root
inline Vector2 TFitHelix::EvaluateMinus(double r2, double Rc, double phi, double D)
{
  double u0 = TMath::Cos(phi),
    v0 = TMath::Sin(phi);
  return EvaluateMinus(r2, Rc, u0, v0, D);
}

// FitHelix Radial for -1 Branch, beta -ve root
inline Vector2 TFitHelix::EvaluateMinus_(double r2, double Rc, double u0, double v0, double D)
{
  double x0 = -D*v0,
    y0 = D*u0,
    beta = GetBetaMinus(r2, Rc, D);
  double beta2 = beta*beta;
/*  double beta_c = beta / c;
  double beta2_c = beta2 / c;
  double beta_ = beta_c * TMath::Sqrt(1.-beta2);
  return TVector2( x0 - u0 * beta_ - v0 * beta2_c,
                   y0 - v0 * beta_ + u0 * beta2_c);*/
  return { x0 - u0 * beta * TMath::Sqrt(1.-beta2) * 2. * Rc - v0 * beta2 * 2. * Rc,
                   y0 - v0 * beta * TMath::Sqrt(1.-beta2) * 2. * Rc + u0 * beta2 * 2. * Rc};
}
// FitHelix Radial for -1 Branch, beta -ve root
inline Vector2 TFitHelix::EvaluateMinus_(double r2, double Rc, double phi, double D)
{
  double u0 = TMath::Cos(phi),
    v0 = TMath::Sin(phi);
 return EvaluateMinus_(r2, Rc, u0, v0, D);
}

// FitHelix Axial
inline double TFitHelix::Evaluate(double s, double l, double z0)
{
  return z0 + l * s;
}


// FitHelix for +1 Branch, beta +ve root
TVector3 TFitHelix::EvaluatePlus(double r2, double Rc, double phi, double D, double l, double z0)
{
  double u0 = TMath::Cos(phi),
    v0 = TMath::Sin(phi);
  double x0 = -D*v0,
    y0 = D*u0,
    beta = GetBetaPlus(r2, Rc, D);
  double beta2 = beta*beta;
  double beta_c = beta * 2. * Rc;
  double beta2_c = beta2 * 2. * Rc;
  double beta_ = beta_c * TMath::Sqrt(1.-beta2);
  return TVector3( x0 + u0 * beta_ - v0 * beta2_c,
                   y0 + v0 * beta_ + u0 * beta2_c,
                   z0 + l * GetArcLengthPlus(r2, Rc, D) );
}

// FitHelix for -1 Branch, beta +ve root
TVector3 TFitHelix::EvaluatePlus_(double r2, double Rc, double phi, double D, double l, double z0)
{
  double u0 = TMath::Cos(phi),
    v0 = TMath::Sin(phi);
  double x0 = -D*v0,
    y0 = D*u0,
    beta = GetBetaPlus(r2, Rc, D);
  double beta2 = beta*beta;
  double beta_c = beta * 2. * Rc;
  double beta2_c = beta2 * 2. * Rc;
  double beta_ = beta_c * TMath::Sqrt(1.-beta2);  
  return TVector3( x0 - u0 * beta_ - v0 * beta2_c,
                   y0 - v0 * beta_ + u0 * beta2_c,
                   z0 + l * GetArcLengthPlus_(r2, Rc, D) );
}

// FitHelix for +1 Branch, beta -ve root
TVector3 TFitHelix::EvaluateMinus(double r2, double Rc, double phi, double D, double l, double z0)
{
  double u0 = TMath::Cos(phi),
    v0 = TMath::Sin(phi);
  double x0 = -D*v0,
    y0 = D*u0,
    beta = GetBetaMinus(r2, Rc, D);
  double beta2 = beta*beta;
  double beta_c = beta * 2. * Rc;
  double beta2_c = beta2 * 2. * Rc;
  double beta_ = beta_c * TMath::Sqrt(1.-beta2);
  return TVector3( x0 + u0 * beta_ - v0 * beta2_c,
                   y0 + v0 * beta_ + u0 * beta2_c,
                   z0 + l * GetArcLengthMinus(r2, Rc, D) );
}

// FitHelix for -1 Branch, beta -ve root
TVector3 TFitHelix::EvaluateMinus_(double r2, double Rc, double phi, double D, double l, double z0)
{
  double u0 = TMath::Cos(phi),
    v0 = TMath::Sin(phi);
  double x0 = -D*v0,
    y0 = D*u0,
    beta = GetBetaMinus(r2, Rc, D);
  double beta2 = beta*beta;
  double beta_c = beta * 2. * Rc;
  double beta2_c = beta2 * 2. * Rc;
  double beta_ = beta_c * TMath::Sqrt(1.-beta2);
  return TVector3( x0 - u0 * beta_ - v0 * beta2_c,
                   y0 - v0 * beta_ + u0 * beta2_c,
                   z0 + l * GetArcLengthMinus_(r2, Rc, D) );
}
//===============================================================================================

// Draw routine
TVector3 TFitHelix::Evaluate(double r2)
{
  double s= GetArcLength(r2);
  Vector2 r;
  if(fBranch==1)
    r=Evaluate(r2, fRc, fphi0, fD);
  else if(fBranch==-1)
    r=Evaluate_(r2, fRc, fphi0, fD);

  return TVector3( r.X, r.Y, Evaluate(s,flambda,fz0) );
}

// FitVertex
TVector3 TFitHelix::EvaluateErrors2(double r2)
{
  double beta = GetBeta(r2,fRc,fD),
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

  return TVector3(dxdc*dxdc*ferr2c + dxdphi*dxdphi*ferr2phi0 + dxdD*dxdD*ferr2D,
		  dydc*dydc*ferr2c + dydphi*dydphi*ferr2phi0 + dydD*dydD*ferr2D,
		  dzdl*dzdl*ferr2lambda + ferr2z0);
}

// Draw routine
TVector3 TFitHelix::EvaluateB(double r2)
{
  if(fBranch==1 && fBeta > 0.)
    return EvaluatePlus(r2,fRc,fphi0,fD,flambda,fz0);
  else if(fBranch==-1 && fBeta > 0.)
    return EvaluatePlus_(r2,fRc,fphi0,fD,flambda,fz0);
  else if(fBranch==1 && fBeta < 0.)
    return EvaluateMinus(r2,fRc,fphi0,fD,flambda,fz0);
  else if(fBranch==-1 && fBeta < 0.)
    return EvaluateMinus_(r2,fRc,fphi0,fD,flambda,fz0);
  else
    {
      return TVector3(-9999999.,-9999999.,-9999999.);
    }
}

// FitVertex
TVector3 TFitHelix::EvaluateErrors2B(double r2)
{
  double beta = -999999.;
  if(fBeta > 0.) 
    beta = GetBetaPlus(r2,fRc,fD);
  else if(fBeta < 0.) 
    beta = GetBetaMinus(r2,fRc,fD);
    
  double beta2 = beta*beta,
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
    dzdl = GetArcLengthB(r2);

  return TVector3(dxdc*dxdc*ferr2c + dxdphi*dxdphi*ferr2phi0 + dxdD*dxdD*ferr2D,
		  dydc*dydc*ferr2c + dydphi*dydphi*ferr2phi0 + dydD*dydD*ferr2D,
		  dzdl*dzdl*ferr2lambda + ferr2z0);
}

// FitVertex
TVector3 TFitHelix::GetPosition(double s)
{
  double rho=2.*fc,
    cp=TMath::Cos(fphi0),sp=TMath::Sin(fphi0),
    crs=TMath::Cos(rho*s),srs=TMath::Sin(rho*s);

  return TVector3(fx0+cp*srs/rho-sp*(1.-crs)/rho,
	     fy0+sp*srs/rho+cp*(1.-crs)/rho,
	     fz0+flambda*s);
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
  return TVector3(dxdc*dxdc*ferr2c + dxdp*dxdp*ferr2phi0 + dxdD*dxdD*ferr2D,
		  dydc*dydc*ferr2c + dydp*dydp*ferr2phi0 + dydD*dydD*ferr2D,
		  s*s*ferr2lambda + ferr2z0);
}

//==============================================================================================
double TFitHelix::Momentum()
{
  if( fc == 0. || fRc == 0. ) 
    {
      std::cerr<<"TFitHelix::Momentum() Error curvature is 0"<<std::endl;
      return -1.;
    }
  double coeff = fa*fRc,
    px=coeff*TMath::Cos(fphi0), // MeV/c
    py=coeff*TMath::Sin(fphi0),
    pz=coeff*flambda;
  //  std::cout<<"TFitHelix::Momentum() coeff (a/2c=a*Rc) is "<<coeff<<std::endl;
  fMomentum.SetXYZ(px,py,pz);
  double pT = fMomentum.Perp();
  double errc = TMath::Sqrt(ferr2c), errphi0 = TMath::Sqrt(ferr2phi0), errlambda = TMath::Sqrt(ferr2lambda);
  fMomentumError.SetXYZ(-px*errc/fc-py*errphi0,-py*errc/fc+px*errphi0,-pz*errc/fc+pT*errlambda);
  return pT;
}

double TFitHelix::GetApproxPathLength()
{
#if BETA>0
  TVector3 r1(EvaluateB(_cathradius*_cathradius));
  TVector3 r2(EvaluateB(_trapradius*_trapradius));
#else
  TVector3 r1(Evaluate(_cathradius*_cathradius));
  TVector3 r2(Evaluate(_trapradius*_trapradius));
#endif
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
     E2 = _ChargedPionMass*_ChargedPionMass+fMomentum.Mag2(),
    beta2 = fMomentum.Mag2()/E2,
    L=GetApproxPathLength();

  // return sigma^2 variance
  return 1.9881e-4*L/p2/beta2/_RadiationLength;
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

  double r=_trapradius,
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
  ferr2Rc     = 4.*TMath::Power(fRc,4.)*ferr2c;
    ferr2phi0  +=Vdiag(1);
    ferr2D     +=Vdiag(2);
    ferr2lambda+=Vdiag(3);
    ferr2z0    +=Vdiag(4);
}

//==============================================================================================
int TFitHelix::TubeIntersection(TVector3& pos1, TVector3& pos2, double radius)
{
  if( TMath::Abs(fD) < radius )
    {
      double beta = GetBeta(radius*radius,fRc,fD),
	s1, s2;
      if( fBranch == 1 )
	{
	  s1 = TMath::ASin(beta)*2.*fRc;
	  s2 = TMath::ASin(-beta)*2.*fRc;
	}
      else if( fBranch == -1 )
	{
	  s1 = (TMath::Pi()-TMath::ASin(beta))*2.*fRc;
	  s2 = (TMath::Pi()-TMath::ASin(-beta))*2.*fRc;
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
  const TVector3* pnt  = fitObj->GetPoint();
  TVector3 h  = fitObj->GetPosition( p[0] );
  //  TVector3 e2 = fitObj->EvaluateErrors2( h.Perp2() );
  TVector3 e2 = fitObj->GetError2(p[0]);
  double tx=pnt->X()-h.X(), ty=pnt->Y()-h.Y(), tz=pnt->Z()-h.Z();
  chi2 = tx*tx/e2.X() + ty*ty/e2.Y() + tz*tz/e2.Z() ;
  return;
}

double TFitHelix::MinDistPoint(TVector3& minpoint)
{
  if(!fPoint)
    {
      std::cerr<<"Call TFitHelix::SetPoint(TVector3* aPoint) first"<<std::endl;
    return -9999999.;
    }

  static double step = 1.e-9;
  hel2pnt = new TMinuit(1);
  hel2pnt->SetObjectFit(this);
  hel2pnt->SetFCN(Hel2PntFunc);

  double arglist[10];
  int ierflg = 0;

  hel2pnt->SetPrintLevel(-1);

  arglist[0] = 1;
  hel2pnt->mnexcm("SET ERR", arglist , 1, ierflg);

#if BETA>0
  double s_start = GetArcLengthB(_trapradius*_trapradius);
#else
  double s_start = GetArcLength(_trapradius*_trapradius);
#endif
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
  minpoint.SetXYZ(mpnt.X(),mpnt.Y(),mpnt.Z());

  //  chi2*=0.5;
  return chi2;
}

//==============================================================================================
bool TFitHelix::IsGood()
{
  // make sure that the fit succeeded
  if( fStatR <= 0 ) fStatus=-2;
  else if( fStatZ <= 0 ) fStatus=-3;
  // do not search for the vertex with bad helices
  else if( (fchi2R/(double) GetRDoF()) <=fChi2RMin)  fStatus=-14;
  else if( (fchi2R/(double) GetRDoF()) > fChi2RCut ) fStatus=-4;
  else if( (fchi2Z/(double) GetZDoF()) <=fChi2ZMin ) fStatus=-15;
  else if( (fchi2Z/(double) GetZDoF()) > fChi2ZCut ) fStatus=-5;
  else if( TMath::Abs(fD)  > fDCut && kDcut )        fStatus=-6;
  else if( TMath::Abs(fc)  > fcCut && kccut )        fStatus=-6;
  else if( fMomentum.Perp() < fpCut && kpcut )       fStatus=-6;
  else if( fNpoints < fPointsCut )                   fStatus=-11;
  else fStatus=1;

  //  std::cout<<"TFitHelix::Status = "<<fStatus<<std::endl;

  return fStatus>0;
}

bool TFitHelix::IsGoodChiSquare()
{
  // make sure that the fit succeeded
  if( fStatR <= 0 ) fStatus=-2;
  else if( fStatZ <= 0 ) fStatus=-3;
  //else if( (fchi2R/(double) GetRDoF()) <=fChi2RMin)  fStatus=-14;
  else if( (fchi2R/(double) GetRDoF()) > fChi2RCut ) fStatus=-4;
  //else if( (fchi2Z/(double) GetZDoF()) <=fChi2ZMin ) fStatus=-15;
  else if( (fchi2Z/(double) GetZDoF()) > fChi2ZCut ) fStatus=-5; 
  else if( fNpoints < fPointsCut )                   fStatus=-11;
  else fStatus=4;

  //  std::cout<<"TFitHelix::Status = "<<fStatus<<std::endl;

  return fStatus>0?true:false;
}
void TFitHelix::Reason()
{
  std::cout<<"  TFitHelix::Reason() Status: "<<GetStatus()<<"\t";
  double chi2R = fchi2R/(double) GetRDoF(), chi2Z = fchi2Z/(double) GetZDoF();
  switch( fStatus )
   {
   case -2:
     std::cout<<"R Fit Cov. Matrix stat: "<<fStatR
	      <<"\tZ Fit Cov. Matrix stat: "<<fStatZ<<std::endl;
     break;
   case -3:
     std::cout<<"R Fit Cov. Matrix stat: "<<fStatR
	      <<"\tZ Fit Cov. Matrix stat: "<<fStatZ<<std::endl;
     break;
   case -14:
     std::cout<<"R Fit chi^2: "<<chi2R
	      <<"\tZ Fit chi^2: "<<chi2Z<<std::endl;
     break;
   case -4:
     std::cout<<"R Fit chi^2: "<<chi2R
	      <<"\tZ Fit chi^2: "<<chi2Z<<std::endl;
     break;
  case -15:
    std::cout<<"R Fit chi^2: "<<chi2R
	      <<"\tZ Fit chi^2: "<<chi2Z<<std::endl;
     break;
  case -5:
     std::cout<<"R Fit chi^2: "<<chi2R
	      <<"\tZ Fit chi^2: "<<chi2Z<<std::endl;
     break;
   case -6:
     if( kDcut )
       std::cout<<"D = "<<fD<<" mm [cut:"<<fDCut<<"]"<<std::endl;
     else if ( kccut )
       std::cout<<"c = "<<fc<<" mm^-1 [cut:"<<fcCut<<"]"<<std::endl;
     else if ( kpcut )
       std::cout<<"pT = "<<fMomentum.Perp()<<" MeV [cut:"<<fpCut<<"]"<<std::endl;
     break;
   case -11:
     std::cout<<"Too few Points..."<<std::endl;
     break;
   default:
     std::cout<<"\n";
   }
}

//==============================================================================================
// bool TFitHelix::IsDuplicated(TFitHelix* right, double cut)
// {
//   //  int cnt=0;
//   int npoints=fPoints.GetEntriesFast();
//   for(int i=0; i<npoints; ++i)
//     {
//       TSpacePoint* pni = (TSpacePoint*) fPoints.At(i);
//       for(int j=0; j<right->fPoints.GetEntriesFast(); ++j)
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

int TFitHelix::Compare(const TObject* aHelix) const
{
  if(TMath::Abs(fc) < TMath::Abs(((TFitHelix*)aHelix)->fc)) return -1;
  else if(TMath::Abs(fc) > TMath::Abs(((TFitHelix*)aHelix)->fc)) return 1;
  else return 0;
}

//==============================================================================================
void TFitHelix::Print(Option_t*) const
{
  std::cout<<" *** TFitHelix ***"<<std::endl;
  std::cout<<"# of points: "<<fNpoints<<std::endl;
  std::cout<<" ("<<std::setw(5)<<std::left<<GetX0()
	   <<", "<<std::setw(5)<<std::left<<GetY0()
	   <<", "<<std::setw(5)<<std::left<<fz0<<")\n"
	   <<" Rc = "<<std::setw(5)<<std::left<<fRc
	   <<" Phi0 = "<<std::setw(5)<<std::left<<fphi0
	   <<"    D = "<<std::setw(5)<<std::left<<fD
	   <<"    L = "<<std::setw(5)<<std::left<<flambda
	   <<std::endl;
  std::cout<<" a = "<<fa<<std::endl;
#if BETA>0
  std::cout<<" Branch : "<<fBranch<<"\t beta/|beta| = "<<fBeta<<std::endl;
#else
  std::cout<<" Branch : "<<fBranch<<std::endl;
#endif
  std::cout<<" Radial Chi2 = "<<fchi2R
	   <<"\t ndf = "<<GetRDoF()
	   <<"\t cov stat = "<<fStatR
	   <<std::endl;
  std::cout<<"  Axial Chi2 = "<<fchi2Z
	   <<"\t ndf = "<<GetZDoF()
	   <<"\t cov stat = "<<fStatZ
	   <<std::endl;
  if(fMomentum.Mag()!=0.0)
    {
      std::cout<<" Momentum = ("
	       <<std::setw(5)<<std::left<<fMomentum.X()<<", "
	       <<std::setw(5)<<std::left<<fMomentum.Y()<<", "
	       <<std::setw(5)<<std::left<<fMomentum.Z()<<") MeV/c"<<std::endl;
      std::cout<<" |p| = "<<fMomentum.Mag()
	       <<" MeV/c\t pT = "<<fMomentum.Perp()<<" MeV/c"<<std::endl;
    }
  if(fResidual.Mag()!=0.0)
    std::cout<<"  Residual = ("
	     <<std::setw(5)<<std::left<<fResidual.X()
	     <<", "<<std::setw(5)<<std::left<<fResidual.Y()
	     <<", "<<std::setw(5)<<std::left<<fResidual.Z()<<") mm"<<std::endl;
  if(fResiduals2!=0.0) 
    std::cout<<"  Residuals Squared = "<<fResiduals2<<" mm^2"<<std::endl;
  if(fParticle!=0)
    std::cout<<"PDG code "<<fParticle<<std::endl;
  std::cout<<" Status: "<<GetStatus()<<std::endl;
  std::cout<<"--------------------------------------------------------------------------"<<std::endl;

}

ClassImp(TFitHelix)
