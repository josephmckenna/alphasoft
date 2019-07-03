#ifndef _THoughPeakFinder_
#define _THoughPeakFinder_

#include <TObject.h>
#include <TH2.h>
#include <TObjArray.h>

#include "THoughPeak.h"

class THoughPeakFinder : public TObject
{
 private:
  TObjArray fpeaks; // array of peaks
  Double_t  fthr;   // peak threshold
  Double_t  fxlimit;// x limit
  Double_t  fylimit;// y limit

 public:
  THoughPeakFinder();
  THoughPeakFinder( TH2D * hist, Double_t thr );
  ~THoughPeakFinder();

  Int_t Is_Peak( TH2D * hist, Int_t x, Int_t y );
  void Reduce();

  void AddLast( THoughPeak * peak ) { fpeaks.AddLast( peak ); }
  THoughPeak * At( Int_t i ) { return (THoughPeak*) fpeaks.At(i); }

  Int_t GetEntries() { return fpeaks.GetEntries(); }

  void SetThr( Double_t thr ) { fthr = thr; }
  void SetXlimit( Double_t xlimit ) { fxlimit = xlimit; }
  void SetYlimit( Double_t ylimit ) { fylimit = ylimit; }

  ClassDef( THoughPeakFinder, 1 );
};





#endif
