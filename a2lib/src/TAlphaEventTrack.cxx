#include "TAlphaEventTrack.h"
#include <TMath.h>
#include <cmath>

//_____________________________________________________________________
TAlphaEventTrack::TAlphaEventTrack()
{
  fcor = 0.;

  faxy = 0.;
  fbxy = 0.;
  fr2xy= 0.;
  
  fayz = 0.;
  fbyz = 0.;
  fr2yz= 0.;
  
  fresxy = 0.;
  fresyz = 0.;
  
  fRES=-999.;
  fDCA=-999.;
	
  funitvector.SetXYZ(0.,0.,0.);
  fr0.SetXYZ(0.,0.,0.);
 
}
TAlphaEventTrack::~TAlphaEventTrack()
{
  //int s=GetNHits();
  //for (int i=0; i<s; i++)
  //   delete fHitArray[i];
  fHitArray.clear();
}
void TAlphaEventTrack::AddHit( TAlphaEventHit* hit ) { 
    fHitArray.push_back( hit ); 
  }
//_____________________________________________________________________
void TAlphaEventTrack::LeastSquares()
{
   // Least squares fitting http://mathworld.wolfram.com/LeastSquaresFitting.html

    Int_t nPnt = GetNHits();
    Double_t x[nPnt];
    Double_t y[nPnt];
    Double_t z[nPnt];
    
    for(Int_t i = 0; i < nPnt; i++)
	  {
	    x[i] = GetHit(i)->XMRS();
	    y[i] = GetHit(i)->YMRS();
	    z[i] = GetHit(i)->ZMRS();
	  }
	  
    Double_t Sx = 0; // compute the sums
    Double_t Sy = 0;
    Double_t Sz = 0; 
    Double_t Sxx = 0;
    Double_t Syy = 0;
    Double_t Sxy = 0;
    Double_t Szz = 0;
    Double_t Szy = 0;
    
    for( int i = 0; i < nPnt; i++)
	  {
	    Sx += x[i];
	    Sy += y[i];
	    Sz += z[i];
	    Sxx += x[i]*x[i];
	    Syy += y[i]*y[i];
	    Sxy += x[i]*y[i];
	    Szz += z[i]*z[i];
	    Szy += z[i]*y[i];
	  } 
    //printf("Sz: %lf, Sy: %lf, Szz: %lf, Szy: %lf\n",Sz,Sy,Szz,Szy); 
	  
    faxy = (Sy*Sxx - Sx*Sxy)/(nPnt*Sxx - Sx*Sx);  // calculate the linear coefficients
    fbxy = (nPnt*Sxy - Sx*Sy)/(nPnt*Sxx - Sx*Sx);
    
    fayz = (Sy*Szz - Sz*Szy)/(nPnt*Szz - Sz*Sz);  
    fbyz = (nPnt*Szy - Sz*Sy)/(nPnt*Szz - Sz*Sz);
    
    //printf("a: %lf, b: %lf\n",a,b);
	  
	  
    // compute the sums of squares
    Double_t ssxx = 0;
    Double_t ssyy = 0;
    Double_t ssxy = 0;
    Double_t sszz = 0;
    Double_t sszy = 0;

    for(int i = 0;i<nPnt;i++)
	  {
           ssxx += (x[i]-Sx)*(x[i]-Sx);
           ssyy += (y[i]-Sy)*(y[i]-Sy);
           ssxy += (x[i]-Sx)*(y[i]-Sy);
           sszz += (z[i]-Sz)*(z[i]-Sz);
           sszy += (z[i]-Sz)*(y[i]-Sy);
	  }
    //printf("sszz: %lf  ssyy: %lf  sszy: %lf\n",sszz,ssyy,sszy);
	  
    // compute the correlation coefficient
    fr2xy = ssxy*ssxy / (ssxx*ssyy);
    fr2yz = sszy*sszy / (sszz*ssyy);  
	
	// Compute the standard error for a and b
	Double_t sxy = sqrt( ( ssyy - ( ssxy * ssxy ) / ssxx ) / ( nPnt - 2 ) );
	fdaxy = sxy * sqrt( 1 / nPnt + Sx*Sx / ssxx );
	fdbxy = sxy / sqrt( ssxx );
	
	Double_t szy = sqrt( ( sszz - ( sszy * sszy ) / ssyy ) / ( nPnt - 2 ) );
	fdayz = szy * sqrt( 1 / nPnt + Sy*Sy / ssyy );
	fdbyz = szy / sqrt( ssyy );
}

