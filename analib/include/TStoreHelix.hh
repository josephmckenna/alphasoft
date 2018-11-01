// Store helix class definition
// for ALPHA-g TPC AGTPCanalysis
// Stores essential information from TFitHelix
// Authors: A. Capra, M. Mathers
// Date: April 2017

#ifndef __TSTOREHELIX__
#define __TSTOREHELIX__ 1

#include "TObject.h"
#include "TObjArray.h"
#include "TVector3.h"

#include "TFitHelix.hh"

class TStoreHelix : public TObject
{
private:
  double fc;
  double fphi0;
  double fD;

  double flambda;
  double fz0;

  double fx0;
  double fy0;

  double ferr2c;
  double ferr2phi0;
  double ferr2D;

  double ferr2lambda;
  double ferr2z0;

  int fBranch;
  double fBeta;

  const TObjArray* fSpacePoints;
  int fNpoints;

  double fchi2R;
  double fchi2Z;
  int fStatus;

  TVector3 fMomentum;  // MeV/c
  TVector3 fMomentumError;
 
public:
  TStoreHelix();
  TStoreHelix(TFitHelix*, const TObjArray*);
  TStoreHelix(TFitHelix*);
  virtual ~TStoreHelix();  // destructor

  inline double GetC() const       {return fc;}
  inline void SetC(double c)       {fc=c;}
  inline double GetPhi0() const    {return fphi0;}
  inline void SetPhi0(double phi0) {fphi0=phi0;}
  inline double GetD() const       {return fD;}
  inline void SetD(double d)       {fD=d;}

  inline double GetLambda() const {return flambda;}
  inline void SetLambda(double l) {flambda=l;}
  inline double GetX0() const     {return fx0;}
  inline void SetX0(double x)     {fx0=x;}
  inline double GetY0() const     {return fy0;}
  inline void SetY0(double y)     {fy0=y;}
  inline double GetZ0() const     {return fz0;}
  inline void SetZ0(double z)     {fz0=z;}

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
  inline double GetZchi2() const {return fchi2Z;}

  inline int GetStatus() const { return fStatus; }
  inline int GetBranch()   const { return fBranch; }
  inline double GetFBeta() const { return fBeta; }

  inline TVector3 GetMomentumV() const      {return fMomentum;}// MeV/c
  inline TVector3 GetMomentumVerror() const {return fMomentumError;}

  inline const TObjArray* GetSpacePoints() const { return fSpacePoints; }
  inline void SetSpacePoints(const TObjArray* p) { fSpacePoints = p; }
  inline int GetNumberOfPoints() const { return fNpoints; }
  inline void SetNumberOfPoints(int np) { fNpoints = np; }

  virtual void Print(Option_t *option="") const;

  ClassDef(TStoreHelix,2)
};

#endif