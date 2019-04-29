// TAlphaEventObject.cxx
#include <cstring>
#include <stdlib.h>

#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TMath.h>
#include "TAlphaEventObject.h"

ClassImp( TAlphaEventObject )

//______________________________________________________________________________
TAlphaEventObject::TAlphaEventObject()
{
  //ctor
  fX = 0.;
  fY = 0.;
  fZ = 0.;
  fXMRS = 0.;
  fYMRS = 0.;
  fZMRS = 0.;
  fCos = 0.;
  fSin = 0.;
  fXCenter = 0.;
  fYCenter = 0.;
  fZCenter = 0.;
  fLayer = -1;
  fSilNum = -1;
  fSilName[0] = (char)NULL;
  fSilName[1] = (char)NULL;
  fSilName[2] = (char)NULL;
  fSilName[3] = (char)NULL;
  fSilName[4] = (char)NULL;
}

//______________________________________________________________________________
TAlphaEventObject::TAlphaEventObject(const Char_t * SilName, const Bool_t IsSilModule)
{
  fSilNum = ReturnSilNum( SilName );
  fLayer = ReturnLayer( fSilNum );
  strcpy( fSilName, SilName );
  SetName( fSilName );

  SetValues( fSilName, IsSilModule );
}

//______________________________________________________________________________
TAlphaEventObject::TAlphaEventObject(const Int_t SilNum, const Bool_t IsSilModule)
{
  fSilNum = SilNum;
  fLayer = ReturnLayer( fSilNum );
  char* retsilname = ReturnSilName( fSilNum);
  strcpy( fSilName, retsilname );
  SetName( fSilName );
  delete[] retsilname;

  SetValues( fSilName, IsSilModule );
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
  return fCos;
}

//______________________________________________________________________________
Double_t TAlphaEventObject::GetSin() const
{
  // Return sin from vertical
  return fSin;
}

//______________________________________________________________________________
Double_t TAlphaEventObject::GetXCenter() const
{
  // Return x of the center of the module (sil module)
  return fXCenter;
}

//______________________________________________________________________________
Double_t TAlphaEventObject::GetYCenter() const
{
  // Return y of the center of the module
  return fYCenter;
}

