#include "../include/TSiliconStrip.h"

// TSiliconStrip Class =====================================================================================
//
// Class representing a single Si strip.
//
// JWS 10/10/2008
//
// =========================================================================================================

ClassImp(TSiliconStrip)

TSiliconStrip::TSiliconStrip()
{
  StripNumber=-999;
  RawADC=-999;
  PedSubADC=-999.;
  stripRMS=-999.;
  Hit = false;
}

TSiliconStrip::TSiliconStrip( Int_t  _StripNumber, Int_t _RawADC )
{
  StripNumber=_StripNumber;
  RawADC=_RawADC;
  PedSubADC=-999.;
  stripRMS=-999.;
  Hit = false;
}

TSiliconStrip::~TSiliconStrip()
{
}

TSiliconStrip::TSiliconStrip( TSiliconStrip* & Strip )
{
  StripNumber   = Strip->GetStripNumber();
  RawADC        = Strip->GetRawADC();
  PedSubADC     = Strip->GetPedSubADC();
  stripRMS    = Strip->GetStripRMS();
  Hit           = Strip->IsAHit();
}

void TSiliconStrip::Print() 
{
  Int_t HitInt(0);
  if( Hit ) HitInt = 1;  
  printf( "... Strip Number %d \t Raw ADC %d \t PedSub ADC %f \t Hit %d \n", StripNumber, RawADC, PedSubADC, HitInt );
}

void TSiliconStrip::PrintToFile( FILE * f ) 
{
  fprintf( f, "%d %d ", StripNumber, RawADC );
}
