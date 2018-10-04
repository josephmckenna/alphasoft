// Straight Line class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: July 2016

#include "TFitLine.hh"

#include "TSpacePoint.hh"
#include "TMath.h"

#include <iostream>
#include <iomanip>

#include <TMinuit.h>

static TMinuit* lfitter=0;
void FitFunc(int&, double*, double& chi2, double* p, int)
{
  TFitLine* fitObj = (TFitLine*) lfitter->GetObjectFit();
  const TObjArray* PointsColl = fitObj->GetPointsArray();
  if(PointsColl->GetEntriesFast()==0) return;
  
  TSpacePoint* apnt=0;
  double tx,ty,tz,d2;
  chi2=0.;
  for(int i=0; i<PointsColl->GetEntriesFast(); ++i)
    {
      apnt=(TSpacePoint*) PointsColl->At(i);
      double r2 = apnt->GetR() * apnt->GetR();
      TVector3 f = fitObj->Evaluate( r2, p[0], p[1], p[2], p[3], p[4], p[5]  );
      tx = ( apnt->GetX() - f.X() ) / apnt->GetErrX(); 
      ty = ( apnt->GetY() - f.Y() ) / apnt->GetErrY();
      tz = ( apnt->GetZ() - f.Z() ) / apnt->GetErrZ();
      d2 = tx*tx + ty*ty + tz*tz;
      chi2+=d2;
    }
  apnt=0;
  return;
}

void PointDistFunc(int&, double*, double& d2, double* p, int)
{
  TFitLine* fitObj = (TFitLine*) lfitter->GetObjectFit();
  const TObjArray* PointsColl = fitObj->GetPointsArray();
  if(PointsColl->GetEntriesFast()==0) return;
  
  TSpacePoint* apnt=0;
  d2=0.;
  for(int i=0; i<PointsColl->GetEntriesFast(); ++i)
    {
      apnt=(TSpacePoint*) PointsColl->At(i);
      double hit[]={apnt->GetX(),apnt->GetY(),apnt->GetZ()};
      d2+=fitObj->PointDistance2(p,hit);
    }
  apnt=0;
  return;
}

TFitLine::TFitLine():TTrack(),
		     fchi2(0.),fStat(-1),
		     fChi2Min(4.e-2),fChi2Cut(40.)
{ 
  fPointsCut = 10;
}

TFitLine::TFitLine(TObjArray* points):TTrack(points),
				      fchi2(0.),fStat(-1),
				      fChi2Min(4.e-2),fChi2Cut(40.)
{ 
  fPointsCut = 10;
}

TFitLine::TFitLine(const TTrack& atrack):TTrack(atrack),
					 fchi2(0.),fStat(-1),
					 fChi2Min(4.e-2),fChi2Cut(40.)
{ 
  fPointsCut = 10;
}

TFitLine::~TFitLine()
{
  fPoints.Clear();
}

TVector3 TFitLine::GetU() const
{
  TVector3 u = TVector3(fux,fuy,fuz);
  return u;
  //Leaky if not carefully deleted:
  //double* u = new double[3];
  //u[0]=fux; u[1]=fuy; u[2]=fuz;
  //return u;
}

TVector3 TFitLine::Get0() const
{
  TVector3 u = TVector3(fx0,fy0,fz0);
  return u;
  //Leaky if not carefully deleted:
  //double* u = new double[3];
  //u[0]=fx0; u[1]=fy0; u[2]=fz0;
  //return u;
}

