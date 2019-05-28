#include "TAlphaEventStrip.h"

//_____________________________________________________________________
TAlphaEventStrip::TAlphaEventStrip()
{
  fStripNumber = 0;
  fADC = 0.;
  fRMS =0.;
}

//_____________________________________________________________________
TAlphaEventStrip::TAlphaEventStrip(Int_t StripNumber, Double_t ADC, Double_t StripRMS)
{
  fStripNumber = StripNumber;
  fADC = ADC;
  fRMS = StripRMS;
}

//_____________________________________________________________________
void TAlphaEventStrip::Print(Option_t*  /*option*/) const
{
  std::cout << fStripNumber << "\t" << fADC << "\t" << fRMS<<std::endl;
}

// end
