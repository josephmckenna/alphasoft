#ifndef __TAlphaEventPCluster__
#define __TAlphaEventPCluster__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventPCluster                                                  //
//                                                                      //
// group of p-side strips                                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TAlphaEventObject.h"

class TAlphaEventPCluster: public TAlphaEventObject {
private:
  int nStrips;
  
  Double_t MeanStrip;
  Double_t fADC;   //p-side ADC value
  Double_t fSigma; //summed significance of cluster

public:
  TAlphaEventPCluster(const char* SilName,TAlphaEventMap* m);
  TAlphaEventPCluster(const Int_t SilNum, TAlphaEventMap* m);
  TAlphaEventPCluster(TAlphaEventMap* m) : TAlphaEventObject(m) {};
  TAlphaEventPCluster(){};
  ~TAlphaEventPCluster();

  void                Calculate(const int firstStrip,const int nStrips,const double* adc, const double* rms);

  Double_t            GetADC() { return fADC; }
  Double_t            GetSigma() { return fSigma; }
  Int_t               GetNStrips() { return nStrips; }
  void                SetADC(Double_t ADC)   { fADC   = ADC; }
  void                Print(Option_t* option = "") const;
  
  ClassDef(TAlphaEventPCluster,5);
};

#endif
