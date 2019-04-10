// TAlphaEventHelix
#include <TROOT.h>
#include <TInterpreter.h>
#include <TVirtualMC.h>
#include <Riostream.h>
#include <TGeoManager.h>
#include <TVirtualGeoTrack.h>
#include <TMinuit.h>

#include "TAlphaEvent.h"
#include "TAlphaEventCosmicHelix.h"
#include "TAlphaEventHit.h"

ClassImp(TAlphaEventCosmicHelix)

//_____________________________________________________________________
TAlphaEventCosmicHelix::TAlphaEventCosmicHelix()
{
//ctor 
  fHits.SetOwner(kTRUE);
}

//_____________________________________________________________________
TAlphaEventCosmicHelix::TAlphaEventCosmicHelix( TAlphaEventTrack * Track )
{
//ctor

  fa = 0.;
  fb = 0.;
  fR = 0.;
  fth= 0.;
  fphi = 0.;
  flambda = 0.;

  fc = 0.;
  fd0 = 0.;
  fphi0 = 0.;
  fLambda = 0.;
  fz0 = 0.;

  fx0 = 0.;
  fy0 = 0.;
  fz0 = 0.;
  
  fParticle = 8;

  for( Int_t i = 0; i < Track->GetNHits(); i++)
     fHits.AddLast( Track->GetHit( i ) );
  
  if( GetNHits() )
   {
     if( MakeCircle() && MakeDipAngle() )
       {
         fIsGood = kTRUE;
         FindParticle();
         First_to_Canonical();
         
         // sometimes the particle type identification screws up, which
         // can been diagnosed by calculating the residual values between
         // the track and the hits
	 Double_t res = GetResiduals();
	 // printf("res: %lf\n",res);
         if( res > 10 ) 
           {
             // flip the sign
	     if( fParticle == 8 )
	       {
		 fParticle = 9;
	       }
	     else
	       {
		 fParticle = 8;
	       }
             // try again
             First_to_Canonical();             

             // if it is still shitty, exit
	     res = GetResiduals();
	     // printf("res: %lf\n",res);
             if( res > 10 ) 
               {
                 fIsGood = kFALSE;
                 return;
               }
           }

         
         // fit the tracks
         LocalFit();
         
         // // Throw out the ones that really don't fit well
         // if( TMath::Prob(fchi2,1) == 0 )
         //   {
         //     fIsGood = kFALSE;
         //     return;
         //   }
         
         // // catastrophic fit falure ( fchi2 == nan )
         // if( fchi2 != fchi2 )
         //   {
         //     fIsGood = kFALSE;
         //     return;
         //   }
       
       }
     else
       {
         fIsGood = kFALSE;
       }
   }
}

//_____________________________________________________________________
TAlphaEventCosmicHelix::~TAlphaEventCosmicHelix()
{
//dtor
  fHits.Clear();
  fPoints.Delete();
}

