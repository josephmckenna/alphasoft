// Methods for TAlphaEventPStrip
#include "TAlphaEventPStrip.h"

//_____________________________________________________________________
TAlphaEventPStrip::TAlphaEventPStrip():
  TAlphaEventStrip()
{
  // ctor
}

//_____________________________________________________________________
TAlphaEventPStrip::TAlphaEventPStrip(Int_t StripNumber, Double_t ADC, Double_t StripSigma):
  TAlphaEventStrip(StripNumber, ADC, StripSigma)
{
  // ctor
}

// end