//______________________________________________________________________________
Double_t TAlphaEventObject::GetZCenter() const
{
  // Return z of the center of the module
  return fZCenter;
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
Int_t TAlphaEventObject::ReturnLayer(Int_t SilNum)
{
  if(nSil == 72){
  // Take the module number 0..71
  // return the detector layer
  
  if(SilNum < 10) return 0;
  else if(SilNum >= 10 && SilNum < 22) return 1;
  else if(SilNum >= 22 && SilNum < 36) return 2;
  else if(SilNum >= 36 && SilNum < 46) return 0;
  else if(SilNum >= 46 && SilNum < 58) return 1;
  else if(SilNum >= 58 && SilNum < 72) return 2;

  return -1;
  }
  if(nSil == 60) {

  // Take the module number 0..59
  // return the detector layer

  if(SilNum < 8) return 0;
  else if(SilNum >= 8  && SilNum < 18) return 1;
  else if(SilNum >= 18 && SilNum < 30) return 2;
  else if(SilNum >= 30 && SilNum < 38) return 0;
  else if(SilNum >= 38 && SilNum < 48) return 1;
  else if(SilNum >= 48 && SilNum < 60) return 2;

  return -1;
  }
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
  fXMRS = fX*fCos - fY*fSin + fXCenter;
  fYMRS = fX*fSin + fY*fCos + fYCenter;
  fZMRS = fZ;  
}

//______________________________________________________________________________
char * TAlphaEventObject::ReturnSilName(Int_t SilNum)
{
  assert( SilNum >= 0 );
  assert( SilNum < nSil );
  
  if(nSil == 72){
  // Return the silname e.g. 4si5
  // from the silicon module number 0..71

  char * name = new char[5];
  
  // AD end
  if ( SilNum < 10 ) sprintf(name,"0si%1d",SilNum);
  if ( SilNum >= 10 && SilNum < 22 )
  {
      if(SilNum == 20) sprintf(name, "1siA" );
      else if ( SilNum == 21 ) sprintf(name, "1siB");
      else  sprintf(name,"1si%1d",SilNum - 10 );
  }
  if ( SilNum >= 22 && SilNum < 36 )
  {
      if( SilNum == 32 ) sprintf( name,"2siA" );
      else if( SilNum == 33 ) sprintf( name,"2siB" );
      else if( SilNum == 34 ) sprintf( name,"2siC" );
      else if( SilNum == 35 ) sprintf( name,"2siD" );
      else sprintf(name,"2si%d",SilNum - 10 - 12);
  }

  if ( SilNum >= 36 && SilNum < 46 ) sprintf(name,"3si%1d",SilNum - 10 - 12 - 14);
  if ( SilNum >= 46 && SilNum < 58 )
  {
      if (SilNum == 56) sprintf(name, "4siA");
      else if (SilNum == 57) sprintf(name, "4siB");
      else sprintf(name,"4si%1d",SilNum - 10 - 12 - 14 - 10);
  }
  if ( SilNum >= 58 && SilNum < 72 )
  {
      if( SilNum == 68 ) sprintf( name,"5siA" );
      else if( SilNum == 69 ) sprintf( name,"5siB" );
      else if( SilNum == 70 ) sprintf( name,"5siC" );
      else if( SilNum == 71 ) sprintf( name,"5siD" );
      else sprintf(name,"5si%1d",SilNum - 10 - 12 - 14 - 10 - 12);
  }
  return name;
}
  
  
  if(nSil ==60) {
    // Return the silname e.g. 4si5
    // from the silicon module number 0..59

    char * name = new char[5];
  // AD end
    if ( SilNum < 8 )
      {
        sprintf(name,"0si%1d",SilNum);
      }
    if ( SilNum >= 8 && SilNum < 18 )
      {
        sprintf(name,"1si%1d",SilNum - 8);
      }
    if ( SilNum >=18 && SilNum < 30 )
      {
        if( SilNum == 28 )
          sprintf( name,"2siA" );
        else if( SilNum == 29 )
          sprintf( name,"2siB" );

        else
          sprintf(name,"2si%d",SilNum - 18);
      }
      
    // e+ end
    if ( SilNum >= 30 && SilNum < 38 )
      {
        sprintf(name,"3si%1d",SilNum - 30);
      }
    if ( SilNum >= 38 && SilNum < 48 )
      {
        sprintf(name,"4si%1d",SilNum - 38);
      }
    if ( SilNum >= 48 && SilNum < 60 )
      {
        if( SilNum == 58)
          sprintf( name,"5siA" );
        else if( SilNum == 59 )
          sprintf( name,"5siB" );
        else
          sprintf(name,"5si%1d",SilNum - 48);
  
      }
  return name;
  }
}

//______________________________________________________________________________
void TAlphaEventObject::SetValues(const char * SilName, const Bool_t IsSilModule )
{
  assert( gGeoManager );

  fX = 0.;
  fY = 0.;
  fZ = 0.;
  fXMRS = 0.;
  fYMRS = 0.;
  fZMRS = 0.;
  fCos = 0.;
  fSin = 0.;
  fXCenter = 0.;
  fYCenter = 0.;
  fZCenter = 0.;

  // Grab the module parameters from the GeoManager
  Int_t n = (Int_t) gGeoManager->GetTopVolume()->GetNodes()->GetEntries();
  TGeoNode* node = NULL;
  for( Int_t i = 0; i < n; i++ )
    {
      node = gGeoManager->GetTopVolume()->GetNode( i );
      if( strncmp(node->GetName(),SilName,4) == 0 ) break;
      else node = NULL;
    }
  
  if( node )
    {
      fXCenter = node->GetMatrix()->GetTranslation()[0];
      fYCenter = node->GetMatrix()->GetTranslation()[1];
      fZCenter = node->GetMatrix()->GetTranslation()[2];

      Double_t radius = TMath::Sqrt( fXCenter*fXCenter + fYCenter*fYCenter );
      fCos = fXCenter/radius;
      fSin = fYCenter/radius;
    }
  else
    {
      fCos = 0;
      fSin = 0;
      fXCenter = 0;
      fYCenter = 0;
      fZCenter = 0;
    }

  if( IsSilModule && node)
    {
      fXMRS = fX*fCos - fY*fSin + fXCenter;
      fYMRS = fX*fSin + fY*fCos + fYCenter;
      fZMRS = fZ;// + fZCenter;
    }
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