//_____________________________________________________________________
void TAlphaEventCosmicHelix::First_to_Canonical( Bool_t Invert )
{
  // Find the reference point on the trajectory
  // as the distance of closest approach to the origin
  DCA(0,0,0,fx0,fy0,fz0);

  if( (fParticle == 8 && !Invert) || (fParticle == 9 && Invert) )
    {
      fphi0 = TMath::ATan2(fy0-fb,fx0-fa) + TMath::Pi();
      fc = -1./(2*fR);
      fLambda = 1./TMath::Tan(TMath::Pi()/2. - flambda);
      fd0 = fR - TMath::Sqrt( fa*fa + fb*fb );
    }
  else
    {
      fphi0 = TMath::ATan2(fy0-fb,fx0-fa);
      fc = 1./(2*fR);
      fLambda = -1./TMath::Tan(TMath::Pi()/2. - flambda);
      fd0 = -1.*fR + TMath::Sqrt( fa*fa + fb*fb );
    }

  Double_t two_pi = 2*TMath::Pi();
  while (fphi0<0) fphi0 += two_pi;
  while (fphi0>= two_pi) fphi0 -= two_pi;

  gEvent->GetVerbose()->Message("TAlphaEventCosmicHelix::First_to_Canonical",
                                "\n----Canonical-----\n");
  gEvent->GetVerbose()->Message("TAlphaEventCosmicHelix::First_to_Canonical",
                                "fParticle: %d\n",
                                fParticle);
  gEvent->GetVerbose()->Message("TAlphaEventCosmicHelix::First_to_Canonical",
                                "c: %lf\n",
                                fc);
  gEvent->GetVerbose()->Message("TAlphaEventCosmicHelix::First_to_Canonical",
                                "phi: %lf\n",
                                fphi0);
  gEvent->GetVerbose()->Message("TAlphaEventCosmicHelix::First_to_Canonical",
                                "D: %lf\n",
                                fd0);
  gEvent->GetVerbose()->Message("TAlphaEventCosmicHelix::First_to_Canonical",
                                "lambda: %lf\n",
                                fLambda);
  gEvent->GetVerbose()->Message("TAlphaEventCosmicHelix::First_to_Canonical",
                                "z0: %lf\n",
                                fz0);
  gEvent->GetVerbose()->Message("TAlphaEventCosmicHelix::First_to_Canonical",
                                "------------------\n");
}

//_____________________________________________________________________
Int_t TAlphaEventCosmicHelix::MakeCircle()
{
  gEvent->GetVerbose()->Message("MakeCircle","Calculating circle parameters.\n");
  // Variable Declaration        

  Int_t n = GetNHits();
  Double_t x[n];
  Double_t y[n];

  // Grab the x,y for the three points
  Double_t xbar = 0;
  Double_t ybar = 0;
  for(Int_t i=0;i<n;i++) 
    {
      x[i] = GetHit(i)->XMRS();
      y[i] = GetHit(i)->YMRS();
      xbar += x[i];
      ybar += y[i];
    }
  
  xbar /= n;
  ybar /= n;

  Double_t xi[n];
  Double_t yi[n];
  Double_t zi[n];
  Double_t Mxx=0;
  Double_t Myy=0;
  Double_t Mxy=0;
  Double_t Mxz=0;
  Double_t Myz=0;
  Double_t Mzz=0;
  for(Int_t i=0;i<n;i++) 
    {
      xi[i] = x[i] - xbar;
      yi[i] = y[i] - ybar;
      zi[i] = xi[i]*xi[i] + yi[i]*yi[i];
      Mxy += xi[i]*yi[i];
      Mxx += xi[i]*xi[i];
      Myy += yi[i]*yi[i];
      Mxz += xi[i]*zi[i];
      Myz += yi[i]*zi[i];
      Mzz += zi[i]*zi[i];
    }
  
  Mxx /= n;
  Myy /= n;
  Mxy /= n;
  Mxz /= n;
  Myz /= n;
  Mzz /= n;

  Double_t Mz = Mxx + Myy;
  Double_t cov_xy = Mxx*Myy - Mxy*Mxy;
  Double_t A3 = 4*Mz;
  Double_t A2 = -3*Mz*Mz - Mzz;
  Double_t A1 = Mzz*Mz + 4*cov_xy*Mz - Mxz*Mxz - Myz*Myz - Mz*Mz*Mz;
  Double_t A0 = Mxz*Mxz*Myy + Myz*Myz*Mxx - Mzz*cov_xy - 2*Mxz*Myz*Mxy + Mz*Mz*cov_xy;
  Double_t A22 = A2 + A2;
  Double_t A33 = A3 + A3 + A3;

  Double_t xnew = 0;
  Double_t ynew = 1e20;
  Double_t eps = 1e-12;
  Double_t IterMax = 20;

  for(Int_t i=0; i<IterMax; i++)
    {
      Double_t yold = ynew;
      ynew = A0 + xnew*(A1 + xnew*(A2 + xnew*A3));
      if( fabs(ynew) > fabs(yold) )
	{
	  printf("wrong direction\n");
	  xnew = 0;
	  continue;
	}
      Double_t Dy = A1 + xnew*(A22 + xnew*A33);
      Double_t xold = xnew;
      xnew = xold - ynew/Dy;
      if( fabs((xnew-xold)/xnew) < eps ) break;
      if( i == IterMax )
	printf("too many iterations\n");
      if( xnew < 0 )
	{
	  printf("negative root\n");
	  xnew = 0;
	}
    }

  Double_t det = xnew*xnew - xnew*Mz + cov_xy;
  Double_t center_x = (Mxz*(Myy-xnew)-Myz*Mxy)/det/2;
  Double_t center_y = (Myz*(Mxx-xnew)-Mxz*Mxy)/det/2;

  fa = center_x + xbar;
  fb = center_y + ybar;
  fR = TMath::Sqrt( center_x*center_x + center_y*center_y + Mz );

  return kTRUE;
}

