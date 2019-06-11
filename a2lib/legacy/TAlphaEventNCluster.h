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
  std::vector<int> fStripNumber; //full board strip number (0..255)
  std::vector<double> fADCs;   //ADC value
  std::vector<double> fRMS;  //RMS of strip
  
  Double_t fADC;   //n-side ADC value
  Double_t fSigma; //summed significance of cluster

public:
  TAlphaEventNCluster(const char* SilName,TAlphaEventMap* m);
  TAlphaEventNCluster(const Int_t SilNum,TAlphaEventMap* m);
  TAlphaEventNCluster(TAlphaEventMap* m): TAlphaEventObject(m) {};
  TAlphaEventNCluster(){};
  ~TAlphaEventNCluster();

 // void                AddStrip(TAlphaEventPStrip *strip) { fStrips.push_back(strip); }
  void                Reserve(int i)
  {
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
  
  ClassDef(TAlphaEventNCluster,4);
};

#endif