void TFitLine::Fit()
{
  if(fNpoints<=fNpar) return;
  fStatus=0;

  // Set starting values for parameters
  static double vstart[fNpar*3];
  Initialization(vstart);

  // Set step sizes for parameters
  static double step[fNpar*3] = {0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001};

  lfitter = new TMinuit(fNpar*3);
  lfitter->SetObjectFit(this);
  lfitter->SetFCN( FitFunc ); // chi^2-like
  //  lfitter->SetFCN( PointDistFunc ); // distance^2

  double arglist[10];
  int ierflg = 0;

  lfitter->SetPrintLevel(-1);

  arglist[0] = 1;
  lfitter->mnexcm("SET ERR", arglist , 1, ierflg);
  
  lfitter->mnparm(0, "ux", vstart[0], step[0], 0,0,ierflg);
  lfitter->mnparm(1, "uy", vstart[1], step[1], 0,0,ierflg);
  lfitter->mnparm(2, "uz", vstart[2], step[2], 0,0,ierflg); 
  lfitter->mnparm(3, "x0", vstart[3], step[3], 0,0,ierflg);
  lfitter->mnparm(4, "y0", vstart[4], step[4], 0,0,ierflg);
  lfitter->mnparm(5, "z0", vstart[5], step[5], 0,0,ierflg);

  lfitter->mnexcm("CALL FCN", arglist, 1, ierflg);

  // Now ready for minimization step
  arglist[0] = 500;
  arglist[1] = 1.;

  lfitter->mnexcm("MIGRAD", arglist, 2, ierflg);

  double nused0,nused1;
  int npar;
  // status integer indicating how good is the covariance
  //   0= not calculated at all
  //   1= approximation only, not accurate
  //   2= full matrix, but forced positive-definite
  //   3= full accurate covariance matrix
  lfitter->mnstat(fchi2,nused0,nused1,npar,npar,fStat);
  
  double errux,erruy,erruz,errx0,erry0,errz0;
  lfitter->GetParameter(0,fux, errux);
  lfitter->GetParameter(1,fuy, erruy);
  lfitter->GetParameter(2,fuz, erruz);
  lfitter->GetParameter(3,fx0, errx0);
  lfitter->GetParameter(4,fy0, erry0);
  lfitter->GetParameter(5,fz0, errz0);

  double mod = TMath::Sqrt(fux*fux+fuy*fuy+fuz*fuz);
  if( mod == 0.)
    std::cerr<<"TFitLine::Fit() NULL SLOPE: error!"<<std::endl;
  else
    {
      fux/=mod;
      fuy/=mod;
      fuz/=mod;
    }

  ferr2ux = errux*errux;  
  ferr2uy = erruy*erruy;
  ferr2uz = erruz*erruz;  
  ferr2x0 = errx0*errx0;  
  ferr2y0 = erry0*erry0;
  ferr2z0 = errz0*errz0;

}


TVector3 TFitLine::GetPosition(double t, 
			       double ux, double uy, double uz, 
			       double x0, double y0, double z0)
{
  TVector3 pos(ux*t+x0,
	       uy*t+y0,
	       uz*t+z0);
  return pos;
}

TVector3 TFitLine::GetPosition(double t)
{
  return GetPosition(t, fux, fuy, fuz, fx0, fy0, fz0);
}

double TFitLine::GetParameter( double r2,
			       double ux, double uy, double uz, 
			       double x0, double y0, double z0)
{
  double a = ux*ux+uy*uy, 
    beta = ux*x0+uy*y0,
    c = x0*x0+y0*y0-r2;
  if( a==0. ) return -9999999.;
  double delta = beta*beta-a*c;
  if(delta<0.) 
    return -9999999.;
  else if(delta==0.)
    return -beta/a;

  double t1 = (-beta-TMath::Sqrt(delta))/a,
    t2 = (-beta+TMath::Sqrt(delta))/a;

  TVector3 p1 = GetPosition(t1,ux,uy,uz,x0,y0,z0);
  //  std::cout<<p1.X()<<"\t"<<p1.Y()<<"\t"<<p1.Z()<<std::endl;
  TVector3 p2 = GetPosition(t2,ux,uy,uz,x0,y0,z0);
  //  std::cout<<p2.X()<<"\t"<<p2.Y()<<"\t"<<p2.Z()<<std::endl;

  TSpacePoint* LastPoint = (TSpacePoint*) fPoints.Last();
  TVector3 point(LastPoint->GetX(),
		 LastPoint->GetY(),
		 LastPoint->GetZ());
  if( (p1-point).Mag() < (p2-point).Mag())
    {  
      //      std::cout<<"t1: "<<t1<<" @\t"<<p1.X()<<"\t"<<p1.Y()<<"\t"<<p1.Z()<<std::endl;
      return t1;
    }
  else
    {
      //      std::cout<<"t2: "<<t2<<" @\t"<<p2.X()<<"\t"<<p2.Y()<<"\t"<<p2.Z()<<std::endl;
      return t2;
    }
}

