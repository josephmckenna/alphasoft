#ifndef __TAlphaEventVertex__
#define __TAlphaEventVertex__

#include "TMath.h"
#include "TObject.h"
#include "TObjArray.h"
#include "TVector3.h"

#include "Minuit2/FCNBase.h"
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
 TVector3         *FindDCAToVertex( TAlphaEventHelix *helix, double, double, double);
class minuit2DCA : public ROOT::Minuit2::FCNBase 
{

public: 

  minuit2DCA(TAlphaEventHelix *i, TAlphaEventHelix *j )
  {
     hi=i;
     hj=j;
  }

  double operator() (const std::vector<double> & par) const
  {
    // Rosebrock function
    TVector3 vhi = hi->GetPoint3D_C(par[0]);
    TVector3 vhj = hj->GetPoint3D_C(par[1]);

    double d = (vhi.X()-vhj.X())*(vhi.X()-vhj.X())
      + (vhi.Y()-vhj.Y())*(vhi.Y()-vhj.Y())
      + (vhi.Z()-vhj.Z())*(vhi.Z()-vhj.Z());

    return d;
  } 
  
  double Up() const { return 1.; }
  
private: 
  TAlphaEventHelix *hi;
  TAlphaEventHelix *hj;
};

class minuit2MeanDCA: public ROOT::Minuit2::FCNBase
{

public:
  minuit2MeanDCA(std::vector<TAlphaEventHelix*>* h)
  {
    helices=h;

  }

  double operator() (const std::vector<double> & par) const
  {
    Double_t MeanDCA = 0;
    int n=helices->size();
    for(int hi=0; hi<n; hi++)
    {
      TAlphaEventHelix *helix = helices->at(hi);
//      fhi = hi;
      TVector3 *dca = FindDCAToVertex( helix,par[0],par[1],par[2] );
      Double_t d = TMath::Sqrt( (dca->X()-par[0])*(dca->X()-par[0]) +
                                (dca->Y()-par[1])*(dca->Y()-par[1]) +
                                (dca->Z()-par[2])*(dca->Z()-par[2]));
      MeanDCA+=d;
      delete dca;
    }
    MeanDCA /= n;
    return MeanDCA;
  }

  double Up() const { return 1.; }

private:
  std::vector<TAlphaEventHelix*>* helices;
};

class minuit2DCAToVertex: public ROOT::Minuit2::FCNBase
{

public:
  minuit2DCAToVertex(TAlphaEventHelix* i, double _x, double _y, double _z)
  {
    hi=i;
    x=_x;
    y=_y;
    z=_z;
  }

  double operator() (const std::vector<double> & par) const
  {
    TVector3 vhi = hi->GetPoint3D_C(par[0]);

    double d2 = (vhi.X()-x)*(vhi.X()-x) +
                (vhi.Y()-y)*(vhi.Y()-y) +
                (vhi.Z()-z)*(vhi.Z()-z);
    return d2;
  }

  double Up() const { return 1.; }

private:
  TAlphaEventHelix *hi;
    double x,y,z;
};
#endif
