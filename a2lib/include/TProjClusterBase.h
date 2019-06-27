
#ifndef _TProjClusterBase_
#define _TProjClusterBase_

#include <TObject.h>
#include <TVector3.h>

class TProjClusterBase : public TObject
{
private:
  Double_t fRPhi;
  Double_t fZ;
  Int_t    fTrack;
  Double_t fAngleFromNormal;
  Double_t fRPhiSigma;
  Double_t fZSigma;
  Double_t fLambda;
  Double_t fd0;
  Int_t fN;

  Bool_t   fMCClosest;

public:
  TProjClusterBase();
  TProjClusterBase(TProjClusterBase *c);
  TProjClusterBase(Double_t RPhi, Double_t Z); 
  TProjClusterBase(Double_t RPhi, Double_t Z, 
		   Double_t AngleFromNormal, Int_t Track);
  ~TProjClusterBase();

  Double_t RPhi() { return fRPhi; }
  Double_t Z() { return fZ; }
  Double_t GetRPhiSigma() { return fRPhiSigma; }
  Double_t GetZSigma() { return fZSigma; }
  Double_t GetAngleFromNormal() { return fAngleFromNormal; }
  Double_t GetLambda() { return fLambda; }
  Double_t Getd0() { return fd0; }
  Int_t    GetN() { return fN; }
  Int_t    GetTrack() { return fTrack; }
  Bool_t   GetMCClosest() { return fMCClosest; }
  void     SetRPhi( Double_t RPhi ) { fRPhi = RPhi; }
  void     SetZ( Double_t Z ) { fZ = Z; }
  void     SetRPhiSigma( Double_t RPhiSigma ) { fRPhiSigma = RPhiSigma; }
  void     SetZSigma( Double_t ZSigma ) { fZSigma = ZSigma; }
  void     SetTrack( Int_t Track ) { fTrack = Track; }
  void     SetN( Int_t N ) { fN = N; }
  void     SetAngleFromNormal( Double_t Angle ) { fAngleFromNormal = Angle; }
  void     SetAngle(TVector3 *vl, TVector3 *vr);
  void     SetLambda(Double_t Lambda) { fLambda = Lambda; }
  void     Setd0(Double_t d0) { fd0 = d0; }
  void     SetMCClosest(Bool_t closest) { fMCClosest = closest; }
  void     SetSigmas();
  void 	   SetSigmas(Bool_t AngleFromNormal);

  virtual void  Clear(Option_t */* option*/ ="");
  virtual void Print( Option_t *opt ="") const;
  ClassDef( TProjClusterBase, 1 );
};

#endif