//_____________________________________________________________________
Int_t TAlphaEventCosmicHelix::MakeDipAngle()
{
  gEvent->GetVerbose()->Message("MakeDipAngle","Calculating dip angle\n");
  if( GetNHits() < 2 )
    {
      gEvent->GetVerbose()->Warning("TAlphaEventCosmicHelix::MakeDipAngle","Not enought hits, aborting\n");
      return kFALSE;
    }

  Double_t x0 = GetHit( 0 )->XMRS();
  Double_t y0 = GetHit( 0 )->YMRS();
  Double_t z0 = GetHit( 0 )->ZMRS();
  Double_t x1 = GetHit( GetNHits()-1 )->XMRS();
  Double_t y1 = GetHit( GetNHits()-1 )->YMRS();
  Double_t z1 = GetHit( GetNHits()-1 )->ZMRS();

  fphi = TMath::ATan2(y0-fb,x0-fa);
          
  Double_t s0= TMath::ATan2(y0-y0+fR*TMath::Sin(fphi),x0-x0+fR*TMath::Cos(fphi));
  Double_t s1= TMath::ATan2(y1-y0+fR*TMath::Sin(fphi),x1-x0+fR*TMath::Cos(fphi));
  if(s0*s1 < 0)
    {
      gEvent->GetVerbose()->Warning("TAlphaEventCosmicHelix::MakeDipAngle","Dip angle error\n");
      return kFALSE;
    }
  
  if((s0-s1)!=0) 
    fth = (z1-z0)/(s0-s1);
  else
    {
      gEvent->GetVerbose()->Warning("TAlphaEventCosmicHelix::MakeDipAngle","Arc length error\n");
      return kFALSE;
    }    

  flambda = TMath::ATan2(fth,fR);

  return kTRUE;
}

//_____________________________________________________________________
TVector3 TAlphaEventCosmicHelix::GetPoint3D( Double_t t )
{ 
  Double_t CosLambda = TMath::Cos(flambda);
  Double_t SinLambda = TMath::Sin(flambda);
  Double_t K1 = 0;
  if(fR!=0) 
    K1 = CosLambda/fR;

  Double_t x = GetHit(0)->XMRS() + fR*(TMath::Cos(K1*t+fphi) - TMath::Cos(fphi));
  Double_t y = GetHit(0)->YMRS() + fR*(TMath::Sin(K1*t+fphi) - TMath::Sin(fphi));
  Double_t z = GetHit(0)->ZMRS() - SinLambda*t;

  TVector3 pnt(x,y,z);

  return pnt;
}

//_____________________________________________________________________
TVector3 TAlphaEventCosmicHelix::GetPoint3D_C( Double_t s )
{ 
  Double_t x0 = -fd0*TMath::Cos(fphi0);
  Double_t y0 = -fd0*TMath::Sin(fphi0);

  Double_t CosPhi = TMath::Cos(fphi0);
  Double_t SinPhi = TMath::Sin(fphi0);

  Double_t x = x0 - 1./(2*fc)*SinPhi*TMath::Sin(2*fc*s) - 
    1./(2*fc)*CosPhi*(1-TMath::Cos(2*fc*s));
  Double_t y = y0 + 1./(2*fc)*CosPhi*TMath::Sin(2*fc*s) - 
    1./(2*fc)*SinPhi*(1-TMath::Cos(2*fc*s));
  Double_t z = fz0 + fLambda*s;

  TVector3 pnt(x,y,z);

  return pnt;
}

