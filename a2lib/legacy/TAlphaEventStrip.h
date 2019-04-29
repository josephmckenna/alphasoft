#ifndef __TAlphaEventStrip__
#define __TAlphaEventStrip__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventStrip                                                      //
//                                                                      //
// Base strip class
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <TObject.h>

class TAlphaEventStrip : public TObject {
private:

  Int_t fStripNumber; //full board strip number (0..255)
  Double_t fADC;   //ADC value
  Double_t fRMS;  //RMS of strip

public:
  // constructors
  TAlphaEventStrip();
  TAlphaEventStrip(Int_t StripNumber, Double_t ADC, Double_t fStripSigma);
  
  // destructor (nothing should be on the heap)
  virtual ~TAlphaEventStrip() {};

  // getters
  Double_t GetADC() { return fADC; }
  Int_t GetStripNumber() { return fStripNumber; }
  Double_t GetStripRMS() { return fRMS; }
  // setters
  void SetADC(Double_t ADC)   { fADC = ADC; }
  void SetStripNumber(Int_t StripNumber) { fStripNumber = StripNumber; }

  virtual void Print(Option_t* option = "") const;

  ClassDef(TAlphaEventStrip,1);
};

#endif
