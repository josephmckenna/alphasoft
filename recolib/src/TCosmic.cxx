// Cosmic Track class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: May 2019

#include "TCosmic.hh"
#include "TPCconstants.hh"
#include "TFitVertex.hh"

#include <TVector3.h>
#include <TMinuit.h>

#include <iostream>
#include <algorithm>

static TMinuit* cfitter=0;
void CosFit(int&, double*, double& chi2, double* p, int)
{
   TFitLine* fitObj = (TFitLine*) cfitter->GetObjectFit();
   const std::vector<TSpacePoint*>* PointsColl = fitObj->GetPointsArray();
   int pcol=PointsColl->size();
   if(pcol==0) return;
  
   TSpacePoint* apnt=0;
   double tx,ty,tz,d2;
   chi2=0.;

   for(int i=0; i<pcol; ++i)
      {
         apnt=(TSpacePoint*) PointsColl->at(i);
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

TCosmic::TCosmic():TFitLine(),fMagneticField(0.),
                   fDCA(agUnknown),fCosAngle(agUnknown),fAngle(agUnknown)
{
   fvstart = new double[9];
   for(int n=0; n<9; ++n) fvstart[n]=agUnknown;
}

TCosmic::TCosmic(TFitHelix* t1,TFitHelix* t2, double b):fMagneticField(b)
{
   fvstart = new double[9];
   AddAllPoints(t1->GetPointsArray(),t2->GetPointsArray());
   CalculateHelDCA(t1,t2); // defined here
}

TCosmic::TCosmic(TFitLine* t1, TFitLine* t2):fMagneticField(0.)
{
   fvstart = new double[9];
   AddAllPoints(t1->GetPointsArray(),t2->GetPointsArray());
   fDCA = t1->Distance(t2);// defined in TFitLine
   fCosAngle = t1->CosAngle(t2);
   fAngle = t1->Angle(t2);
}

TCosmic::TCosmic(TStoreHelix* t1, TStoreHelix* t2, double b):fMagneticField(b)
{
   fvstart = new double[9];
   AddAllPoints( t1->GetSpacePoints(), t2->GetSpacePoints() );
   CalculateHelDCA(t1,t2); // defined here
}

TCosmic::TCosmic(TStoreLine* t1 ,TStoreLine* t2):fMagneticField(0.)
{
   fvstart = new double[9];
   AddAllPoints( t1->GetSpacePoints(), t2->GetSpacePoints() );
   fDCA = LineDistance(t1,t2); // defined here
   fCosAngle = t1->GetDirection()->Dot( *(t2->GetDirection()) );
   fAngle = t1->GetDirection()->Angle( *(t2->GetDirection()) );
}

TCosmic::~TCosmic()
{
   delete[] fvstart;
   fPoints.clear();
   fResiduals.clear();
}

int TCosmic::AddAllPoints(const TObjArray* pcol1, const TObjArray* pcol2)
{
   int np1 = pcol1->GetEntriesFast(),
      np2 = pcol2->GetEntriesFast();
   for(int i=0; i<np1; ++i)
      {
         TSpacePoint* ap = (TSpacePoint*) pcol1->At(i);
         AddPoint( ap );
      }
   for(int i=0; i<np2; ++i)
      {
         TSpacePoint* ap = (TSpacePoint*) pcol2->At(i);
         AddPoint( ap );
      }
   return fNpoints;
}

int TCosmic::AddAllPoints(const std::vector<TSpacePoint*>* pcol1, 
			  const std::vector<TSpacePoint*>* pcol2)
{ 
   for(auto p: *pcol1) AddPoint( p );
   for(auto p: *pcol2) AddPoint( p );
   return fNpoints;
}

void TCosmic::Fit()
{
   if(fNpoints<=fNpar) return;
   fStatus=0;

   // Set step sizes for parameters
   static double step[fNpar*3] = {0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001};

   Initialization();

   cfitter = new TMinuit(fNpar*3);
   cfitter->SetObjectFit(this);
   cfitter->SetFCN( CosFit ); // chi^2-like
 
   double arglist[10];
   int ierflg = 0;

   cfitter->SetPrintLevel(-1);

   arglist[0] = 1;
   cfitter->mnexcm("SET ERR", arglist , 1, ierflg);
  
   cfitter->mnparm(0, "ux", fvstart[0], step[0], 0,0,ierflg);
   cfitter->mnparm(1, "uy", fvstart[1], step[1], 0,0,ierflg);
   cfitter->mnparm(2, "uz", fvstart[2], step[2], 0,0,ierflg); 
   cfitter->mnparm(3, "x0", fvstart[3], step[3], 0,0,ierflg);
   cfitter->mnparm(4, "y0", fvstart[4], step[4], 0,0,ierflg);
   cfitter->mnparm(5, "z0", fvstart[5], step[5], 0,0,ierflg);

   cfitter->mnexcm("CALL FCN", arglist, 1, ierflg);

   // Now ready for minimization step
   arglist[0] = 500;
   arglist[1] = 1.;

   cfitter->mnexcm("MIGRAD", arglist, 2, ierflg);

   double nused0,nused1;
   int npar;
   // status integer indicating how good is the covariance
   //   0= not calculated at all
   //   1= approximation only, not accurate
   //   2= full matrix, but forced positive-definite
   //   3= full accurate covariance matrix
   cfitter->mnstat(fchi2,nused0,nused1,npar,npar,fStat);
  
   double errux,erruy,erruz,errx0,erry0,errz0;
   cfitter->GetParameter(0,fux, errux);
   cfitter->GetParameter(1,fuy, erruy);
   cfitter->GetParameter(2,fuz, erruz);
   cfitter->GetParameter(3,fx0, errx0);
   cfitter->GetParameter(4,fy0, erry0);
   cfitter->GetParameter(5,fz0, errz0);

   delete cfitter;

   double mod = TMath::Sqrt(fux*fux+fuy*fuy+fuz*fuz);
   if( mod == 0.)
      std::cerr<<"TCosmic::Fit() NULL SLOPE: error!"<<std::endl;
   else if( mod == 1. )
      std::cout<<"TCosmic::Fit() UNIT SLOPE: warning!"<<std::endl;
   else
      {
         fux/=mod;
         fuy/=mod;
         fuz/=mod;
      }

   fr0 = sqrt( fx0*fx0 + fy0*fy0 );
   //   std::cout<<"TCosmic::Fit() |v|="<<mod<<" ur="<<fr0<<std::endl;

   ferr2ux = errux*errux;  
   ferr2uy = erruy*erruy;
   ferr2uz = erruz*erruz;  
   ferr2x0 = errx0*errx0;  
   ferr2y0 = erry0*erry0;
   ferr2z0 = errz0*errz0;
}

void TCosmic::Initialization()
{
   double mod,dx,dy,dz,mx,my,mz,x0,y0,z0;
   mx=my=mz=x0=y0=z0=0.;
   int npoints=fPoints.size();
   //std::cout<<"TCosmic::Initialization npoints: "<<npoints<<std::endl;
   for(int i=0;i<npoints-1;i+=2)
      {
         //     std::cout<<"TCosmic::Initialization   "<<i<<std::endl;
         TSpacePoint* PointOne = (TSpacePoint*) fPoints.at(i);
         double x1 = PointOne->GetX(),
            y1 = PointOne->GetY(),
            z1 = PointOne->GetZ();
         x0+=x1; y0+=y1; z0+=z1;
         //      PointOne->Print();

         TSpacePoint* PointTwo = (TSpacePoint*) fPoints.at(i+1);
         double x2 = PointTwo->GetX(),
            y2 = PointTwo->GetY(),
            z2 = PointTwo->GetZ();
         x0+=x2; y0+=y2; z0+=z2;
         //      PointTwo->Print();

         dx = x2-x1; dy = y2-y1; dz=z2-z1;
         //      std::cout<<"TCosmic::Initialization (dx,dy,dz) = ("<<dx<<","<<dy<<","<<dz<<") mm"<<std::endl;
         mod=TMath::Sqrt(dx*dx+dy*dy+dz*dz);
         if( mod == 0. ) continue;
         //      std::cout<<"TCosmic::Initialization mod: "<<mod<<std::endl;
         dx/=mod; dy/=mod; dz/=mod;
         //      std::cout<<"TCosmic::Initialization (dx,dy,dz)/mod = ("<<dx<<","<<dy<<","<<dz<<") mm"<<std::endl;
         if( TMath::IsNaN(dx) || TMath::IsNaN(dy) || TMath::IsNaN(dz) ) continue;
         mx+=dx; my+=dy; mz+=dz;
      } 
   //  std::cout<<"TCosmic::Initialization (mx,my,mz) = ("<<mx<<","<<my<<","<<mz<<") mm"<<std::endl;
   double N = TMath::Floor( double(fPoints.size())*0.5 );
   //  std::cout<<"TCosmic::Initialization N: "<<N<<std::endl;
   mx/=N; my/=N; mz/=N;
   mod=TMath::Sqrt(mx*mx+my*my+mz*mz);
   //  std::cout<<"TCosmic::Initialization mag: "<<mod<<std::endl;
   mx/=mod; my/=mod; mz/=mod;
   //  std::cout<<"TCosmic::Initialization (mx,my,mz)/mag = ("<<mx<<","<<my<<","<<mz<<") mm"<<std::endl;
   fvstart[0]=mx;
   fvstart[1]=my;
   fvstart[2]=mz;

   fvstart[3]=((TSpacePoint*) fPoints.front())->GetX();
   fvstart[4]=((TSpacePoint*) fPoints.front())->GetY();
   fvstart[5]=((TSpacePoint*) fPoints.front())->GetZ();
}

int TCosmic::CalculateHelDCA(TStoreHelix* hi, TStoreHelix* hj)
{
   TFitHelix* hel1 = new TFitHelix(hi);
   hel1->SetMagneticField(fMagneticField);
   TFitHelix* hel2 = new TFitHelix(hj);
   hel2->SetMagneticField(fMagneticField);
   int stat = CalculateHelDCA(hel1,hel2);
   delete hel1;
   delete hel2;
   return stat;
}

int TCosmic::CalculateHelDCA(TFitHelix* hel1, TFitHelix* hel2)
{
   TFitVertex* c = new TFitVertex(-1);
   c->AddHelix( hel1 );
   c->AddHelix( hel2 );
   int stat = c->FindDCA();
   if( stat > 0 )
      {
         //fDCA = c->GetNewChi2(); //mis-name since I'm re-using the vertexing algorithm
         fDCA = c->GetMeanVertex()->Mag();
         TVector3 p1 = hel1->GetMomentumV();
         TVector3 p2 = hel2->GetMomentumV();
         fCosAngle = p1.Unit().Dot(p1.Unit());
         fAngle = p1.Unit().Angle(p1.Unit());
      }
   else
      {
         fDCA = fCosAngle = fAngle = agUnknown;
      }

   delete c;
   return stat;
}

double TCosmic::LineDistance(TStoreLine* l0, TStoreLine* l1)
{
   TVector3 u0 = *(l0->GetDirection());
   TVector3 u1 = *(l1->GetDirection());
   TVector3 p0 = *(l0->GetPoint());
   TVector3 p1 = *(l1->GetPoint());
  
   TVector3 n0 = u0.Cross( u1 ); // normal to lines
   TVector3 c =  p1 - p0;
   if( n0.Mag() == 0. ) return -1.;
  
   TVector3 n1 = n0.Cross( u1 ); // normal to plane formed by n0 and line1
  
   double tau = c.Dot( n1 ) / u0.Dot( n1 ); // intersection between
   TVector3 q0 = tau * u0 + p0;             // plane and line0
  
   double t1 = ( (q0-p0).Cross(n0) ).Dot( u0.Cross(n0) ) / ( u0.Cross(n0) ).Mag2();
   TVector3 q1 = t1 * u0 + p0;
  
   double t2 = ( (q0-p1).Cross(n0) ).Dot( u1.Cross(n0) ) / ( u1.Cross(n0) ).Mag2();
   TVector3 q2 = t2*u1+p1;
  
   TVector3 Q = q2 - q1;
  
   return Q.Mag();
}

ClassImp(TCosmic)

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