//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::GetsFromR( Double_t R )
{
  Double_t s = 0;
  //printf("R: %lf\n",R);
  if(R<fabs(fd0))return 0;
  if(R>(fR+fabs(fd0)))return 0;

  s = 1./fc*TMath::ASin(fc*TMath::Sqrt((R*R-fd0*fd0)/(1.+2.*fd0*fc)));

  return s;
}

//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::GetsFromR_opposite( Double_t R )
{
  Double_t s = 0;
  //printf("R: %lf\n",R);
  if(R<fabs(fd0))return 0;
  if(R>(fR+fabs(fd0)))return 0;

  s = 1./fc*(TMath::Pi() - TMath::ASin(fc*TMath::Sqrt((R*R-fd0*fd0)/(1.+2.*fd0*fc))));

  return s;
}


//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::GetDpDc( TAlphaEventHit * h, Int_t nHit )
{
  Double_t dpdc = 0;

  Double_t p = 0;
  Double_t n = 0;
  Double_t s = 0;
  Double_t x0b = 0;
  Double_t y0b = 0;
  Double_t u0b = 0;
  Double_t v0b = 0;
  Double_t ub = 0;
  Double_t vb = 0;
  GetLocal( h, p, n, s, x0b, y0b, u0b, v0b, ub, vb, nHit);

  Double_t rho = 2*fc;
  Double_t dsdrho = (-1./rho)*(x0b+s*ub)/ub;
  //Double_t dsdrho = 0.5*s*s*v0b/u0b;
  dpdc = 2*(ub-u0b)/(rho*rho) + 2*(s/rho + dsdrho)*vb;
  //dpdc = u0b*s*s + 2*v0b*dsdrho;
  return dpdc;
}

//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::GetDpDphi( TAlphaEventHit * h, Int_t nHit )
{
  Double_t dpdphi = 0;

  Double_t p = 0;
  Double_t n = 0;
  Double_t s = 0;
  Double_t x0b = 0;
  Double_t y0b = 0;
  Double_t u0b = 0;
  Double_t v0b = 0;
  Double_t ub = 0;
  Double_t vb = 0;
  GetLocal( h, p, n, s, x0b, y0b, u0b, v0b, ub, vb, nHit);

  Double_t delta = TMath::Sqrt( h->GetXCenter()*h->GetXCenter() +
				h->GetYCenter()*h->GetYCenter());
  Double_t dsdphi = p/ub;
  dpdphi = delta + vb*dsdphi;

  return dpdphi;
}

//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::GetDpDd0( TAlphaEventHit * h, Int_t nHit )
{
  Double_t dpdd0 = 0;

  Double_t p = 0;
  Double_t n = 0;
  Double_t s = 0;
  Double_t x0b = 0;
  Double_t y0b = 0;
  Double_t u0b = 0;
  Double_t v0b = 0;
  Double_t ub = 0;
  Double_t vb = 0;
  GetLocal( h, p, n, s, x0b, y0b, u0b, v0b, ub, vb, nHit);

  Double_t dsdd0 = v0b/ub;
  dpdd0 = u0b + vb*dsdd0;

  return dpdd0;
}

