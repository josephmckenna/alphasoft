// Methods for TAlphaEventNStrip
#include "TAlphaEventNStrip.h"

//_____________________________________________________________________
TAlphaEventNStrip::TAlphaEventNStrip():
  TAlphaEventStrip()
{
  // ctor
}

//_____________________________________________________________________
TAlphaEventNStrip::TAlphaEventNStrip(Int_t StripNumber, Double_t ADC, Double_t StripSigma):
  TAlphaEventStrip(StripNumber, ADC, StripSigma)
{
  // ctor
}

//end
