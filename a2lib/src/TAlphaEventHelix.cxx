// TAlphaEventHelix
#include <TROOT.h>
#include <TInterpreter.h>
//#include <TVirtualMC.h>
#include <Riostream.h>
#include <TGeoManager.h>
#include <TVirtualGeoTrack.h>
#include <TMinuit.h>

#include "TAlphaEvent.h"
#include "TAlphaEventHelix.h"
#include "TAlphaEventHit.h"


ClassImp(TAlphaEventHelix)

void fcnHelix(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag);

//_____________________________________________________________________
TAlphaEventHelix::TAlphaEventHelix()
{
//ctor 
}

//_____________________________________________________________________
TAlphaEventHelix::TAlphaEventHelix( TAlphaEventTrack * Track )
{
  // The only functional TAlphaEventHelix constructor.
  // Requires a TAlphaEventTrack containing 3 hits

  // Initialize variables
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

  fx0 = 0.;
  fy0 = 0.;
  fz0 = 0.;
  
  // load in hits
  const int nhits=Track->GetNHits();
  for( Int_t i = 0; i < nhits; i++)
    {
      //Track->GetHit(i)->Print();
      //fHits.AddLast(  Track->GetHit(i) );
      AddHit( Track->GetHit(i) );
    }

  // determine track parameters
  if( GetNHits() )
   {
     // find the circle parameters first
     fCircleStatus = DetermineCircleParameters(); 
     fLineStatus   = DetermineLineParameters();

     if( fCircleStatus==1 && fLineStatus==1 )
       {
         DetermineSagitta(); 
         First_to_Canonical();
         fHelixStatus = 1;
         return;
       }
     else
       {
         // failed either the circle or line parameter
         // determination. This should be very infrequent
         fHelixStatus = -1;
         return;
       }
   }
  fHelixStatus = 0;
}
void TAlphaEventHelix::FitHelix()
{
   if( fHelixStatus==1 )
   {
      FitLineParameters();
      //fChi2 /= 3.; // Reduce the chi2 by its DOF = (3 zed hits) - (2 param) = 1
      fChi2 /= 1.;
   }
   return;
}

//_____________________________________________________________________
void TAlphaEventHelix::AddHit( TAlphaEventHit * hit )
{
  fHits.push_back(hit);
}

//_____________________________________________________________________
TAlphaEventHelix::~TAlphaEventHelix()
{
//dtor
  //I no longer hold copies hits
  //int size=GetNHits();
  //for (int i=0; i<size; i++)
  //   delete fHits[i];
  fHits.clear();
}

//_____________________________________________________________________
void TAlphaEventHelix::First_to_Canonical( Bool_t Invert )
{
  // Here we convert to the 'canonical' helix parameters 
  // (fc,fd0,fphi0,fLambda,fz0)
  // also, the reference point is moved to the closest point 
  // to the origin

  // first, find the point of closest approach to the origin
  // in the x-y plane
  Double_t phi = TMath::ATan2(fb,fa);
  Double_t d = TMath::Sqrt( fa*fa + fb*fb ) - fR;

  fx0 = d*TMath::Cos(phi);
  fy0 = d*TMath::Sin(phi);
  
  // Calculate most of the other parameters from the initial
  // parameter set. This depends on the sign of the curvature
  if( (fParticleID == 211 && !Invert) || (fParticleID == -211 && Invert) )
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

  // Use the other parameters to find z0
  Double_t x = GetHit(0)->XMRS();
  Double_t y = GetHit(0)->YMRS();
  Double_t z = GetHit(0)->ZMRS();

  // find s, then solve for fx0
  Int_t iflag=0;
  Double_t s = GetsFromR( TMath::Sqrt( x*x + y*y), iflag );
  fz0 = z - fLambda*s;

  fd0_trap = d - 2.2275;
  if( fd0_trap < 0. ) fd0_trap = 0;
  
  // make sure phi0 is positive
  Double_t two_pi = 2*TMath::Pi();
  while (fphi0<0) fphi0 += two_pi;
  while (fphi0>= two_pi) fphi0 -= two_pi;
  /*
  char* string;
  sprintf(string,"TAlphaEventHelix::First_to_Canonical",
                "\n----Canonical-----\nfParticle: %d\nc: %lf\nphi: %lf\nD: %lf\nlambda: %lf\nz0: %lf\n------------------\n",
                fParticleID,fc,fphi0,fd0,fLambda,fz0);
  std::cout<<string<<std::endl;*/
}

