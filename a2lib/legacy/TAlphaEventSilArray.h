// TAlphaEventSilArray.h

#ifndef __TAlphaEventSilArray__
#define __TAlphaEventSilArray__

#include <TObject.h>
#include <TString.h>

class TAlphaEventSilArray : public TObject
{
 private:
  Double_t fArray[128];
  TString fSilName;
  Int_t fASIC;

 public:
  TAlphaEventSilArray( Int_t ASIC, TString SilName, Double_t * Array );
  TAlphaEventSilArray();
  ~TAlphaEventSilArray();

  void SetArray( Int_t i, Double_t x ) { fArray[i] = x; }
  void SetSilName( TString silName )    { fSilName = silName; }
  void SetASIC( Int_t asic )        { fASIC = asic; }

  Double_t GetArray( Int_t i )      { return fArray[i]; }
  const char * GetSilName()                 { return fSilName.Data(); }
  Int_t GetASIC()                   { return fASIC; }

  Double_t operator[] (Int_t i) const;
  Double_t &operator[] (Int_t i);

  virtual void Print(Option_t * opt = "") const;

  ClassDef( TAlphaEventSilArray, 1 );
};

inline Double_t TAlphaEventSilArray::operator[] (Int_t i) const
{
  if( (i < 0) || (i>127) ) return 0.;
  return fArray[i];
}

inline Double_t &TAlphaEventSilArray::operator[] (Int_t i)
{
  if( (i < 0) || (i > 127) ) i = 0;
  return fArray[i];
}

#endif // end TAlphaEventSilArray.h
