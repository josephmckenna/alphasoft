#ifndef __TAlphaEventTrack__
#define __TAlphaEventTrack__

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TAlphaEventTrack                                                     //
//                                                                      //
// Object describing TAlphaEvent silicon track                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TObject.h"
#include "TAlphaEventHit.h"
#include "TVector3.h"
#include "TPrincipal.h"
#include "TVectorD.h"

class TAlphaEventTrack : public TObject {
private:
  std::vector<TAlphaEventHit*> fHitArray;

  Double_t faxy; // LeastSquares Intercept (XY)
  Double_t fbxy; // LeastSquares Slope (XY)
  Double_t fr2xy;// LeastSquares Correlation Coefficient (XY)
  
  Double_t fayz; // LeastSquares Intercept (YZ)
  Double_t fbyz; // LeastSquares Slope (YZ)
  Double_t fr2yz;// LeastSquares Correlation Coefficient (YZ)
  
  Double_t fresxy; // XY absolute residuals
  Double_t fresyz; // YZ absolute residuals

  Double_t fdaxy; // Standard error for a
  Double_t fdbxy; // Standard error for b
	
  Double_t fdayz;
  Double_t fdbyz;
	
  TVector3 funitvector; // 3D unit vector parallel to the track
  TVector3 fr0;	// Point on the track line

  Double_t fcor;
  
  // used for cosmic rejection
  Double_t fRES;   // the residual
  Double_t fDCA;   // the minimum distance best 6-hits track to z-axis

public:
  TAlphaEventTrack();
  ~TAlphaEventTrack();

  void LeastSquares();
  void Residuals();
  void MakeLine();
  void MakeLinePCA();
  
  void Setunitvector(TVector3 v)      { funitvector=v; }
  void Setr0(TVector3 v)	      { fr0=v; }
  void Setcor(Double_t corr)          { fcor=corr;}
  //void SetDCAandRES(Double_t d, Double_t r) {fRES=r; fDCA=d;}
  void SetDCA(Double_t d) {fDCA=d;}
  void SetRES(Double_t r) {fRES=r;}
	  
  Double_t Getaxy()			{ return faxy; }
  Double_t Getbxy()			{ return fbxy; }
  Double_t Getr2xy()			{ return fr2xy; }
  Double_t Getayz()			{ return fayz; }
  Double_t Getbyz()			{ return fbyz; }
  Double_t Getr2yz()			{ return fr2yz; }
  Double_t Getresxy()			{ return fresxy; }
  Double_t Getresyz()			{ return fresyz; }
	
  Double_t Getdaxy()			{ return fdaxy; }
  Double_t Getdbxy()			{ return fdbxy; }
  Double_t Getdayz()			{ return fdayz; }
  Double_t Getdbyz()			{ return fdbyz; }

  Double_t Getcor()			{ return fcor; }
  
  Double_t GetRES()			{ return fRES; }
  Double_t GetDCA()			{ return fDCA; }

  Double_t fyx(Double_t x) { return faxy+fbxy*x;   }
  Double_t fxy(Double_t y) { if(fbxy) return (y-faxy)/fbxy;
                             else    return 0; }
  
  Double_t fyz(Double_t z) { return fayz+fbyz*z;   }
  Double_t fzy(Double_t y) { if(fbyz) return (y-fayz)/fbyz;
                             else    return 0; }

  TVector3 Getunitvector()	{ return funitvector; }
  TVector3 Getr0()			{ return fr0; }

  Int_t SortHits();

  Int_t GetNHits() { return fHitArray.size(); }
  void AddHit( TAlphaEventHit* cluster );
  TAlphaEventHit * GetHit( Int_t i ) { return fHitArray.at(i); }
  
  virtual void      Clear(Option_t * /*option*/ ="");
  
  // used for cosmic rejection
  Double_t CalculateTheResidual();
  Double_t DetermineDCA();
	
  ClassDef(TAlphaEventTrack,2);
};

#endif