//_____________________________________________________________________
void TAlphaEventTrack::Residuals()
{
    Int_t nPnt = GetNHits();
    Double_t x[nPnt];
    Double_t y[nPnt];
    Double_t z[nPnt];
	  
    for(Int_t i = 0; i < nPnt; i++)
	  {
	    x[i] = GetHit(i)->XMRS();
	    y[i] = GetHit(i)->YMRS();
	    z[i] = GetHit(i)->ZMRS();
	  }
	  
    for(Int_t i = 0; i< nPnt; i++)
	  {
	     Double_t cos = GetHit(i)->GetCos();
		 Double_t sin = GetHit(i)->GetSin();
		   
		 TVector3 h( Getunitvector() );
		 TVector3 r0( Getr0() ); // First point
		 if( r0.X() == 0. && r0.Y() == 0. && r0.Z() == 0 ) continue; // fit failed
		 TVector3 r1( r0 + 1*h );		   // Second points
		 TVector3 n( cos, sin, 0);
		   
		 // Find the parameter that leaves us in the plane of the module	 
		 Double_t d = -n.X()*x[i] - n.Y()*y[i] - n.Z()*z[i];
		 Double_t t = ( d + n * r0 ) / ( n * (r0 -r1) );
		 // Impact point of the fitted line on plane of the module
		 TVector3 impact( r0 + t*(r1 - r0) );  
		 
		 // Transform into local coordinates  
		 TVector3 impactlocal( impact.X()*cos + impact.Y()*sin,
							  -impact.X()*sin + impact.Y()*cos,
							   impact.Z());
		   
		 TVector3 coorlocal( x[i]*cos + y[i]*sin,-x[i]*sin + y[i]*cos, z[i]);
		   
		   
		 // Calculate Residuals
		 Double_t yres = coorlocal.Y() - impactlocal.Y();
		 Double_t zres = coorlocal.Z() - impactlocal.Z();
		  
		 fresxy += fabs(yres);
		 fresyz += fabs(zres);
	  }
  

}

//_____________________________________________________________________
void TAlphaEventTrack::MakeLine()
{	
	// We have two 3d planes
	//    0 = axy + bxy * x - y   (1)
	//    0 = ayz + byz * z - y   (2)
	// The intersection of which forms a line
	// So, first considering the normal vectors to the planes
	// n1 = (n1x,n1y,n1z) n2 = (n2x,n2y,n2z)
	// where, using the Hessian normal form,
	//  n1x = bxy/sqrt(bxy - 1) n1y = -1/sqrt(byx - 1) n1z = 0
	//  n2x = 0                 n2y = -1/sqrt(byz - 1) n1z = byz/sqrt(byz - 1)
	// A line is formed only if the planes are not parallel,
	// which means than n1 x n2 =/= 0
	// so  n1y*n2z - n1z*n2y =/= 0 
	// or  n1z*n2x - n1x*n2z =/= 0 
	// or  n1x*n2y - n1y*n2x =/= 0  Aslong as one of these is nonzero
	// (n1y*n2z - n1z*n2y, n1z*n2x - n1x*n2z, n1x*n2y - n1y*n2x) is
	// parallel to the line
	// So ( -1*byz , - bxy * byz , bxy * -1 )
	
	TVector3 temp( - fbyz , - fbxy * fbyz , - fbxy );
	
	// Make the vector unit length
	funitvector = temp.Unit();
	
	// Then calculate a point on the line from the two equations
	// So set y = 0
	// Then r0 = ( -axy/bxy , 0 , -ayz/byz )
	
	if( fabs(fbxy) > 1e-10 && fabs(fbyz) > 1e-10)
		fr0.SetXYZ( -faxy/fbxy , 0. , -fayz/fbyz );
	else
		fr0.SetXYZ( 0. , 0. , 0. ); // fit failed
}
void TAlphaEventTrack::MakeLinePCA()
{
printf("I SHOULD NOT HAPPEN!!!\n");
}
//_____________________________________________________________________
void TAlphaEventTrack::MakeLinePCA(TPrincipal &princomp)
{
  // This method is used to calculate the linear correlation of the
  // track hits in the Master Reference Frame using a principal
  // components analysis method.

  Int_t nPnt = GetNHits();
  
  princomp.Clear();
  
  for(Int_t i = 0; i < nPnt; i++)
    {
      Double_t x = GetHit( i )->XMRS();
      Double_t y = GetHit( i )->YMRS();
      Double_t z = GetHit( i )->ZMRS();

      //printf("%lf %lf %lf\n",x,y,z);
      Double_t xyz[3] = {x,y,z};
      princomp.AddRow( xyz );
    }

  // A problem that can occur is if two or more of the rows
  // contain the same column values. This will end up causing
  // a div by zero exception, so we need to check for this.
  // In this case, the hits have to be very linear, usually
  // straight up and down in the Z coordinate. So I'll just
  // set fcor = 1 and return, relying on the rest of the 
  // tracking to catch if this track is actually really shitty
  TMatrixD * pcov = (TMatrixD*)princomp.GetCovarianceMatrix();
  Double_t * dov = pcov->GetMatrixArray();
  if( dov[0] < 0.000001 || dov[4] < 0.000001 || dov[8] < 0.000001 )
    {
      fcor = 1.;
      return;
    }
  
  princomp.MakePrincipals();

  //princomp.Print("MSE");

  // Principal components (eigenvalues)
  Double_t * v;
  v = ((TVectorD*)princomp.GetEigenValues())->GetMatrixArray();
  fcor=v[0];

  // The eigenfunction forms the direction vector
  Double_t * ev = ((TMatrixD*)princomp.GetEigenVectors())->GetMatrixArray();
  
  TVector3 temp(ev[0],ev[3],ev[6]);
  funitvector = temp.Unit();

  Double_t mx = ((TVectorD*)princomp.GetMeanValues())->GetMatrixArray()[0];
  Double_t my = ((TVectorD*)princomp.GetMeanValues())->GetMatrixArray()[1];
  Double_t mz = ((TVectorD*)princomp.GetMeanValues())->GetMatrixArray()[2];
  fr0.SetXYZ(mx,my,mz);
  return;
}

