#ifndef __TAlphaEventHelix__
#define __TAlphaEventHelix__

#include "TObject.h"
#include "TObjArray.h"
#include "TAlphaEventTrack.h"
#include "TAlphaEventHit.h"
#include "TMath.h"
#include "TVector3.h"
#include "TMatrixD.h"

class TAlphaEventHelix: public TObject
{
 private:
  TObjArray       fHits;

  Int_t           fParticleID;
  Double_t        fChi2;

  Int_t           fCircleStatus;
  Int_t           fLineStatus;
  Int_t           fHelixStatus;

  // First pass parameters
  Double_t        fa;
  Double_t        fb;
  Double_t        fR;
  Double_t        fth;
  Double_t        fphi;
  Double_t        flambda;

  // 'Canonical' representation
  Double_t        fc;
  Double_t        fd0;
  Double_t        fphi0;
  Double_t        fLambda;
  // Reference point
  Double_t        fx0;
  Double_t        fy0;
  Double_t        fz0;

  Double_t        fd0_trap;

 public:
  TAlphaEventHelix();
  TAlphaEventHelix( TAlphaEventTrack * track );
  ~TAlphaEventHelix();

  void            AddHit( TAlphaEventHit * cluster );
  Int_t           DetermineCircleParameters();
  Int_t           DetermineLineParameters();
  void            DetermineSagitta();
  Int_t           FitLineParameters();
  void            First_to_Canonical( Bool_t Invert = kFALSE );

  Double_t        Geta() { return fa; }
  Double_t        Getb() { return fb; }
  Double_t        GetChi2() { return fChi2; }
  Int_t           GetCircleStatus() { return fCircleStatus; }
  Int_t           GetHelixStatus() { return fHelixStatus; }
  TAlphaEventHit* GetHit( Int_t i ) { return (TAlphaEventHit*)fHits.At( i ); }
  Double_t        Getfc() { return fc; }
  Double_t        Getfd0() { return fd0; }
  Double_t        Getfd0_trap() { return fd0_trap; }
  Double_t        Getfphi() { return fphi0; }
  Double_t        Getflambda() { return fLambda; }
  Double_t        Getfz0() { return fz0; }
  Int_t           GetLineStatus() { return fLineStatus; }
  Int_t           GetParticleID() { return fParticleID; }
  TVector3        GetPoint3D( Double_t t );
  TVector3        GetPoint3D_C( Double_t s );
  Int_t           GetNHits() { return fHits.GetEntriesFast(); }
  Double_t        GetR() { return fR; }
  Double_t        Getth() { return fth; }
  Double_t        Getphi() { return fphi; }
  Double_t        Getlambda() { return flambda; }
  Double_t        GetsFromR( Double_t R, Int_t &iflag );
  Double_t        GetsFromR_opposite( Double_t R );  
  Double_t        GetTotalHitSignificance(){
  
    Double_t sig=0;
  for (int iHit =0; iHit <fHits.GetEntries(); iHit ++)
  {
   sig+=((TAlphaEventHit*)fHits.At( iHit ))->GetHitSignifance();
  }
  return sig;
}
  
  void            Setfc( Double_t c ) { fc = c; }
  void            Setfd0( Double_t d0 ) { fd0 = d0; }
  void            Setfd0_trap( Double_t d0 ) { fd0_trap = d0; }
  void            Setfphi( Double_t phi ) { fphi0 = phi; }
  void            Setflambda( Double_t lambda ) { fLambda = lambda; }
  void            Setfz0( Double_t z0 ) { fz0 = z0; }
  void            SetHelixStatus( Int_t status ) { fHelixStatus = status; }
  Int_t           SortHits();
  
  virtual void    Print(const Option_t* = "") const;
  ClassDef(TAlphaEventHelix,1);
};


#endif
