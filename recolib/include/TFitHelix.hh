// Helix class definition
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: May 2014

#ifndef __TFITHELIX__
#define __TFITHELIX__ 1

#include <TObject.h>
#include <TObjArray.h>
#include <TMath.h>
#include <TVector3.h>
#include <TMinuit.h>

#include "TPCconstants.hh"
#include "TTrack.hh"

struct Vector2 {
  double X;
  double Y;
};


class TStoreHelix;

#define BETA 0
class TSpacePoint;
class TFitHelix : public TTrack
{
private:
  double fc;
  double fRc;
  double fphi0;
  double fD;

  double flambda;
  double fz0;

  double fa;
  double fx0;
  double fy0;

  double ferr2c;
  double ferr2Rc;
  double ferr2phi0;
  double ferr2D;

  double ferr2lambda;
  double ferr2z0;

  int fBranch;
  double fBeta;

  TVector3 fMomentum;  // MeV/c
  TVector3 fMomentumError;

  static const int fNpar=5;

  static const int fRNpar=3;
  double fchi2R;
  int fStatR;

  static const int fZNpar=2;
  double fchi2Z;
  int fStatZ;

  double fChi2RCut;
  double fChi2ZCut;
  double fChi2RMin;
  double fChi2ZMin;
  double fcCut;
  double fDCut;
  double fpCut;

  // parameters initialization
  void Initialization(double* Ipar);

  // utilities to calculate internal helix parameters
  double GetBeta      ( const double r2, const double Rc, const double D ) const;
  double GetBetaPlus  ( const double r2, const double Rc, const double D ) const;
  double GetBetaMinus ( const double r2, const double Rc, const double D ) const;

  double GetArcLength ( const double r2, const double Rc, const double D ) const;
  double GetArcLength_( const double r2, const double Rc, const double D ) const;

  double GetArcLengthPlus ( const double r2, const double Rc, const double D ) const;
  double GetArcLengthPlus_( const double r2, const double Rc, const double D ) const;
  double GetArcLengthMinus ( const double r2, const double Rc, const double D ) const;
  double GetArcLengthMinus_( const double r2, const double Rc, const double D ) const;

  // Multiple Scattering angle variance
  double VarMS() const;

  // select the successful radial fit with the smallest chi^2
  // among the 4 combiation of branch and  beta sign
  TMinuit* SelectBestFit();

public:
  TFitHelix();
  //  TFitHelix(double B=0);
  TFitHelix(const TTrack& atrack);
  TFitHelix(TObjArray*);
  TFitHelix( const TFitHelix& right );
  TFitHelix(TStoreHelix*);

  TFitHelix& operator=( const TFitHelix& right );
  
  ~TFitHelix();

  inline double GetC() const       {return fc;}
  inline void SetC(double c)       {fc=c;}
  inline double GetRc() const      {return fRc;}
  inline void SetRc(double rc)     {fRc=rc;}
  inline double GetPhi0() const    {return fphi0;}
  inline void SetPhi0(double phi0) {fphi0=phi0;}
  inline double GetD() const       {return fD;}
  inline void SetD(double d)       {fD=d;}

  inline double GetLambda() const {return flambda;}
  inline void SetLambda(double l) {flambda=l;}
  inline double GetZ0() const     {return fz0;}
  inline void SetZ0(double z)     {fz0=z;}

  inline double GetA() const       {return fa;}
  inline void SetA(double a)       {fa=a;}

  inline double GetErrC() const       {return ferr2c;}
  inline void SetErrC(double c)       {ferr2c=c;}
  inline double GetErrRc() const       {return ferr2Rc;}
  inline void SetErrRc(double rc)      {ferr2Rc=rc;}
  inline double GetErrPhi0() const    {return ferr2phi0;}
  inline void SetErrPhi0(double phi0) {ferr2phi0=phi0;}
  inline double GetErrD() const       {return ferr2D;}
  inline void SetErrD(double d)       {ferr2D=d;}

  inline double GetErrLambda() const {return ferr2lambda;}
  inline void SetErrLambda(double l) {ferr2lambda=l;}
  inline double GetErrZ0() const     {return ferr2z0;}
  inline void SetErrZ0(double z)     {ferr2z0=z;}

  inline double GetRchi2() const {return fchi2R;}
  inline void SetRchi2(double rchi2) {fchi2R = rchi2;}
  inline int GetRDoF() const     {return 2*GetNumberOfPoints() - fRNpar;}
  inline int GetStatR() const    {return fStatR;}

  inline double GetZchi2() const {return fchi2Z;}
  inline void SetZchi2(double zchi2) {fchi2Z = zchi2;}
  inline int GetZDoF() const     {return GetNumberOfPoints() - fZNpar;}
  inline int GetStatZ() const    {return fStatZ;}

  inline void SetXY0()
  {
    //#ifdef _GNU_SOURCE
    //  sincos(TMath::RadToDeg()*fphi0,&fx0,&fy0);
    //  fx0=-fD*fx0;
    //  fy0=fD*fy0;
    //#else
      fx0=-fD*TMath::Sin(fphi0); fy0=fD*TMath::Cos(fphi0);
    //#endif
  }
  inline double GetX0() const {return fx0;}
  inline void SetX0(double x) {fx0 = x;}
  inline double GetY0() const {return fy0;}
  inline void SetY0(double y) {fy0 = y;}

  inline void SetBranch(int br) { fBranch = br;}
  inline int GetBranch() const {return fBranch;}
  inline double GetFBeta() const { return fBeta;}