double TFitLine::GetParameter( double r2 )
{
  return GetParameter( r2, fux, fuy, fuz, fx0, fy0, fz0);
}

TVector3 TFitLine::Evaluate(double r2, 
			    double ux, double uy, double uz, 
			    double x0, double y0, double z0)
{
  double a = ux*ux+uy*uy, 
    beta = ux*x0+uy*y0,
    c = x0*x0+y0*y0-r2;
  double delta = beta*beta-a*c;
  if(delta<0.) 
    return TVector3(-9999999.,-9999999.,-9999999.);
  else if(delta==0.)
    return GetPosition(-beta/a,ux,uy,uz,x0,y0,z0);
    
  //  std::cout<<"discriminator: "<<delta<<std::endl;
  double t1 = (-beta-TMath::Sqrt(delta))/a,
    t2 = (-beta+TMath::Sqrt(delta))/a;

  TVector3 p1 = GetPosition(t1,ux,uy,uz,x0,y0,z0);
  //  std::cout<<p1.X()<<"\t"<<p1.Y()<<"\t"<<p1.Z()<<std::endl;
  TVector3 p2 = GetPosition(t2,ux,uy,uz,x0,y0,z0);
  //  std::cout<<p2.X()<<"\t"<<p2.Y()<<"\t"<<p2.Z()<<std::endl;

  TSpacePoint* LastPoint = (TSpacePoint*) fPoints.Last();
  TVector3 point(LastPoint->GetX(),
		 LastPoint->GetY(),
		 LastPoint->GetZ());
  if( (p1-point).Mag() < (p2-point).Mag())
    {  
      //      std::cout<<"p1\t"<<p1.X()<<"\t"<<p1.Y()<<"\t"<<p1.Z()<<std::endl;
      return p1;
    }
  else
    {
      //      std::cout<<"p2\t"<<p2.X()<<"\t"<<p2.Y()<<"\t"<<p2.Z()<<std::endl;
      return p2;
    }
}

TVector3 TFitLine::Evaluate(double r2)
{
  return Evaluate(r2, fux, fuy, fuz, fx0, fy0, fz0);
}


// use analytical line through first and last point
// to initialize line
// void TFitLine::Initialization(double* Ipar)
// {
//   //  TSpacePoint* LastPoint = (TSpacePoint*) fPoints.Last();
//   TSpacePoint* LastPoint = (TSpacePoint*) fPoints.At(3);
//   double x1 = LastPoint->GetX(),
//     y1 = LastPoint->GetY(),
//     z1 = LastPoint->GetZ();
//   std::cout<<"(x1,y1,z1) = ("<<x1<<","<<y1<<","<<z1<<") mm"<<std::endl;

//   TSpacePoint* FirstPoint = (TSpacePoint*) fPoints.First();
//   double x2 = FirstPoint->GetX(),
//     y2 = FirstPoint->GetY(),
//     z2 = FirstPoint->GetZ();
//   std::cout<<"(x2,y2,z2) = ("<<x2<<","<<y2<<","<<z2<<") mm"<<std::endl;

//   double dx = x2-x1, dy = y2-y1, dz=z2-z1;
//   double mod=TMath::Sqrt(dx*dx+dy*dy+dz*dz);
//   dx/=mod; dy/=mod; dz/=mod;
//   std::cout<<"(dx,dy,dz) = ("<<dx<<","<<dy<<","<<dz<<") mm"<<std::endl;
//   //  double x0=dx*t+x1, y0=dy*t+y1, z0=dz*t+z1;
//   //  std::cout<<"(x0,y0,z0) = ("<<x0<<","<<y0<<","<<z0<<") mm"<<std::endl;
  