//_____________________________________________________________________
Int_t TAlphaEventHelix::DetermineCircleParameters()
{
  // Here we calculate the circle parameters for the helix, given three hits. 
  // Really, the only way this should fail is if the three hits are colinear,
  // in which case, this the track is more of a line than a helix

  //std::cout<<"DetermineCircleParameters"<<
  //				"Calculating circle parameters"<<std::endl;

  const Int_t NHits = GetNHits();

  // If this function is being calling this function with anything other than 
  // three hits, then there is something really wrong. In that case, I will 
  // exit here in a very messy, noisy way, so someone will notice and fix whatever
  // is wrong.
  assert(NHits==3);

  // We will proceed under the assumption that we only have three hits (one in
  // each layer). This assumption should be enforced by the assert statement.
  // From this assumption, the circle parameters should be calcable exactly
  // (unless the track is more consistent with a straight line). We can get away
  // with a little hard coding, after enforcing our assumptions.

  Double_t x[3]; // x coordinates
  Double_t y[3]; // y coordinates

  // Grab the x,y for the three points
  for(Int_t ihit = 0; ihit<3; ihit++) 
    {
      x[ihit] = GetHit(ihit)->XMRS();
      y[ihit] = GetHit(ihit)->YMRS();
    }

  // This method proceedes along the lines of 
  // http://planetmath.org/encyclopedia/ThreePointFormulaForTheCircle.html

  /* The equation for the circle is: (x-fa)^2 + (x-fb)^2 = fR^2,
     which can always be reduced to the form: x^2 + y^2 + Dx + Ey + F = 0,
     this should be satisfied for all three hits, so we can write the 
     system of equations,
     / x1 y1 1 \ /D\  / x1^2 + y1^2 \
     | x2 y2 1 |.|E|= | x2^2 + y2^2 |
     \ x3 y3 1 / \F/  \ x3^2 + y3^2 /
     D, E, and F can be solved using Cramer's rule, assuming that the matix is inveratable
     Finally, completing the squares for the reduced form
     (x+D/2)^2 + (y+E/2)^2 = (D^2 - 4F + E^2)/4,
     or fa = -D/2, fb = -E/2, fR = 0.5*Sqrt(D^2 - 4F + E^2) */


  // Set up the system the matrix
  TMatrixD M(3,3);
  M(0,0) = x[0]; M(0,1) = y[0]; M(0,2) = 1;
  M(1,0) = x[1]; M(1,1) = y[1]; M(1,2) = 1;
  M(2,0) = x[2]; M(2,1) = y[2]; M(2,2) = 1;
 
  // check whether the matrix is invertable
  Double_t Det = M.Determinant();
  if (Det==0)
    {
      // the determinate is zero :( 
      // we can't proceed this way, so abort

      printf("CIRCLE DETERMINATE PROBLEM\n");
     // gEvent->GetVerbose()->Warning("TAlphaEventHelix::DetermineCircleParameters",
	//			    "Determinant evaluates to zero, aborting\n");
      return -1; // determinate exit code
    }
   
  // Solve for D, E, and F, using Cramer's rule

  // intermediate steps...
  Double_t vc[3] = { 0., 0., 0. };
  Double_t  v[3] = { -(x[0]*x[0]+ y[0]*y[0]),
		     -(x[1]*x[1]+ y[1]*y[1]),
		     -(x[2]*x[2]+ y[2]*y[2]) };
  M.Invert(&Det);
  for(Int_t j=0;j<3;j++) 
    for(Int_t k=0;k<3;k++)
      {
        vc[j]+=M(j,k)*v[k];
      }

  // solutions
  Double_t D = vc[0];
  Double_t E = vc[1];
  Double_t F = vc[2];
          
  // Finally, calculate the circle parameters
  Double_t r = 0.25*D*D + 0.25*E*E - F;
  fa = -0.5*D;
  fb = -0.5*E;

  fphi = TMath::ATan2(y[0]-fb,x[0]-fa);
      
  // Extra check for a negative sqrt.
  // This is also indicative of something going horribly wrong
  if (r>0)
    {
      fR = TMath::Sqrt(r);
    }
  else
    {
      printf("CIRCLE SQRT PROBLEM\n");
      return -2; // sqrt exit code
    }

  return 1; // successful exit code
}