  inline void SetChi2RCut(double cut) {fChi2RCut=cut;}
  inline double GetChi2RCut() const   {return fChi2RCut;}
  inline void SetChi2ZCut(double cut) {fChi2ZCut=cut;}
  inline double GetChi2ZCut() const   {return fChi2ZCut;}
  inline void SetChi2RMin(double min) {fChi2RMin=min;}
  inline double GetChi2RMin() const   {return fChi2RMin;}
  inline void SetChi2ZMin(double min) {fChi2ZMin=min;}
  inline double GetChi2ZMin() const   {return fChi2ZMin;}

  inline void SetcCut(double cut) {fcCut=cut;}
  inline double GetcCut() const   {return fcCut;}
  inline void SetDCut(double cut) {fDCut=cut;}
  inline double GetDCut() const   {return fDCut;}

  inline void SetMomentumCut(double cut) {fpCut=cut;}
  inline double GetMomentumCut() const   {return fpCut;}

  inline TVector3 GetMomentumV() const      {return fMomentum;}// MeV/c
  inline TVector3 GetMomentumVerror() const {return fMomentumError;}

  inline void SetMomentumV(double px, double py, double pz) {fMomentum.SetX(px); fMomentum.SetY(py); fMomentum.SetZ(pz);}


  // LS fit to helix canonical form
  virtual void Fit();
  virtual void FitM2();
  void RadialFit(double* Ipar);
  void AxialFit(double* Ipar);

  // Evaluate the function for fitting
  Vector2 Evaluate ( const double r2, const double Rc, const double phi, const double D ) const; // +1 branch
  Vector2 Evaluate ( const double r2, const double Rc, const double u0, const double v0, const double D ) const; // +1 branch
  Vector2 Evaluate_( const double r2, const double Rc, const double phi, const double D )  const; // -1 branch
  Vector2 Evaluate_( const double r2, const double Rc, const double u0, const double v0, const double D ) const; // -1 branch

  // +1 branch, beta +ve root
  Vector2 EvaluatePlus ( const double r2, const double Rc, const double phi, const double D ) const; 
  Vector2 EvaluatePlus ( const double r2, const double Rc, const double u0, const double v0, const double D ) const; 
  // -1 branch, beta +ve root
  Vector2 EvaluatePlus_( const double r2, const double Rc, const double phi, const double D ) const;
  Vector2 EvaluatePlus_( const double r2, const double Rc, const double u0, const double v0, const double D ) const;
  // +1 branch, beta -ve root
  Vector2 EvaluateMinus ( const double r2, const double Rc, const double phi, const double D ) const;
  Vector2 EvaluateMinus ( const double r2, const double Rc, const double u0, const double v0, const double D ) const;
  // -1 branch, beta -ve root
  Vector2 EvaluateMinus_( const double r2, const double Rc, const double phi, const double D ) const;
  Vector2 EvaluateMinus_( const double r2, const double Rc, const double u0, const double v0, const double D ) const;

  double Evaluate   ( const double s,  const double l, const double z0 ) const;            // axial fit

  // unused but useful
  // +1 branch
  TVector3 Evaluate ( const double r2, const double Rc, const double phi, const double D, const double l, const double z0  ) const;
  // -1 branch 
  TVector3 Evaluate_( const double r2, const double Rc, const double phi, const double D, const double l, const double z0  ) const;
  // +1 branch, beta +ve root
  TVector3 EvaluatePlus ( const double r2, const double Rc, const double phi, const double D, const double l, const double z0  ) const;
  // -1 branch, beta +ve root
  TVector3 EvaluatePlus_( const double r2, const double Rc, const double phi, const double D, const double l, const double z0  ) const;
  // +1 branch, beta -ve root
  TVector3 EvaluateMinus ( const double r2, const double Rc, const double phi, const double D, const double l, const double z0  ) const;
  // -1 branch, beta -ve root
  TVector3 EvaluateMinus_( const double r2, const double Rc, const double phi, const double D, const double l, const double z0  ) const;

  // Radial arclength parameter for fitting/vertexing
  double GetArcLength ( const double r2 ) const;
  double GetArcLengthB( const double r2 ) const;

  // Evaluate the function for plotting
  TVector3 Evaluate( const double r2 ) const;
  // Evaluate errors for vertexing
  TVector3 EvaluateErrors2( const double r2 ) const;
  // Evaluate the function for plotting
  TVector3 EvaluateB( const double r2 ) const;
  // Evaluate errors for vertexing
  TVector3 EvaluateErrors2B( const double r2 ) const;

  // Evaluate function and errors for vertexing
  TVector3 GetPosition(const double s) const;
  TVector3 GetError2(const double s) const;

  double Momentum(); // returns pT in MeV/c

  virtual double GetApproxPathLength() const;
  void AddMSerror();

  int TubeIntersection(TVector3&, TVector3&, 
		       const double radius = ALPHAg::_trapradius) const;

  virtual double MinDistPoint(TVector3&);

  virtual bool IsGood();
  bool IsGoodChiSquare();
  void Reason();
  bool IsDuplicated(const TFitHelix*,const double) const;

  virtual void Print(Option_t *option="") const;
  virtual void Clear(Option_t *option="");

  // for sorting helix arrays from lowest c first to highest c last
  inline bool IsSortable() const { return true; }
  int Compare(const TObject*) const;

  ClassDef(TFitHelix,3)
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
