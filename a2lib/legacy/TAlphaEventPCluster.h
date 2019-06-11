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
  std::vector<int> fStripNumber; //full board strip number (0..255)
  std::vector<double> fADCs;   //ADC value
  std::vector<double> fRMS;  //RMS of strip
  
  Double_t fADC;   //p-side ADC value
  Double_t fSigma; //summed significance of cluster

public:
  TAlphaEventPCluster(const char* SilName,TAlphaEventMap* m);
  TAlphaEventPCluster(const Int_t SilNum, TAlphaEventMap* m);
  TAlphaEventPCluster(TAlphaEventMap* m) : TAlphaEventObject(m) {};
  TAlphaEventPCluster(){};
  ~TAlphaEventPCluster();

 // void                AddStrip(TAlphaEventPStrip *strip) { fStrips.push_back(strip); }
  void                Reserve(int i)
  {
    if (i<=0) return;
    fStripNumber.reserve(i);
    fADCs.reserve(i);
    fRMS.reserve(i);
  }
  void                AddStrip(int i, double adc, double rms)
  {
    nStrips++;
    fStripNumber.push_back(i);
    fADCs.push_back(adc);
    fRMS.push_back(rms);
  }
  void                Calculate();
  void                Suppress();
  Double_t            GetADC() { return fADC; }
  Double_t            GetSigma() { return fSigma; }
  Int_t               GetNStrips() { return nStrips; }
  void                SetADC(Double_t ADC)   { fADC   = ADC; }
  void                Print(Option_t* option = "") const;
  
  ClassDef(TAlphaEventPCluster,4);
};

#endif
