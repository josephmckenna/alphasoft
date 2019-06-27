#ifndef __TAlphaEventCosmicHelix__
#define __TAlphaEventCosmicHelix__

#include "TObject.h"
#include "TObjArray.h"
#include "TAlphaEventHit.h"
#include "TAlphaEventTrack.h"
#include "TMath.h"
#include "TVector3.h"
#include "TMatrixD.h"

class TAlphaEventCosmicHelix: public TObject
{
 private:
  TObjArray fHits;
  TObjArray fPoints;

  Double_t fchi2;

  // First pass parameters
  Double_t fa;
  Double_t fb;
  Double_t fR;
  Double_t fth;
  Double_t fphi;
  Double_t flambda;

  // 'Canonical' representation
  Double_t fc;
  Double_t fd0;
  Double_t fphi0;
  Double_t fLambda;
  Double_t fz_0;

  Double_t fcovc[25];

  Double_t fx0;
  Double_t fy0;
  Double_t fz0;

  Double_t fX;
  Double_t fY;
  Double_t fZ;

  Double_t fPx;
  Double_t fPy;
  Double_t fPz;

  Int_t fParticle;

  Bool_t fIsGood;
  Bool_t fIsIncluded;

 public:
  TAlphaEventCosmicHelix();
  TAlphaEventCosmicHelix( TAlphaEventTrack * track );
  ~TAlphaEventCosmicHelix();

  void AddHit( TAlphaEventHit * cluster ) { fHits.AddLast( (TObject*)cluster ); }
  TAlphaEventHit* GetHit( Int_t i ) { return (TAlphaEventHit*)fHits.At( i ); }
  Int_t GetNHits() { return fHits.GetEntries(); }

  TVector3 GetPoint3D( Double_t t );
  TVector3 GetPoint3D_C( Double_t s );

  Double_t Geta() { return fa; }
  Double_t Getb() { return fb; }
  Double_t GetR() { return fR; }
  Double_t Getth() { return fth; }
  Double_t Getphi() { return fphi; }
  Double_t Getlambda() { return flambda; }

  Double_t GetChi2() { return fchi2; }

  Double_t Px() { return fPx; }
  Double_t Py() { return fPy; }
  Double_t Pz() { return fPz; }

  Int_t MakeCircle();
  Int_t MakeDipAngle();

  Bool_t IsGood() { return fIsGood; }
  void   SetIsGood( Bool_t good ) { fIsGood = good; }
  Bool_t IsIncluded() { return fIsIncluded; }
  void   SetIsIncluded( Bool_t good ) { fIsIncluded = good; }
  Double_t DistanceToFurthestHit();
  Double_t GetResiduals();

  void FindParticle();
  
  Int_t GetParticleID() { return fParticle; }

  void First_to_Canonical( Bool_t Invert = kFALSE );
 
  Double_t GetsFromR( Double_t R );
  Double_t GetsFromR_opposite( Double_t R );  

  Double_t Getfc() { return fc; }
  Double_t Getfd0() { return fd0; }
  Double_t Getfphi() { return fphi0; }
  Double_t Getflambda() { return fLambda; }
  Double_t Getfz0() { return fz0; }
  void Setfc( Double_t c ) { fc = c; }
  void Setfd0( Double_t d0 ) { fd0 = d0; }
  void Setfphi( Double_t phi ) { fphi = phi; }
  void Setflambda( Double_t lambda ) { fLambda = lambda; }
  void Setfz0( Double_t z0 ) { fz0 = z0; }

  // 5 * row + col 
  Double_t GetCovC( Int_t row, Int_t col ) { return fcovc[5*row+col]; }
  Double_t *GetCovC() { return fcovc; }

  Double_t GetSigma2c() { return GetCovC(0,0); }
  Double_t GetSigma2phi() { return GetCovC(1,1); }
  Double_t GetSigma2d0() { return GetCovC(2,2); }
  Double_t GetSigma2lambda() { return GetCovC(3,3); }
  Double_t GetSigma2z0c() { return GetCovC(4,4); }
  
  void LocalFit();
  void GetLocal( TAlphaEventHit * h, 
                 Double_t &p, 
                 Double_t &n, 
                 Double_t &s,
                 Double_t &x0b,
                 Double_t &y0b,
                 Double_t &u0b,
                 Double_t &v0b,
                 Double_t &ub,
                 Double_t &vb,
		 Int_t nHit);
  
  Double_t GetDpDc( TAlphaEventHit * h, Int_t nHit );
  Double_t GetDpDphi( TAlphaEventHit * h, Int_t nHit );
  Double_t GetDpDd0( TAlphaEventHit * h, Int_t nHit );
  Double_t GetDnDc( TAlphaEventHit * h, Int_t nHit );
  Double_t GetDnDphi( TAlphaEventHit * h, Int_t nHit );
  Double_t GetDnDd0( TAlphaEventHit * h, Int_t nHit );
  Double_t GetDnDlambda( TAlphaEventHit * h, Int_t nHit );

  Int_t GetNPoints() { return fPoints.GetEntries(); }
  TVector3 * GetPoint(Int_t i) { return (TVector3*) fPoints.At(i); }

  Double_t DCA( Double_t x, Double_t y, Double_t z);
  Double_t DCA( Double_t x, Double_t y, Double_t z,
		Double_t &xref, Double_t &yref, Double_t &zref );

  Double_t DCA_point( Double_t x, Double_t y, Double_t z);

  virtual void Print(const Option_t* = "") const;
  ClassDef(TAlphaEventCosmicHelix,1);
};


#endif