//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::GetDnDc( TAlphaEventHit * h, Int_t nHit )
{
  Double_t dndc = 0;

  Double_t p = 0;
  Double_t n = 0;
  Double_t s = 0;
  Double_t x0b = 0;
  Double_t y0b = 0;
  Double_t u0b = 0;
  Double_t v0b = 0;
  Double_t ub = 0;
  Double_t vb = 0;
  GetLocal( h, p, n, s, x0b, y0b, u0b, v0b, ub, vb, nHit);

  Double_t rho = 2*fc;
  Double_t dsdrho = (-1./rho)*(x0b+s*ub)/ub;
  //Double_t dsdrho = 0.5*s*s*v0b/u0b;
  dndc = 2.*fLambda*dsdrho;
  
  return dndc;
}

//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::GetDnDlambda( TAlphaEventHit * h, Int_t nHit )
{
  Double_t dndlambda = 0;

  Double_t p = 0;
  Double_t n = 0;
  Double_t s = 0;
  Double_t x0b = 0;
  Double_t y0b = 0;
  Double_t u0b = 0;
  Double_t v0b = 0;
  Double_t ub = 0;
  Double_t vb = 0;
  GetLocal( h, p, n, s, x0b, y0b, u0b, v0b, ub, vb, nHit);

  dndlambda = s;
  
  return dndlambda;
}


//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::GetDnDphi( TAlphaEventHit * h, Int_t nHit )
{
  Double_t dndphi = 0;

  Double_t p = 0;
  Double_t n = 0;
  Double_t s = 0;
  Double_t x0b = 0;
  Double_t y0b = 0;
  Double_t u0b = 0;
  Double_t v0b = 0;
  Double_t ub = 0;
  Double_t vb = 0;
  GetLocal( h, p, n, s, x0b, y0b, u0b, v0b, ub, vb, nHit);

  Double_t dsdphi = p/ub;
  dndphi = fLambda*dsdphi;
  
  return dndphi;
}

//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::GetDnDd0( TAlphaEventHit * h, Int_t nHit )
{
  Double_t dndd0 = 0;

  Double_t p = 0;
  Double_t n = 0;
  Double_t s = 0;
  Double_t x0b = 0;
  Double_t y0b = 0;
  Double_t u0b = 0;
  Double_t v0b = 0;
  Double_t ub = 0;
  Double_t vb = 0;
  GetLocal( h, p, n, s, x0b, y0b, u0b, v0b, ub, vb, nHit);

  Double_t dsdd0 = v0b/ub;
  dndd0 = fLambda*dsdd0;

  return dndd0;
}


//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::DistanceToFurthestHit()
{
  Double_t Max = 0;

  for( Int_t i = 0; i < GetNHits(); i++ )
    {
      TAlphaEventHit * h = GetHit( i );   

      Double_t p = 0;
      Double_t n = 0;
      Double_t s = 0;
      Double_t x0b = 0;
      Double_t y0b = 0;
      Double_t u0b = 0;
      Double_t v0b = 0;
      Double_t ub = 0;
      Double_t vb = 0;
      GetLocal( h, p, n, s, x0b, y0b, u0b, v0b, ub, vb, i);

      // printf("(%d)p: %lf %lf %lf\n",i,h->Y(),p,h->Y() - p);
      // printf("(%d)n: %lf %lf %lf\n",i,h->Z(),n,h->Z() - n);
      Double_t Dist=fabs(h->Y() - p)*fabs(h->Y() - p);
      Dist +=fabs(h->Z() - n)*fabs(h->Z() - n);
      Dist = TMath::Sqrt(Dist);
      if (Dist>Max) Max=Dist; 
      
    }
  return Max;
}

//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::GetResiduals()
{
  Double_t residuals = 0;

  for( Int_t i = 0; i < GetNHits(); i++ )
    {
      TAlphaEventHit * h = GetHit( i );   

      Double_t p = 0;
      Double_t n = 0;
      Double_t s = 0;
      Double_t x0b = 0;
      Double_t y0b = 0;
      Double_t u0b = 0;
      Double_t v0b = 0;
      Double_t ub = 0;
      Double_t vb = 0;
      GetLocal( h, p, n, s, x0b, y0b, u0b, v0b, ub, vb, i);

      // printf("(%d)p: %lf %lf %lf\n",i,h->Y(),p,h->Y() - p);
      // printf("(%d)n: %lf %lf %lf\n",i,h->Z(),n,h->Z() - n);

      residuals += fabs(h->Y() - p);
      residuals += fabs(h->Z() - n);
    }
  
  return residuals;
}

