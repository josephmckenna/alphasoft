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
#include <TPolyLine3D.h>

extern double gTrapRadius;

class TDigi;
class TSpacePoint;
class TFitHelix : public TObject
{
private:
  TObjArray fDigi;
  int fNdigi;

  TObjArray fPoints;
  int fNpoints;

  double fc;
  double fphi0;
  double fD;

  double flambda;
  double fz0;

  double fa;
  double fx0;
  double fy0;

  double ferr2c;  
  double ferr2phi0;
  double ferr2D;

  double ferr2lambda;
  double ferr2z0;

  int fBranch;

  TVector3 fMomentum;  // MeV/c
  TVector3 fMomentumError;

  int fParticle;

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

  TVector3 fResiduals;

  TPolyLine3D* fHelix;

  int fStatus;

  // used in tfitvertex, talphafitvertex
  double fitArc;

  // parameters initialization
  void Initialization(double* Ipar);

  // utilities to calculate internal helix parameters
  double GetBeta      ( double r2, double c, double D );
  double GetArcLength ( double r2, double c, double D );
  double GetArcLength_( double r2, double c, double D );

  // Multiple Scattering angle variance
  double VarMS();

  TVector3* faPoint;

public:
  TFitHelix();
  ~TFitHelix();

  int AddDigi(TDigi*);
  inline const TObjArray* GetDigiArray() const {return &fDigi;}
  inline int GetNumberOfDigi()     const {return fNdigi;}

  int AddPoint(TSpacePoint*);
  inline const TObjArray* GetPointsArray() const {return &fPoints;}
  inline int GetNumberOfPoints()     const {return fNpoints;}
  
  inline double GetC() const       {return fc;}
  inline void SetC(double c)       {fc=c;}
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
  inline double GetErrPhi0() const    {return ferr2phi0;}
  inline void SetErrPhi0(double phi0) {ferr2phi0=phi0;}
  inline double GetErrD() const       {return ferr2D;}
  inline void SetErrD(double d)       {ferr2D=d;}

  inline double GetErrLambda() const {return ferr2lambda;}
  inline void SetErrLambda(double l) {ferr2lambda=l;}
  inline double GetErrZ0() const     {return ferr2z0;}
  inline void SetErrZ0(double z)     {ferr2z0=z;}

  inline double GetRchi2() const {return fchi2R;}
  inline int GetRDoF() const     {return fNpoints - fRNpar;}
  inline int GetStatR() const    {return fStatR;}

  inline double GetZchi2() const {return fchi2Z;}
  inline int GetZDoF() const     {return fNpoints - fZNpar;}
  inline int GetStatZ() const    {return fStatZ;}

  inline void SetXY0() { fx0=-fD*TMath::Sin(fphi0); fy0=fD*TMath::Cos(fphi0); }
  inline double GetX0() const {return fx0;}
  inline double GetY0() const {return fy0;}

  inline void SetBranch(int br) { fBranch = br;}
  inline int GetBranch() const {return fBranch;}

  inline void SetParticleType(int pdg) {fParticle=pdg;}
  inline int GetParticleType() const   {return fParticle;}

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

  inline int GetStatus() const {return fStatus;}
  inline void SetStatus(int s) {fStatus=s;}

  inline double GetFitArc() const {return fitArc;}
  inline void SetFitArc(double s) {fitArc = s;}

  inline TVector3 GetMomentumV() const      {return fMomentum;}// MeV/c
  inline TVector3 GetMomentumVerror() const {return fMomentumError;}

  //  double GetPathLength(double rmin=gTrapRadius);
  double GetApproxPathLength();

  inline TVector3 GetResiduals3D() const {return fResiduals;}

  // LS fit to helix canonical form
  void Fit();
  
  // Evaluate the function for fitting
  TVector2 Evaluate ( double r2, double c, double phi, double D ); // +1 branch
  TVector2 Evaluate_( double r2, double c, double phi, double D ); // -1 branch
  double Evaluate   ( double s,  double l, double z0 );            // axial fit
  TVector3 Evaluate ( double r2, double c, double phi, double D, double l, double z0  ); // +1 branch
  TVector3 Evaluate_( double r2, double c, double phi, double D, double l, double z0  ); // -1 branch

  // Radial arclength parameter for fitting/vertexing
  double GetArcLength ( double r2 );

  // Evaluate the function for plotting
  TVector3 Evaluate( double r2 );
  // Evaluate errors for vertexing
  TVector3 EvaluateErrors2( double r2 );
  // Evaluate function and errors for vertexing
  TVector3 GetPosition(double s);
  TVector3 GetError2(double s);

  double Momentum(); // returns pT in MeV/c
  void AddMSerror();

  double CalculateResiduals();
  int TubeIntersection(TVector3&, TVector3&, double radius=gTrapRadius);

  inline void SetPoint(TVector3* p) { faPoint = p;}
  inline TVector3* GetPoint() { return faPoint; }
  double MinDistPoint(const TVector3*, TVector3*);

  bool IsGood();
  bool IsDuplicated(TFitHelix*,double);
  
  virtual void Print(Option_t *option="") const;
  virtual void Draw(Option_t *option="");

  inline TPolyLine3D* GetHelix() const {return fHelix;}

  // for sorting helix arrays from lowest c first to highest c last
  inline bool IsSortable() const { return true; }
  int Compare(const TObject*) const;

  ClassDef(TFitHelix,1)
};

#endif
