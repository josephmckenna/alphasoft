// TAlphaEventObject.cxx
#include <cstring>
#include <stdlib.h>

#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TMath.h>
#include "TAlphaEventObject.h"

ClassImp( TAlphaEventObject )

//______________________________________________________________________________
TAlphaEventObject::TAlphaEventObject(TAlphaEventMap* m)
{
  //ctor
  map=m;

  fX = 0.;
  fY = 0.;
  fZ = 0.;
  fSilNum = -1;

}

//______________________________________________________________________________
TAlphaEventObject::TAlphaEventObject(TAlphaEventMap* m, const Char_t * SilName,   const Bool_t IsSilModule)
{
  map=m;
  fSilNum = ReturnSilNum( SilName );
}

//______________________________________________________________________________
TAlphaEventObject::TAlphaEventObject(TAlphaEventMap* m, const Int_t SilNum, const Bool_t IsSilModule)
{
  map=m;
  fSilNum = SilNum;
}

//______________________________________________________________________________
TAlphaEventObject::~TAlphaEventObject()
{
  // dtor
}

//______________________________________________________________________________
Double_t TAlphaEventObject::X() const
{
  // Return local x coordinate
  return fX;
}

//______________________________________________________________________________
Double_t TAlphaEventObject::Y() const
{
  // Return local y coordinate
  return fY;
}

//______________________________________________________________________________
Double_t TAlphaEventObject::Z() const
{
  // Return local z coordinate
  return fZ;
}

//______________________________________________________________________________
Double_t TAlphaEventObject::XMRS() const
{
  // Return global x coordinate
  return fXMRS;
}

//______________________________________________________________________________
Double_t TAlphaEventObject::YMRS() const
{
  // Return local y coordinate
  return fYMRS;
}

//______________________________________________________________________________
Double_t TAlphaEventObject::ZMRS() const
{
  // Return local z coordinate
  return fZMRS;
}

//______________________________________________________________________________
Double_t TAlphaEventObject::GetCos() const
{
  // Return cos from vertical
  return map->GetCos(fSilNum);
}

//______________________________________________________________________________
Double_t TAlphaEventObject::GetSin() const
{
  // Return sin from vertical
  return map->GetSin(fSilNum); 
}

//______________________________________________________________________________
Double_t TAlphaEventObject::GetXCenter() const
{
  // Return x of the center of the module (sil module)
  return map->GetXCenter(fSilNum);
}

//______________________________________________________________________________
Double_t TAlphaEventObject::GetYCenter() const
{
  // Return y of the center of the module
  return map->GetYCenter(fSilNum);
}

//______________________________________________________________________________
Double_t TAlphaEventObject::GetZCenter() const
{
  // Return z of the center of the module
  return map->GetZCenter(fSilNum);
}

//______________________________________________________________________________
Int_t TAlphaEventObject::GetLayer() const
{
  // Return z of the center of the module
  return map->GetLayer(fSilNum);
}

//______________________________________________________________________________
void TAlphaEventObject::SetXYZ(Double_t x, Double_t y, Double_t z)  
{
  // Set the local coordinates
  fX = x;
  fY = y; 
  fZ = z;
}

//______________________________________________________________________________
void TAlphaEventObject::SetXYZMRS(Double_t x, Double_t y, Double_t z)  
{
  // Set the global coordinates
  fXMRS = x;
  fYMRS = y; 
  fZMRS = z;
}


//______________________________________________________________________________
Int_t TAlphaEventObject::ReturnSilNum(const char* SilName)
{
  if (nSil == 60) {
  // Return the silicon module number 0..59
  // from the silname e.g. 4si5

  Int_t n1 = atoi( &SilName[0] );
  Int_t n2 = 0;
  if( SilName[3] == 'A' )
    n2 = 10;
  else if( SilName[3] == 'B' )
    n2 = 11;
  else
    n2 = atoi( &SilName[3] );
  
  Int_t number = 0;
  
  // AD end
   if( n1 == 0 )
    number = n2 ;
  if( n1 == 1 )
    number = n2 + 8;
  if( n1 == 2 )
    number = n2 + 18;
  // e+ end
  if( n1 == 3 )
    number = n2 + 30;
  if( n1 == 4 )
    number = n2 + 38;
  if( n1 == 5 )
    number = n2 + 48;
  return number;

  }
  
  if(nSil == 72) {
  // Return the silicon module number 0..71
  // from the silname e.g. 4si5

  Int_t n1 = atoi( &SilName[0] );
  Int_t n2 = 0;
  if( SilName[3] == 'A' )
    n2 = 10;
  else if( SilName[3] == 'B' )
    n2 = 11;
  else if( SilName[3] == 'C' )
    n2 = 12;
  else if( SilName[3] == 'D' )
    n2 = 13;
  else
    n2 = atoi( &SilName[3] );
  
  Int_t number = 0;
  
  // AD end
  if( n1 == 0 ) number = n2;	       
  if( n1 == 1 ) number = n2 + 10;      
  if( n1 == 2 ) number = n2 + 10 + 12;
  // e+ end
  if( n1 == 3 ) number = n2 + 10 + 12 + 14;
  if( n1 == 4 ) number = n2 + 10 + 12 + 14 + 10;
  if( n1 == 5 ) number = n2 + 10 + 12 + 14 + 10 + 12;
  return number;
  }
}

 
//______________________________________________________________________________
void TAlphaEventObject::MRS()
{
  fXMRS = fX*GetCos() - fY*GetSin() + GetXCenter();
  fYMRS = fX*GetSin() + fY*GetCos() + GetYCenter();
  fZMRS = fZ;  
}