//_____________________________________________________________________
void TAlphaEventCosmicHelix::GetLocal( TAlphaEventHit * h, 
				 Double_t &p, 
				 Double_t &n, 
				 Double_t &s,
				 Double_t &x0b,
				 Double_t &y0b,
				 Double_t &u0b,
				 Double_t &v0b,
				 Double_t &ub,
				 Double_t &vb,
				 Int_t nHit)
{
  if( nHit < 3 )
    {
      First_to_Canonical(kFALSE);
    }
  else
    {
      First_to_Canonical(kTRUE);
    }


  Double_t u0 = -TMath::Sin(fphi0);
  Double_t v0 = TMath::Cos(fphi0);
  Double_t x0 = -fd0*TMath::Cos(fphi0);
  Double_t y0 = -fd0*TMath::Sin(fphi0);
  Double_t x0bp = x0 - h->GetXCenter();
  Double_t y0bp = y0 - h->GetYCenter();

  x0b =  y0bp*h->GetSin() + x0bp*h->GetCos();
  y0b = -x0bp*h->GetSin() + y0bp*h->GetCos();
  u0b = u0*h->GetCos() + v0*h->GetSin();
  v0b = v0*h->GetCos() - u0*h->GetSin();

  Double_t rho = 2*fc;

  Double_t pp = 1-TMath::Power(v0b-rho*x0b,2);
  if( pp < 0 )
    pp = 0.;

  p = y0b + 1./rho*u0b-1./rho*TMath::Sqrt( pp );
  s = -1./(rho)*TMath::ASin(rho*x0b*u0b + rho*(y0b-p)*v0b );

  ub = u0b*TMath::Cos(rho*s) - v0b*TMath::Sin(rho*s);
  vb = v0b*TMath::Cos(rho*s) + u0b*TMath::Sin(rho*s);

  TVector3 pnt = GetPoint3D_C( s );
  n = pnt.Z();
}

