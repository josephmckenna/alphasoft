#ifndef _THoughPeak_
#define _THoughPeak_

#include <TObject.h>

class THoughPeak : public TObject
{
 private:
  Int_t fx;
  Int_t fy;
  Double_t fvotes;

 public:
  THoughPeak() {}
  ~THoughPeak() {}
  THoughPeak( Int_t x, Int_t y, Double_t votes)
    { fx = x; fy = y; fvotes = votes; }

  Int_t GetX() { return fx; }
  Int_t GetY() { return fy; }
  Double_t GetVotes() { return fvotes; }
  
  ClassDef( THoughPeak, 1 );
};

#endif
