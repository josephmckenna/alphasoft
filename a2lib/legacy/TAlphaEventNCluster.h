#ifndef __TAlphaEventNCluster__
#define __TAlphaEventNCluster__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventNCluster                                                  //
//                                                                      //
// group of n-side strips                                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TAlphaEventNStrip.h"
#include "TAlphaEventObject.h"

class TAlphaEventNCluster:public TAlphaEventObject {
private:
  std::vector<TAlphaEventNStrip*> fStrips;//strips container
  Double_t fADC;   //n-side ADC value
  Double_t fSigma; //summed significance of cluster

public:
  TAlphaEventNCluster(const char* SilName,TAlphaEventMap* m);
  TAlphaEventNCluster(const Int_t SilNum,TAlphaEventMap* m);
  TAlphaEventNCluster(TAlphaEventMap* m): TAlphaEventObject(m) {};
  virtual ~TAlphaEventNCluster();

  void                AddStrip(TAlphaEventNStrip *strip) { fStrips.push_back(strip); }
  void                Calculate();
  void                Suppress();
  Double_t            GetADC() { return fADC; }
  Double_t            GetSigma() { return fSigma; }
  Int_t               GetNStrips() { return fStrips.size(); }
  TAlphaEventNStrip * GetStrip(Int_t strip) { return fStrips.at(strip); }
  void                SetADC(Double_t ADC)   { fADC   = ADC; }
  void                Print(Option_t* option = "") const;
  
  ClassDef(TAlphaEventNCluster,3);
};

#endif