//______________________________________________________________________________
Double_t TAlphaEventObject::GetnPos(Int_t strip)
{
// given strip number [0,255] returns strip position (z coordinate in MRS)
// strips [0,127] are ASIC 1 which starts at 
// "ASIC1_start" (z coordinate in the local reference frame of the hybrid)
// strips [128,255] are ASIC 2 which starts at
// "ASIC2_start" (z coordinate in the local reference frame of the hybrid)
// values in mm, output in cm

  assert( strip >= 0  );
  assert( strip < 256 );

  double n_pitch = SilNPitch()*10.;
  double PCBmount = 302.25;
  double ASIC1_start = -188.338;
  double ASIC2_start = -72.938;

  double n = 0.;
  double s =(double)strip;
  if( strip < 128 ) n = ASIC1_start - n_pitch*s; // ASIC 1
  else n = ASIC2_start - n_pitch*(s-128.);  // ASIC 2

  n+=PCBmount;
  if( fSilNum < nSil/2 ) n *= -1.;

  return n/10.;
}

//______________________________________________________________________________
Int_t TAlphaEventObject::ReturnNStrip( Double_t pos )
{
// given z (MRS) coordinate of the strip 
// returns number [0,255]

  // in cm
  const Double_t n_pitch = SilNPitch();
  double ASIC1_start = -18.8338;
  double ASIC1_end = -29.9463;
  double ASIC2_start = -7.2938;
  double ASIC2_end = -18.4063;
  double PCBmount = -30.225;
  Int_t strip = N_FAIL();
  Double_t s = 0.;
  
  if ( fSilNum < nSil/2 ) pos *= -1.;
  
  pos += PCBmount;
    
  if ( pos <= ASIC1_start && pos >= ASIC1_end ) // ASIC1
    {
      s = (ASIC1_start - pos )/n_pitch; 
      strip = TMath::Nint(s);
    }

  if ( pos <= ASIC2_start && pos >= ASIC2_end ) // ASIC2
    {
      s = (ASIC2_start - pos )/n_pitch;
      strip = TMath::Nint(s)+128;
    }

  if( pos < ASIC1_end ) // boundary flags
    return N_LEFT();
  if( pos > ASIC2_start )
    return N_RIGHT();
  if( pos > ASIC1_start && pos < ASIC2_end )
    return N_MIDDLE();

//  printf("npos: %lf, s: %lf strip: %d -- %s %d\n",pos,s,strip,ReturnSilName(fSilNum),fSilNum);

  return strip;
}


//______________________________________________________________________________
Double_t TAlphaEventObject::GetpPos(Int_t strip)
{
// given strip number [0,255] returns strip position (y coordinate in the 
// local reference frame of the hybrid)
// strips [0,127] are ASIC 3 which starts at 
// "ASIC3_start" (y coordinate in the local reference frame of the hybrid)
// strips [128,255] are ASIC 4 which starts at
// "ASIC4_start" (y coordinate in the local reference frame of the hybrid)
// values in mm, output in cm

  assert( strip >= 0  );
  assert( strip < 256 );

  double p_pitch = SilPPitch()*10.; // mm
  double ASIC3_start = 28.938;
  double ASIC4_start = -0.1135;

  double p = 0.;
  double s = (double)strip;
  if ( strip < 128 ) p = (ASIC3_start - p_pitch*s); // ASIC 3
  else p = (ASIC4_start - p_pitch*(s-128.)); // ASIC 4
  
  if( fSilNum < nSil/2 ) p *= -1.0;
  
  return p/10.; // return in cm
}

//______________________________________________________________________________
Int_t TAlphaEventObject::ReturnPStrip( Double_t pos )
{
// given y (local reference frame of the hybrid) coordinate of the strip 
// returns strip number [0,255]
  // in cm
  const Double_t p_pitch = SilPPitch();
  double ASIC3_start = 2.8938;
  double ASIC3_end   = 0.01089;
  double ASIC4_start = -0.01135;
  double ASIC4_end   = -2.89425;
  Int_t strip = P_FAIL();
  Double_t s = 0.;

  // Invert the other half of the detector
  if ( fSilNum < nSil/2 ) pos *= -1.;

  if( pos <= ASIC3_start  && pos >= ASIC3_end ) // ASIC3
    {
      s = (ASIC3_start-pos)/p_pitch;
      strip = TMath::Nint(s);
    }
  else if( pos <= ASIC4_start && pos >= ASIC4_end ) // ASIC4
    {
      s = (ASIC4_start-pos)/p_pitch;
      strip = TMath::Nint(s)+128;
    }

  // outside conditions
  if( pos < ASIC4_end )
    return P_LEFT();
  if( pos > ASIC3_start )
    return P_RIGHT();
   if( pos > ASIC4_start && pos < ASIC3_end )
     return P_MIDDLE();

//  printf("ppos: %lf, s: %lf strip: %d -- %s %d\n",pos,s,strip,ReturnSilName(fSilNum),fSilNum);

  return strip;
}
