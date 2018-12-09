// Store line class definition
// for ALPHA-g TPC AGTPCanalysis
// Stores essential information from TFitLine
// Authors: A. Capra
// Date: June 2017

#ifndef __TSTORELINE__
#define __TSTORELINE__ 1

#include "TObject.h"
#include "TObjArray.h"
#include "TVector3.h"

#include "TFitLine.hh"

class TStoreLine : public TObject
{
private:
  const TVector3 fDirection;
  const TVector3 fPoint;

  TVector3 fDirectionError;
  TVector3 fPointError;

  TObjArray fSpacePoints;
  int fNpoints;

  double fchi2;
  int fStatus;

  TVector3 fResidual;
  std::vector<double> fResiduals;
  double fResiduals2;

public:
  TStoreLine();
  TStoreLine(TFitLine*, const TObjArray*);
  TStoreLine(TFitLine*);
  virtual ~TStoreLine();  // destructor

  inline const TVector3* GetDirection() const { return &fDirection; }
  inline const TVector3* GetPoint() const { return &fPoint; }

  inline const TVector3* GetDirectionError() const { return &fDirectionError; }
  inline const TVector3* GePointError() const { return &fPointError; }

  inline const TObjArray* GetSpacePoints() const { return &fSpacePoints; }
  inline void SetSpacePoints(TObjArray* p) { fSpacePoints = *p; }
  inline int GetNumberOfPoints() const { return fNpoints; }
  inline void SetNumberOfPoints(int np) { fNpoints = np; }

  inline double GetChi2() const { return fchi2; }
  inline int GetStatus() const { return fStatus; }

  TVector3 GetResidual() const                   { return fResidual; }
  std::vector<double> GetResidualsVector() const { return fResiduals; }
  double GetResidualsSquared() const             { return fResiduals2; }
  void SetResidual(TVector3 r)                    { fResidual=r; }
  void SetResidualsVector(std::vector<double>& r) { fResiduals=r; }
  void SetResidualsSquared(double rq)             { fResiduals2=rq; }

  virtual void Print(Option_t *option="") const;

  ClassDef(TStoreLine,3)
};

#endif
