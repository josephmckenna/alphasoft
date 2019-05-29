#ifndef __TAlphaEventNStrip__
#define __TAlphaEventNStrip__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventStrip                                                      //
//                                                                      //
// Object describing n side silicon strip                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TAlphaEventStrip.h"

class TAlphaEventNStrip : public TAlphaEventStrip {
public:
  TAlphaEventNStrip();
  TAlphaEventNStrip(Int_t StripNumber, Double_t ADC, Double_t StripRMS);
  //TAlphaEventNStrip(TAlphaEventNStrip* &);
  virtual ~TAlphaEventNStrip() {};

  ClassDef(TAlphaEventNStrip,1);
};

#endif
