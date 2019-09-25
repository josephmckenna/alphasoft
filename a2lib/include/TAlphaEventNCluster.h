#ifndef __TAlphaEventNCluster__
#define __TAlphaEventNCluster__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventNCluster                                                  //
//                                                                      //
// group of n-side strips                                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TAlphaEventObject.h"

class TAlphaEventNCluster:public TAlphaEventObject {
private:
  int nStrips;
  
  Double_t MeanStrip;
  Double_t fADC;   //n-side ADC value
  Double_t fSigma; //summed significance of cluster

public:
  TAlphaEventNCluster(const char* SilName,TAlphaEventMap* m);
  TAlphaEventNCluster(const Int_t SilNum,TAlphaEventMap* m);
  TAlphaEventNCluster(TAlphaEventMap* m): TAlphaEventObject(m) {};
  TAlphaEventNCluster(){};
  ~TAlphaEventNCluster();

  void                Calculate(int firstStrip,int nStrips,double* adc, double* rms);

  Double_t            GetADC() { return fADC; }
  Double_t            GetSigma() { return fSigma; }
  Int_t               GetNStrips() { return nStrips; }
  void                SetADC(Double_t ADC)   { fADC   = ADC; }
  void                Print(Option_t* option = "") const;
  
  ClassDef(TAlphaEventNCluster,5);
};

#endif