//_____________________________________________________________________
void TAlphaEventCosmicHelix::LocalFit()
{
  TMatrixD A(2*GetNHits(),5); A.Zero();
  TMatrixD Vy(2*GetNHits(),2*GetNHits()); Vy.Zero();
  TMatrixD y(2*GetNHits(),1); y.Zero();
  Int_t row = 0;
  Int_t col = 0;
  Int_t xy = 0;

  Double_t ydata[2*GetNHits()];
  for( Int_t i = 0; i < 2*GetNHits(); i++ )
    ydata[i] = 0.;
  for( Int_t i = 0; i < GetNHits(); i++ )
    {
      TAlphaEventHit * h = GetHit( i );   

      Double_t p = 0;
      Double_t n = 0;
      Double_t s = 0;
      Double_t x0b = 0;
      Double_t y0b = 0;
      Double_t u0b = 0;
      Double_t v0b = 0;
      Double_t ub = 0;
      Double_t vb = 0;
      GetLocal( h, p, n, s, x0b, y0b, u0b, v0b, ub, vb, i);

      ydata[2*i+0] = h->Y() - p;
      ydata[2*i+1] = h->Z() - n;
    }
  y.SetMatrixArray(ydata);
  //y.Print();

  Double_t Vydata[2*GetNHits()*2*GetNHits()];
  for( Int_t i = 0; i < 2*GetNHits()*2*GetNHits(); i++ )
    Vydata[i] = 0.;
  for( Int_t i = 0; i < GetNHits(); i++ )
    {
      TAlphaEventHit * h = GetHit( i );   
      for( Int_t j = 0; j < 2; j++ )
	{
	  row = 2*i+j;
	  col = 2*i+j;
	  xy = row*(2*GetNHits()) + col;
	  switch(j)
	    {
	    case 0:
	      Vydata[xy] = h->GetPSigma2();
	      break;
	    case 1:
	      Vydata[xy] = h->GetNSigma2();
	      break;
	    }
	}
    }
  Vy.SetMatrixArray(Vydata);
  Vy.Invert();
  
  Double_t Adata[5*2*GetNHits()];
  for( Int_t i = 0; i < 5*2*GetNHits(); i++ )
    Adata[i] = 0.;
  for( Int_t i = 0; i < GetNHits(); i++ )
    {
      TAlphaEventHit * h = GetHit( i );   

      row = 2*i;

      // dp/dc
      col = 0;
      xy = 5*row + col;
      Adata[xy] = GetDpDc( h, i );
      // dp/dphi
      col = 1;
      xy = 5*row + col;
      Adata[xy] = GetDpDphi( h, i );
      // dp/dd0
      col = 2;
      xy = 5*row + col;
      Adata[xy] = GetDpDd0( h, i );

      row = 2*i + 1;

      // dn/dc
      col = 0;
      xy = 5*row + col;
      Adata[xy] = GetDnDc( h, i );      
      // dn/dphi
      col = 1;
      xy = 5*row + col;
      Adata[xy] = GetDnDphi( h, i ); 
      // dn/dd0
      col = 2;
      xy = 5*row + col;
      Adata[xy] = GetDnDd0( h, i );
      // dn/dlambda
      col = 3;
      xy = 5*row + col;
      Adata[xy] = GetDnDlambda( h, i );
      // dz/dz0
      col = 4;
      xy = 5*row + col;
      Adata[xy] = 1;
    }
  A.SetMatrixArray(Adata);

  TMatrixD m(Vy,TMatrixD::kMult,A);
  TMatrixD Va(A,TMatrixD::kTransposeMult,m);
  Va.Invert();

  // nu = L*R = Va*A^t*Vy*y
  TMatrixD L(Va,TMatrixD::kMultTranspose,A);
  TMatrixD R(Vy,TMatrixD::kMult,y);
  TMatrixD nu(L,TMatrixD::kMult,R);
  //nu.Print();

  Double_t * nuu;
  nuu = nu.GetMatrixArray();
  //printf("%lf %lf %lf %lf %lf\n",nuu[0],nuu[1],nuu[2],nuu[3],nuu[4]);
  fc += nuu[0];
  fphi0 += nuu[1];
  fd0 += nuu[2];
  fLambda += nuu[3];
  fz0 += nuu[4];

  // compute the chi^2
  // chi^2 = r^t*Vy*r, where r = y - A*nu
  TMatrixD l(A,TMatrixD::kMult,nu);
  TMatrixD r(2*GetNHits(),1);
  r = y - l;
  TMatrixD rr(Vy,TMatrixD::kMult,r);
  TMatrixD chi2(r,TMatrixD::kTransposeMult,rr);
  //chi2.Print();

  fchi2 = TMatrixDRow(chi2,0)[0]; // 6 degrees of freedom - 5 parameters
  //printf("chi^2: %lf, prob: %E\n", fchi2, TMath::Prob(fchi2,7));
  
  //Va.Print(); //covariance matrix
  Double_t * cov = Va.GetMatrixArray();
  for( Int_t i = 0; i < 5*5; i++ )
    {
      fcovc[i] = cov[i];
    }
}

//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::DCA( Double_t x, Double_t y, Double_t z )
{
  Double_t DCA = 1000.;

  for( Int_t t = 0; t < 100; t++ )
   {
     TVector3 pnt = GetPoint3D_C( t - 50 );
      Double_t x0 = pnt.X();
     Double_t y0 = pnt.Y();
     Double_t z0 = pnt.Z();

     Double_t dca = TMath::Sqrt( (x-x0)*(x-x0) + (y-y0)*(y-y0) + (z-z0)*(z-z0) );
     if( dca < DCA )
       DCA = dca;
   }

  return DCA;
}

