#ifndef __TAlphaEventPCluster__
#define __TAlphaEventPCluster__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventPCluster                                                  //
//                                                                      //
// group of p-side strips                                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TObjArray.h"
#include "TAlphaEventPStrip.h"
#include "TAlphaEventObject.h"

class TAlphaEventPCluster : public TAlphaEventObject {
private:
  TObjArray fStrips;//strips container
  Double_t fADC;   //p-side ADC value
  Double_t fSigma; //summed significance of cluster

public:
  TAlphaEventPCluster(const char* SilName);
  TAlphaEventPCluster(const Int_t SilNum);
  TAlphaEventPCluster() {};
  virtual ~TAlphaEventPCluster();

  void                AddStrip(TAlphaEventPStrip *strip) { fStrips.AddLast(strip); }
  void                Calculate();
  void                Suppress();
  Double_t            GetADC() { return fADC; }
  Double_t            GetSigma() { return fSigma; }
  Int_t               GetNStrips() { return fStrips.GetEntries(); }
  TAlphaEventPStrip * GetStrip(Int_t strip) { return (TAlphaEventPStrip*) fStrips.At(strip); }
  void                SetADC(Double_t ADC)   { fADC   = ADC; }
  void                Print(Option_t* option = "") const;
  
  ClassDef(TAlphaEventPCluster,2);
};

#endif
