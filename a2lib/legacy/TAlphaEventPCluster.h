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
#include "TAlphaEventMap.h"
class TAlphaEventMap;
class TAlphaEventPCluster: public TAlphaEventObject {
private:
  std::vector<TAlphaEventPStrip*>  fStrips;//strips container
  Double_t fADC;   //p-side ADC value
  Double_t fSigma; //summed significance of cluster

public:
  TAlphaEventPCluster(const char* SilName,TAlphaEventMap* m);
  TAlphaEventPCluster(const Int_t SilNum, TAlphaEventMap* m);
  TAlphaEventPCluster(TAlphaEventMap* m) : TAlphaEventObject(m) {};
  ~TAlphaEventPCluster();

  void                AddStrip(TAlphaEventPStrip *strip) { fStrips.push_back(strip); }
  void                Calculate();
  void                Suppress();
  Double_t            GetADC() { return fADC; }
  Double_t            GetSigma() { return fSigma; }
  Int_t               GetNStrips() { return fStrips.size(); }
  TAlphaEventPStrip * GetStrip(Int_t strip) { return (TAlphaEventPStrip*) fStrips.at(strip); }
  void                SetADC(Double_t ADC)   { fADC   = ADC; }
  void                Print(Option_t* option = "") const;
  
  ClassDef(TAlphaEventPCluster,3);
};

#endif