//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::DCA( Double_t x, Double_t y, Double_t ,
				Double_t &xref, Double_t &yref, Double_t &zref )
{
  Double_t DCA = 1000.;

  for( Int_t t = 0; t < 100; t++ )
   {
     TVector3 pnt = GetPoint3D( double(t) - 50. );
     Double_t x0 = pnt.X();
     Double_t y0 = pnt.Y();
     Double_t z0 = pnt.Z();

     Double_t dca = TMath::Sqrt( (x-x0)*(x-x0) + (y-y0)*(y-y0) );
     if( dca < DCA )
       {
	 DCA = dca;
	 xref = x0;
	 yref = y0;
	 zref = z0;
       }
   }

  return DCA;
}


//_____________________________________________________________________
void TAlphaEventCosmicHelix::FindParticle()
{
  // try to determine the direction of curvature by                             
  // interpolating between the inner and outer layer,                           
  // the finding the sign of the residual and the                               
  // interpolated inner point                                                   

  TAlphaEventHit * p0 = GetHit( 0 );
  TAlphaEventHit * p1 = GetHit( 1 );
  TAlphaEventHit * p2 = GetHit( 2 );

  TVector3 r0( p0->XMRS(), p0->YMRS(), p0->ZMRS() );
  TVector3 r1( p1->XMRS(), p1->YMRS(), p1->ZMRS() );
  TVector3 r2( p2->XMRS(), p2->YMRS(), p2->ZMRS() );

  TVector3 n( p1->GetCos(), p1->GetSin(), 0 );

  Double_t d = -n*r1;
  Double_t t = ( -d - n*r0 ) / ( n * ( r2-r0 ) );

  TVector3 p( r0 + t*(r2-r0) );

  // transform into coordinates of the plane                                    
  TVector3 p1loc( r1.X()*p1->GetCos() + r1.Y()*p1->GetSin(),
                  -r1.X()*p1->GetSin()+ r1.Y()*p1->GetCos(),
                  0. );

  TVector3 ploc( p.X()*p1->GetCos() + p.Y()*p1->GetSin(),
                 -p.X()*p1->GetSin() + p.Y()*p1->GetCos(),
                 0.);

  if( (p1loc.Y()-ploc.Y()) < 0. )
    fParticle = 9; // pi-                                                       
  else
    fParticle = 8; // pi+                                                       
}

//_____________________________________________________________________
Double_t TAlphaEventCosmicHelix::DCA_point( Double_t x, Double_t y, Double_t z )
{
  Double_t DCA = 1000.;

  for( Int_t t = 0; t < fPoints.GetEntries(); t++ )
   {
     TVector3 *pnt = (TVector3*) fPoints.At( t );
     Double_t x0 = pnt->X();
     Double_t y0 = pnt->Y();
     Double_t z0 = pnt->Z();

     Double_t dca = TMath::Sqrt( (x-x0)*(x-x0) + (y-y0)*(y-y0) + (z-z0)*(z-z0) );
     if( dca < DCA )
       DCA = dca;
   }

  return DCA;
}

//_____________________________________________________________________
void TAlphaEventCosmicHelix::Print(const Option_t* /* option */) const
{
  printf("\n-------- TAlphaEventCosmicHelix -------\n");
  for( Int_t iHit = 0; iHit < fHits.GetEntries(); iHit++ )
    printf("iHit: %d  %lf %lf %lf\n",iHit,((TAlphaEventHit*)fHits.At( iHit ))->XMRS(),
	((TAlphaEventHit*)fHits.At( iHit ))->YMRS(),((TAlphaEventHit*)fHits.At( iHit ))->ZMRS());
  printf("a: %lf b: %lf R: %lf \nth: %lf phi: %lf lambda: %lf\n",fa,fb,fR,fth,fphi,flambda);
  printf("d0: %lf Lambda: %lf fz0: %lf\n",fd0,fLambda,fz0);
  printf("fParticle: %d\n",fParticle);
  printf("-----------------------------------\n\n");
}
