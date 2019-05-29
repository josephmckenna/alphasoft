#ifndef __TAlphaEventNCluster__
#define __TAlphaEventNCluster__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventNCluster                                                  //
//                                                                      //
// group of n-side strips                                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TObjArray.h"
#include "TAlphaEventNStrip.h"
#include "TAlphaEventObject.h"

class TAlphaEventNCluster : public TAlphaEventObject {
private:
  TObjArray fStrips;//strips container
  Double_t fADC;   //n-side ADC value
  Double_t fSigma; //summed significance of cluster

public:
  TAlphaEventNCluster(const char* SilName);
  TAlphaEventNCluster(const Int_t SilNum);
  TAlphaEventNCluster() {};
  virtual ~TAlphaEventNCluster();

  void                AddStrip(TAlphaEventStrip *strip) { fStrips.AddLast( (TObject*) strip); }
  void                Calculate();
  void                Suppress();
  Double_t            GetADC() { return fADC; }
  Double_t            GetSigma() { return fSigma; }
  Int_t               GetNStrips() { return fStrips.GetEntries(); }
  TAlphaEventNStrip * GetStrip(Int_t strip) { return (TAlphaEventNStrip*) fStrips.At(strip); }
  void                SetADC(Double_t ADC)   { fADC   = ADC; }
  void                Print(Option_t* option = "") const;
  
  ClassDef(TAlphaEventNCluster,2);
};

#endif
