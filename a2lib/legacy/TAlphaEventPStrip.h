#ifndef __TAlphaEventPStrip__
#define __TAlphaEventPStrip__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventPStrip                                                      //
//                                                                      //
// Object describing p-side silicon strip                               //
// Inherits from TAlphaEventStrip
//////////////////////////////////////////////////////////////////////////

#include "TAlphaEventStrip.h"

class TAlphaEventPStrip : public TAlphaEventStrip {
public:
  TAlphaEventPStrip();
  TAlphaEventPStrip(Int_t StripNumber, Double_t ADC, Double_t StripRMS);
  virtual ~TAlphaEventPStrip() {};

  ClassDef(TAlphaEventPStrip,1);
};

#endif