//_____________________________________________________________________
Int_t TAlphaEventTrack::SortHits()
{
  // Figure out a defining position for the helix
  // That is, use the point closest to the axis

  Int_t NHits = GetNHits();
  TAlphaEventHit *h[NHits];
  Double_t        R[NHits];
  
  // assign the arrays
  for( Int_t ihits = 0; ihits<NHits; ihits++ )
    {
      h[ihits] = fHitArray.at(ihits);
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
      fHitArray[ihits] = h[i];
      //delete h[i];
    }
  return kTRUE;
}

//_____________________________________________________________________
void TAlphaEventTrack::Clear(Option_t *)
{
  fHitArray.clear();
  funitvector.SetXYZ(-999.,-999.,-999);
  fr0.SetXYZ(-999.,-999.,-999);
  fcor=-999.;
  fRES=-999.;
  fDCA=-999.;
}
//_____________________________________________________________________
Double_t TAlphaEventTrack::CalculateTheResidual()
{
	if(GetNHits()!=6) return -999.;
	// Calculate the sum of the perpendicular distances to the fitted line (the residual)
	Double_t PR = 0.;
	Double_t x,y,z,QR;
	TVector3 QP;

	for(Int_t n=0; n < 6; ++n)
	{
		x = GetHit(n)->XMRS();
		y = GetHit(n)->YMRS();
		z = GetHit(n)->ZMRS();
		  
		QP.SetXYZ( fr0.X() - x, fr0.Y() - y, fr0.Z() - z );
		QR = TMath::Abs(QP.Dot(funitvector));
		
		PR += QP.Dot(QP) - QR*QR ;
		//printf("PR: %f \t Total: %f \n",QP.Dot(QP)-QR*QR,PR);
	}

	fRES = PR;
	return PR;
}

//_____________________________________________________________________
Double_t TAlphaEventTrack::DetermineDCA()
{
	Double_t ux,uy,x0,y0,tbar,DCA;
	
	ux=funitvector.X();
	uy=funitvector.Y();
	x0=fr0.X();
	y0=fr0.Y();
	if((ux*ux+uy*uy)==0.) return -1.;
	tbar=-(x0*ux+y0*uy)/(ux*ux+uy*uy);
	DCA=TMath::Sqrt(TMath::Power(x0+ux*tbar,2.)+TMath::Power(y0+uy*tbar,2.));
	
	fDCA=DCA;
	return DCA;
}
