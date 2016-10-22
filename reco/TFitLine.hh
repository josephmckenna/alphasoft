// Straight Line class definition
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: July 2016

#ifndef __TFITLINE__
#define __TFITLINE__ 1

#include <TObject.h>
#include <TObjArray.h>
#include <TMath.h>
#include <TVector3.h>
#include <TPolyLine3D.h>

#include <vector>

extern double gTrapRadius;

class TDigi;
class TSpacePoint;
class TFitLine : public TObject
{
private:  
  TObjArray fPoints;
  int fNpoints;

  double fux;
  double fuy;
  double fuz;
  double fx0;
  double fy0;
  double fz0;

  double ferr2ux;
  double ferr2uy;
  double ferr2uz;
  double ferr2x0;
  double ferr2y0;
  double ferr2z0;

  static const int fNpar=2;
  double fchi2;
  int fStat;

  int fStatus;

  TPolyLine3D* fLine;

  int fParticle;

  std::vector<double> fResiduals;
  double fRes2;

  // parameters initialization
  void Initialization(double* Ipar);

public:
  TFitLine();
  ~TFitLine();  

  int AddPoint(TSpacePoint*);
  inline const TObjArray* GetPointsArray() const {return &fPoints;}
  inline int GetNumberOfPoints()           const {return fNpoints;}

  void Fit();

  TVector3 GetPosition(double t, 
		       double ux, double uy, double uz, 
		       double x0, double y0, double z0);
  TVector3 GetPosition(double t);
  TVector3 Evaluate(double r2, 
		    double ux, double uy, double uz, 
		    double x0, double y0, double z0);
  TVector3 Evaluate(double r2);
  

  double* GetU() const;
  double* Get0() const;

  inline double GetUx() const {return fux;}
  inline double GetUy() const {return fuy;}
  inline double GetUz() const {return fuz;}
  inline double GetX0() const {return fx0;}
  inline double GetY0() const {return fy0;}
  inline double GetZ0() const {return fz0;}

  inline double GetUxErr2() const {return ferr2ux;}
  inline double GetUyErr2() const {return ferr2uy;}
  inline double GetUzErr2() const {return ferr2uz;}
  inline double GetX0Err2() const {return ferr2x0;}
  inline double GetY0Err2() const {return ferr2y0;}
  inline double GetZ0Err2() const {return ferr2z0;}  

  inline int GetStat()    const { return fStat; }
  inline double GetChi2() const { return fchi2; }
  inline int GetDoF()     const { return fNpoints - fNpar; }

  inline int GetStatus() const { return fStatus;}

  double MinDistPoint(const TVector3*, TVector3*);
  double PointDistance2(double* par, double* point);

  double CalculateResiduals();
  std::vector<double> GetResidualsV() { return fResiduals; }
  double GetResiduals2() { return fRes2; }
  
  bool IsGood();

  virtual void Print(Option_t *option="") const;
  virtual void Draw(Option_t *option="");

  inline TPolyLine3D* GetLine() const {return fLine;}


ClassDef(TFitLine,1)
};

#endif