//_____________________________________________________________________
Int_t TAlphaEventHelix::DetermineLineParameters()
{
  // Here we determine the non-bending-plane helix parameters.
  // The really important one is lambda, which is proportionality 
  // between the arclength and the z position. 

 // gEvent->GetVerbose()->Message("DetermineLineParameters",
//				"Calculating Line Parameters\n");

  const Int_t NHits = GetNHits();
  // If this function is being calling this function with anything other than 
  // three hits, then there is something really wrong. In that case, I will 
  // exit here in a very messy, noisy way, so someone will notice and fix whatever
  // is wrong.
  assert(NHits==3);

  // We'll need the hit positions
  Double_t x0 = GetHit( 0 )->XMRS();
  Double_t y0 = GetHit( 0 )->YMRS();
  Double_t z0 = GetHit( 0 )->ZMRS();
  Double_t x1 = GetHit( 1 )->XMRS();
  Double_t y1 = GetHit( 1 )->YMRS();
  Double_t z1 = GetHit( 1 )->ZMRS();
          
  // the azimuthal angles are needed to calculate the
  // arclength parameter.
  Double_t phi0 = TMath::ATan2(y0-fb,x0-fa);
  Double_t phi1 = TMath::ATan2(y1-fb,x1-fa);

  // We have to deal with the branching in phi, so we'll
  // look for the minimum of the first turn. 
  Double_t phi0_2pi = phi0 + 2*TMath::Pi();
  Double_t phi1_2pi = phi1 + 2*TMath::Pi();

  Double_t    s[3] = { phi0     - phi1,
	   	       phi0     - phi1_2pi,
		       phi0_2pi - phi1 };
  Double_t sabs[3] = { fabs(phi0     - phi1),
		       fabs(phi0     - phi1_2pi),
		       fabs(phi0_2pi - phi1) };

  // use the min
  Int_t s_idx = TMath::LocMin( 3, sabs );
  Double_t ms01 = s[s_idx];

  if(ms01!=0)
    {
      fth = (z1-z0)/(ms01);
    }
  else
    {
      // I guess this scenerio is possible if phi0 == phi1.
      // hopefully this is very unlikely
     // gEvent->GetVerbose()->Warning("TAlphaEventHelix::MakeDipAngle",
//				    "Arc length error\n");
      return -1;
    }    
  
  // finally, calculate the theta value
  flambda = TMath::ATan2(fth,fR);
  fz0 = z0;

  return 1; // successful exit code
}
#include "manalyzer.h"
//_____________________________________________________________________
Int_t TAlphaEventHelix::FitLineParameters()
{



  minuit2Helix fcn(this);
  ROOT::Minuit2::MnUserParameters upar;
  //Oct 13 fits 1300mW sim
  upar.Add( "fz0"     , fz0     , 0.1, fz0-5    , fz0+5 );
  upar.Add( "fLambda" , fLambda , 0.1, fLambda-1, fLambda+1 );
  
  // create MIGRAD minimizer
  ROOT::Minuit2::MnMigrad mini(fcn, upar);
  //minimdca.SetMaxIterations(10);

  // create Minimizer (default is Migrad)
  mini(40);
  upar =mini.Parameters();
  /*
  
  

std::lock_guard<std::mutex> lock(TAMultithreadHelper::gfLock);
  // Fit the Line Parameters using Minuit

  TMinuit mini(2);
  mini.SetPrintLevel(-1);

  // The function to minimize
  mini.SetFCN(fcnHelix);

  // Use the our best parameters as seed values
  // They should be pretty good already, so don't allow them to 
  // vary too much
  mini.DefineParameter(0, "fz0"     , fz0     , 0.1, fz0-5    , fz0+5 );
  mini.DefineParameter(1, "fLambda" , fLambda , 0.1, fLambda-1, fLambda+1 );

  // For how TMinuit operates, the fitness function (fncHelix) needs
  // to be outside the TAlphaEventHelix object (note the function prototype
  // at the top of this file). So to be able to access the helix private 
  // values, we have to pass TMinuit _this_ object
  mini.SetObjectFit(this);
  
  // Ok. I really hate ROOT for this. Event if I set the print level
  // to a minimum, there is still hardcoded printing to screen, and no
  // way to make it stop without commenting out the commands in 
  // $(ROOTSYS)/math/minuit/src/TMinuit.cxx (which is what I do on my
  // personal ROOT version :( )
  //AO  Int_t err;
 //AO   Double_t tmp[1];
 //AO   tmp[1]=7;
 //AO   mini.mnexcm("SET OUT",tmp,1,err) ;
  // This function shouldn't take that many iterations to converge,
  // but whateves.
  mini.SetMaxIterations(40);
  
  // Perform the function minimization
  mini.Migrad();

  Double_t minifz0;
  Double_t minifLambda;    
  Double_t errfz0;
  Double_t errfLambda;
      
  // Grab the results
  mini.GetParameter(0,minifz0    ,errfz0);
  mini.GetParameter(1,minifLambda,errfLambda);
            
  // This is just for printing the results 
  Double_t FCNbefore = 0.;
  Double_t par[2] = {fz0, fLambda };
  Int_t npar = 2;
  Int_t iflag = 0;
  fcnHelix(npar,NULL,FCNbefore,par,iflag);
      
  Double_t FCNafter  = 0.;
  Double_t par2[2] = {minifz0,minifLambda };
  fcnHelix(npar,NULL,FCNafter,par2,iflag);
            
  // printf("fz0:     %lf %lf +/- %lf\n",fz0,minifz0,errfz0);
  // printf("fLambda: %lf %lf +/- %lf\n",fLambda,minifLambda,errfLambda);
  // printf("chi2: %lf %lf\n",FCNbefore,FCNafter);

  // use the new values
  fz0     = minifz0;
  fLambda = minifLambda;
  fChi2 = FCNafter;
*/
  fz0     = upar.Value(0);
  fLambda = upar.Value(1);
  fChi2   = 0;
  return kTRUE;
}

