#ifndef __TAlphaEventVertex__
#define __TAlphaEventVertex__

#include "TMath.h"
#include "TObject.h"
#include "TObjArray.h"
#include "TVector3.h"

#include "TAlphaEventHelix.h"

class TAlphaEventVertex : public TObject
{
 private:

  Bool_t    fIsGood;
  std::vector<TAlphaEventHelix*> fHelices;
  std::vector<TVector3*> fDCAs;
  std::vector<TVector3*> fDCAa;
  std::vector<TVector3*> fDCAb;
  Double_t  fDCA;

  Int_t     fhi;
  Int_t     fhj;

  // vertex values
  Double_t  fX;
  Double_t  fY;
  Double_t  fZ;
  
 public:
  TAlphaEventVertex();
  ~TAlphaEventVertex();

  void              AddHelix( TAlphaEventHelix * helix )  { fHelices.push_back( helix ); }
  void              AddDCA( TVector3 * dca )  { fDCAs.push_back( dca ); }
  void              AddDCAa( TVector3 * dca )  { fDCAa.push_back( dca ); }
  void              AddDCAb( TVector3 * dca )  { fDCAb.push_back( dca ); }
  Double_t          CalculateVertexMeanDCA( Double_t vx,
					    Double_t vy,
					    Double_t vz );
  TVector3         *FindDCA( TAlphaEventHelix * ha, TAlphaEventHelix * hb);
  TVector3         *FindDCAToVertex( TAlphaEventHelix *helix);
  Int_t             Getfhi() { return fhi; }
  Int_t             Getfhj() { return fhj; }
  int             GetNHelices() { return fHelices.size(); }
  TAlphaEventHelix *GetHelix( int i ) { return fHelices.at( i ); }
  int             GetNDCAs() { return fDCAs.size(); }
  int             GetNDCAa() { return fDCAa.size(); }
  TVector3         *GetDCA( Int_t i )  { return fDCAs.at( i ); }
  TVector3         *GetDCAa( Int_t i ) { return fDCAa.at( i ); }
  TVector3         *GetDCAb( Int_t i ) { return fDCAb.at( i ); }
  Double_t          GetDCA() { return fDCA; }
  Bool_t            IsGood() { return fIsGood; }
  Double_t          MinimizeVertexMeanDCA();
  void              RecVertex(); 
  void              SetXYZ( Double_t x, Double_t y, Double_t z ) { fX = x; fY = y; fZ = z; }
  Double_t          Phi() { return fX == 0.0 && fY == 0.0 ? 0.0 : TMath::ATan2(fY,fX); }
  void              Setfhi( Int_t hi ) { fhi = hi; }
  void              SetDCA( Double_t dca ) { fDCA = dca; }
  void              SetIsGood(Bool_t isgood)  { fIsGood = isgood; }
  Double_t          X() { return fX; }
  Double_t          Y() { return fY; }
  Double_t          Z() { return fZ; }

  virtual void      Clear(Option_t * /*option*/ ="");
  virtual void      Print(const Option_t* = "") const;
  ClassDef(TAlphaEventVertex,1);
};

#endif
