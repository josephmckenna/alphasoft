#ifndef __TAlphaEventHit__
#define __TAlphaEventHit__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventHit                                                       //
//                                                                      //
// contains n-clusters and p-clusters                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <TObjArray.h>

#include "TAlphaEventPCluster.h"
#include "TAlphaEventNCluster.h"
#include "TAlphaEventObject.h"

class TAlphaEventHit : public TAlphaEventObject {
private:

  Double_t fPSigma2;
  Double_t fNSigma2;

  Int_t fNn;
  Int_t fNp;
  
  Double_t fHitSignificance;

public:
  TAlphaEventHit(const char* SilName);
  TAlphaEventHit(const Int_t SilNum);
  TAlphaEventHit(TAlphaEventHit* hit);
  TAlphaEventHit(const Int_t SilNum, TAlphaEventPCluster * &p, TAlphaEventNCluster * &n);
  TAlphaEventHit(const Char_t *SilName, TAlphaEventPCluster * &p, TAlphaEventNCluster * &n);
  TAlphaEventHit() {};
  virtual ~TAlphaEventHit();

  Int_t GetNn() { return fNn; }
  Int_t GetNp() { return fNp; }

  Double_t GetPSigma2() { return fPSigma2; }
  Double_t GetNSigma2() { return fNSigma2; }

  void SetPSigma2(Double_t p) { fPSigma2=p; }
  void SetNSigma2(Double_t n) { fNSigma2=n; }
  Double_t GetHitSignifance() { return fHitSignificance; }

  void Print(Option_t* option = "") const;
  
  ClassDef(TAlphaEventHit,1);
};

#endif
