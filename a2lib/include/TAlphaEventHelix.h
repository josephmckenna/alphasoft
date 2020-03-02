#ifndef __TAlphaEventHelix__
#define __TAlphaEventHelix__

#include "TObject.h"
#include "TAlphaEventTrack.h"
#include "TAlphaEventHit.h"
#include "TMath.h"
#include "TVector3.h"
#include "TMatrixD.h"


#include "Minuit2/FCNBase.h"
//#include <Minuit2/Minuit2Minimizer.h>
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MnUserParameterState.h"
#include "Minuit2/MnPrint.h"
#include "Minuit2/MnMigrad.h"
#include "Minuit2/MnMinos.h"
#include "Minuit2/MnContours.h"
#include "Minuit2/MnPlot.h"
#include "Minuit2/MinosError.h"
#include "Minuit2/ContoursError.h"
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MnUserParameterState.h"
#include "Minuit2/MnPrint.h"
#include "Minuit2/MnMigrad.h"
#include "Minuit2/MnMinos.h"
#include "Minuit2/MnContours.h"
#include "Minuit2/MnPlot.h"
#include "Minuit2/MinosError.h"
#include "Minuit2/ContoursError.h"
#include "TAlphaEventHelix.h"

class TAlphaEventHelix: public TObject
{
 private:
  std::vector<TAlphaEventHit*>  fHits;

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
  void            FitHelix();
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
  TAlphaEventHit* GetHit( Int_t i ) { return fHits.at( i ); }
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
  Int_t           GetNHits() { return fHits.size(); }
  Double_t        GetR() { return fR; }
  Double_t        Getth() { return fth; }
  Double_t        Getphi() { return fphi; }
  Double_t        Getlambda() { return flambda; }
  Double_t        GetsFromR( Double_t R, Int_t &iflag );
  Double_t        GetsFromR_opposite( Double_t R );  
  Double_t        GetTotalHitSignificance()
  {
     Double_t sig=0;
     int size=GetNHits();
     for (int iHit =0; iHit <size; iHit ++)
     {
        sig+=((TAlphaEventHit*)fHits.at( iHit ))->GetHitSignifance();
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


class minuit2Helix: public ROOT::Minuit2::FCNBase
{

public:
  minuit2Helix(TAlphaEventHelix * h)
  {
    helix=h;
  }

  double operator() (const std::vector<double> & par) const
  {
    Double_t chi2 = 0;
    Double_t z0     = par[0];
    Double_t Lambda = par[1];

    for( Int_t ihit = 0; ihit < helix->GetNHits(); ihit++ )
    {
      TAlphaEventHit * hit = helix->GetHit( ihit );

      Double_t x = hit->XMRS();
      Double_t y = hit->YMRS();
      Double_t z = hit->ZMRS();

      Int_t iflag=0;
      Double_t s = helix->GetsFromR( TMath::Sqrt( x*x + y*y ), iflag );

      Double_t zprime = z0 + Lambda*s;
      chi2 += (z-zprime)*(z-zprime)/hit->GetNSigma2();
    }
    return chi2;
  }
  double Up() const { return 1.; }

private:
  TAlphaEventHelix* helix;
};
#endif