//   Ipar[0]=dx;
//   Ipar[1]=dy;
//   Ipar[2]=dz;
//   Ipar[3]=x2;
//   Ipar[4]=y2;
//   Ipar[5]=z2;
// }

void TFitLine::Initialization(double* Ipar)
{
  double mod,dx,dy,dz,mx,my,mz,x0,y0,z0;
  mx=my=mz=x0=y0=z0=0.;
  for(int i=0;i<fPoints.GetEntriesFast()-1;i+=2)
    {
      TSpacePoint* PointOne = (TSpacePoint*) fPoints.At(i);
      double x1 = PointOne->GetX(),
	y1 = PointOne->GetY(),
	z1 = PointOne->GetZ();
      x0+=x1; y0+=y1; z0+=z1;

      TSpacePoint* PointTwo = (TSpacePoint*) fPoints.At(i+1);
      double x2 = PointTwo->GetX(),
	y2 = PointTwo->GetY(),
	z2 = PointTwo->GetZ();
      x0+=x2; y0+=y2; z0+=z2;

      dx = x2-x1; dy = y2-y1; dz=z2-z1;
      mod=TMath::Sqrt(dx*dx+dy*dy+dz*dz);
      dx/=mod; dy/=mod; dz/=mod;
      //      std::cout<<"(dx,dy,dz) = ("<<dx<<","<<dy<<","<<dz<<") mm"<<std::endl;
      mx+=dx; my+=dy; mz+=dz;
    } 

  double N = TMath::Floor( double(fPoints.GetEntriesFast())*0.5 );
  mx/=N; my/=N; mz/=N;
  mod=TMath::Sqrt(mx*mx+my*my+mz*mz);
  mx/=mod; my/=mod; mz/=mod;
  Ipar[0]=mx;
  Ipar[1]=my;
  Ipar[2]=mz;

  // Ipar[3]=x0*0.5/N;
  // Ipar[4]=y0*0.5/N;
  // Ipar[5]=z0*0.5/N;
  Ipar[3]=((TSpacePoint*) fPoints.First())->GetX();
  Ipar[4]=((TSpacePoint*) fPoints.First())->GetY();
  Ipar[5]=((TSpacePoint*) fPoints.First())->GetZ();

  // std::cout<<"Tentative slope: ";
  // for(int p=0;p<3;++p)
  //   std::cout<<Ipar[p]<<" ";
  // std::cout<<"\nA point: ";
  // for(int p=3;p<6;++p)
  //   std::cout<<Ipar[p]<<" ";
  // std::cout<<"\n"<<std::endl;
}

double TFitLine::MinDistPoint(TVector3& minpoint)
{
  TVector3 p0;
  if(!fPoint)
    {
      std::cerr<<"Call TFitLine::SetPoint(TVector3* aPoint) first"<<std::endl;
      return -9999999.;
    }
  else
    p0.SetXYZ(fPoint->X(),fPoint->Y(),fPoint->Z());

  TVector3 u(fux, fuy, fuz);
  TVector3 v0(fx0,fy0,fz0);
  //  TVector3 v0(GetPosition(0.0));
  
  //  double t = (p1-*point).Dot(p2-p1)/(p2-p1).Mag2();
  double t = (v0-p0).Dot(u)/u.Mag2();
  TVector3 res = GetPosition(TMath::Abs(t));
  minpoint.SetXYZ(res.X(),res.Y(),res.Z());
  if(0)
    std::cout<<"TFitLine::MinDistPoint  between point = ("<<p0.X()<<", "<<p0.Y()<<", "<<p0.Z()<<")   @ t = "<<t<<"   ("<<res.X()<<", "<<res.Y()<<", "<<res.Z()<<")"<<std::endl;

  //  double d = ((p2-p1).Cross(p1-*point)).Mag()/(p2-p1).Mag();
  double d = (u.Cross(v0-p0)).Mag()/u.Mag();
  return d;
}