//_____________________________________________________________________
void fcnHelix(Int_t &/*npar*/, Double_t * /*gin*/ , Double_t &f, Double_t *par, Int_t /*iflag*/ )
{
  // par[0] = z0
  // par[1] = Lambda

  //TAlphaEventHelix * helix = (TAlphaEventHelix*)gMinuit->GetObjectFit();
  TAlphaEventHelix * helix = NULL;

  Double_t chi2 = 0;
  Double_t z0     = par[0];
  Double_t Lambda = par[1];
  const int nhits=helix->GetNHits();
  for( Int_t ihit = 0; ihit < nhits; ihit++ )
    {
      TAlphaEventHit * hit = helix->GetHit( ihit );

      Double_t x = hit->XMRS();
      Double_t y = hit->YMRS();
      Double_t z = hit->ZMRS();

      Int_t iflag=0;
      Double_t s = helix->GetsFromR( TMath::Sqrt( x*x + y*y ), iflag );

      Double_t zprime = z0 + Lambda*s;
      chi2 += (z-zprime)*(z-zprime)/hit->GetNSigma2();
    }

  // return the chi2
  f = chi2;
}

//_____________________________________________________________________
TVector3 TAlphaEventHelix::GetPoint3D( Double_t t )
{ 
  // This function uses flambda, fR, fphi0, and Hit0 as the helix parameters

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
TVector3 TAlphaEventHelix::GetPoint3D_C( const Double_t s )
{ 
  // find a point along the helix, given the arclength parameter
  // this function uses fphi0, fc, fLambda, fx0,fy0,fz as the helix 
  // parameters
  Double_t fc_s_2=2*fc*s;
  
  Double_t SinPhi = TMath::Sin(fphi0);
  Double_t CosPhi = TMath::Cos(fphi0);

  Double_t Sin2_fs_s=TMath::Sin(fc_s_2);
  Double_t Cos2_fs_s=TMath::Cos(fc_s_2);

  Double_t x = fx0 - 1./(2*fc)*SinPhi*Sin2_fs_s - 
    1./(2*fc)*CosPhi*(1-Cos2_fs_s);
  Double_t y = fy0 + 1./(2*fc)*CosPhi*Sin2_fs_s - 
    1./(2*fc)*SinPhi*(1-Cos2_fs_s);
  Double_t z = fz0 + fLambda*s;

  TVector3 pnt(x,y,z);

  return pnt;
}

//_____________________________________________________________________
Double_t TAlphaEventHelix::GetsFromR( const Double_t R, Int_t &iflag )
{
  // find the arclength parameter for a given radius
  
  //printf("R: %lf\n",R);

  Double_t s = 0;
  if(R<fabs(fd0))
    {
      iflag = -1;
      return 0;
    }
  if(R>(2*fR+fabs(fd0)))
    {
      iflag = -2;
      return 0;
    }

  s = 1./fc*TMath::ASin(fc*TMath::Sqrt((R*R-fd0*fd0)/(1.+2.*fd0*fc)));
  iflag = 1;
  return s;
}

//_____________________________________________________________________
Double_t TAlphaEventHelix::GetsFromR_opposite( const Double_t R )
{
  
  //printf("R: %lf\n",R);
  Double_t fabsd=fabs(fd0);
  if(R<fabsd)return 0;
  if(R>(fR+fabsd))return 0;
  Double_t s = 1./fc*(TMath::Pi() - TMath::ASin(fc*TMath::Sqrt((R*R-fd0*fd0)/(1.+2.*fd0*fc))));

  return s;
}

//_____________________________________________________________________
Int_t TAlphaEventHelix::SortHits()
{
  // Figure out a defining position for the helix
  // That is, use the point closest to the axis

  const Int_t NHits = GetNHits();
  TAlphaEventHit *h[NHits];
  Double_t        R[NHits];

  // assign the arrays
  for( Int_t ihits = 0; ihits<NHits; ihits++ )
    {
      h[ihits] = fHits.at(ihits);
      R[ihits] = TMath::Sqrt( h[ihits]->XMRS()*h[ihits]->XMRS() + 
			      h[ihits]->YMRS()*h[ihits]->YMRS() );
    }

  // sort by radius
  Int_t idx[NHits];
  TMath::Sort( NHits, R, idx, kFALSE );

  // reassign the hits
  for( Int_t ihits = 0; ihits<NHits; ihits++ )
    {
      Int_t i = idx[ihits];
      fHits[ihits] = h[i];
    }

  return kTRUE;
}

//_____________________________________________________________________
void TAlphaEventHelix::DetermineSagitta()
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
    fParticleID = -211; // pi-
  else
    fParticleID =  211; // pi+                                                       
}


//_____________________________________________________________________
void TAlphaEventHelix::Print(const Option_t* /* option */) const
{
  printf("\n-------- TAlphaEventHelix -------\n");
  for( size_t iHit = 0; iHit < fHits.size(); iHit++ )
    printf("iHit: %ld  %lf %lf %lf\n",iHit,(fHits.at( iHit ))->XMRS(),
                                          (fHits.at( iHit ))->YMRS(),
                                          (fHits.at( iHit ))->ZMRS());
  printf("a: %lf b: %lf R: %lf \nth: %lf phi: %lf lambda: %lf\n",fa,fb,fR,fth,fphi,flambda);
  printf("-----------------------------------\n\n");
}