double TFitLine::PointDistance2(double* par, double* point)
{
  TVector3 u(par[0],par[1],par[2]);
  if(u.Mag2()==0.) return -9999999.;
  TVector3 p(par[3],par[4],par[5]);
  TVector3 h(point[0],point[1],point[2]);
  return (u.Cross(p-h)).Mag2()/u.Mag2();
}

double TFitLine::MinRad()
{
  double D = 0., 
    den = TMath::Sqrt( fux*fux + fuy*fuy );
  if( fux != -fuy && den != 0. )
    {
      D = TMath::Abs( fx0*fuy - fx0*fux ) / den ;
    }
  return D;
}

double TFitLine::MinRad2()
{
  double a = fux*fx0+fuy*fy0, 
    b = fux*fux + fuy*fuy,
    c = fx0*fx0+fy0*fy0;

  if( b <= 0. ) return -1.;
 
  return a*a/b+c;
}

bool TFitLine::IsGood()
{
  if( fStat <= 0 )                                fStatus=-2;
  else if( (fchi2/(double) GetDoF()) <=fChi2Min ) fStatus=-14;
  else if( (fchi2/(double) GetDoF()) > fChi2Cut ) fStatus=-4;
  else if( fNpoints < fPointsCut )                fStatus=-11;
  else                                            fStatus=1;

  if(fStatus>0)
    return true;
  else
    return false;
}

void TFitLine::Reason()
{
  std::cout<<"  TFitLine::Reason() Status: "<<GetStatus()<<"\t";
  double chi2 = fchi2/(double) GetDoF();
  switch( fStatus )
   {
   case -2:
     std::cout<<" Fit Cov. Matrix stat: "<<fStat<<std::endl;
     break;
   case -14:
     std::cout<<" Fit chi^2: "<<chi2<<std::endl;
     break;
   case -4:
     std::cout<<" Fit chi^2: "<<chi2<<std::endl;
     break;
   case -11:
     std::cout<<"Too few Points..."<<std::endl;
     break;
   default:
     std::cout<<"\n";
   }
}

double TFitLine::Angle( TFitLine* line )
{
  TVector3 u0(fux, fuy, fuz);
  TVector3 u1(line->GetUx(),line->GetUy(),line->GetUz());
  return u0.Angle(u1);
}

double TFitLine::CosAngle( TFitLine* line )
{
  TVector3 u0(fux, fuy, fuz);
  TVector3 u1(line->GetUx(),line->GetUy(),line->GetUz());
  return u0.Dot(u1);
}

TVector3 TFitLine::Sagitta(TFitLine* line)
{
  TVector3 u0(fux, fuy, fuz);
  TVector3 u1(line->fux, line->fuy, line->fuz);
  TVector3 p0(fx0, fy0, fz0);
  TVector3 p1(line->fx0, line->fy0, line->fz0);

  TVector3 n0 = u0.Cross( u1 ); // normal to lines
  TVector3 c =  p1 - p0;
  if( n0.Mag() == 0. ) return TVector3(-99999,-99999,-99999);
  
  TVector3 n1 = n0.Cross( u1 ); // normal to plane formed by n0 and line1

  double tau = c.Dot( n1 ) / u0.Dot( n1 ); // intersection between
  TVector3 q0 = tau * u0 + p0;             // plane and line0

  double t1 = ( (q0-p0).Cross(n0) ).Dot( u0.Cross(n0) ) / ( u0.Cross(n0) ).Mag2();
  TVector3 q1 = t1 * u0 + p0;

  double t2 = ( (q0-p1).Cross(n0) ).Dot( u1.Cross(n0) ) / ( u1.Cross(n0) ).Mag2();
  TVector3 q2 = t2*u1+p1;

  TVector3 Q = q2 - q1;
  
  //  cout<<dist<<"\t"<<Q.Mag()<<endl;

  return Q;
}

double TFitLine::Distance(TFitLine* line)
{
  return Sagitta(line).Mag();
}

void TFitLine::Print(Option_t*) const
{
  std::cout<<" *** TFitLine ***"<<std::endl;
  std::cout<<"# of points: "<<fNpoints<<std::endl;
  std::cout<<" q = ("<<std::setw(5)<<std::left<<fx0
	   <<", "<<std::setw(5)<<std::left<<fy0
	   <<", "<<std::setw(5)<<std::left<<fz0<<")\n"
	   <<" u = ("<<std::setw(5)<<std::left<<fux
	   <<", "<<std::setw(5)<<std::left<<fuy
	   <<", "<<std::setw(5)<<std::left<<fuz<<")\n"
	   <<"phi = "<<TMath::ATan2(fuy,fux)<<" rad\t"
    //<<"theta = "<<TMath::ACos(fuz)<<" rad\n";
	   <<"theta = "<<TMath::ATan2(fuz,TMath::Sqrt(fux*fux+fuy*fuy))<<" rad\n";
  std::cout<<"chi^2 = "<<fchi2<<"\t ndf = "<<GetDoF()<<"\t cov stat = "<<fStat<<std::endl;
  if(fResidual.Mag()!=0.0)
    std::cout<<"  Residual = ("
	     <<std::setw(5)<<std::left<<fResidual.X()
	     <<", "<<std::setw(5)<<std::left<<fResidual.Y()
	     <<", "<<std::setw(5)<<std::left<<fResidual.Z()<<") mm"<<std::endl;
  if(fResiduals2!=0.0) 
    std::cout<<"  Residuals Squared = "<<fResiduals2<<" mm^2"<<std::endl;
  if(fParticle!=0)
    std::cout<<"PDG code "<<fParticle<<std::endl;
  std::cout<<"Status: "<<fStatus<<std::endl;
  std::cout<<"--------------------------------------------------------------------------"<<std::endl;

}

// void TFitLine::Draw(Option_t*)
// {
//   fGraph = new TPolyLine3D();
  
//   // TVector3 u(GetU());
//   // TVector3 p(Get0());

//   // double t0 = -p.Dot(u)/u.Mag2(),
//   //   Npoints=100.;
//   // double t1=t0,t2=t0;

//   // std::cout<<"t0: "<<t0<<std::endl;

//   // TVector3 pos;
//   // do
//   //   {
//   //     pos=GetPosition(t1);
//   //     ++t1;
//   //   }
//   // while(pos.Perp()<TPCBase::TPCBaseInstance()->GetROradius(true));
//   // std::cout<<"t1: "<<t1<<std::endl;
//   // pos.Print();
//   // do
//   //   {
//   //     pos=GetPosition(t2);
//   //     --t2;
//   //   }
//   // while(pos.Perp()<TPCBase::TPCBaseInstance()->GetROradius(true));
//   // //  while(pos.Perp()>0.);
//   // std::cout<<"t2: "<<t2<<std::endl;
//   // pos.Print();

//   // double tmin=t1<t2?t1:t2;
//   // double tmax=t1>t2?t1:t2;
    
//   // double dt=TMath::Abs(tmax-tmin)/Npoints;
//   // int ip=0;
//   // for(double t = tmin; t <= tmax; t += dt)
//   //   {
//   //     TVector3 p = GetPosition(t);
//   //     if(p==TVector3(-9999999.,-9999999.,-9999999.))
//   // 	continue;
//   //     std::cout<<"t: "<<t<<std::endl;
//   //     p.Print();
//   //     fGraph->SetPoint(ip,p.X(),p.Y(),p.Z());
//   //     ++ip;
//   //   }

//   int ip=0;
//   for( double r=0.; r<220.; r+=0.2)
//     {
//       TVector3 p = Evaluate( r*r );
//       if( p == TVector3(-9999999.,-9999999.,-9999999.) )
//    	continue;
//       //      p.Print();
//       fGraph->SetPoint(ip,p.X(),p.Y(),p.Z());
//       ++ip;
//     }

//   if(fStatus==1) // good Line
//     {
//       fGraph->SetLineColor(kGreen);
//       fGraph->SetLineWidth(2);
//     }
//   else // not good
//     {
//       fGraph->SetLineColor(kGray);
//       fGraph->SetLineWidth(1);
//     }
// }

ClassImp(TFitLine)
