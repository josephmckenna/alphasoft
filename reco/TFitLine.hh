// Straight Line class definition
// for ALPHA-g TPC analysis
// Author: A.Capra 
// Date: July 2016

#ifndef __TFITLINE__
#define __TFITLINE__ 1

#include "TObject.h"
#include "TObjArray.h"
#include "TMath.h"
#include "TVector3.h"
#include "TPolyLine3D.h"

#include <vector>

#include "TTrack.hh"

class TFitLine : public TTrack
{
private:  
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

  double fChi2Min;
  double fChi2Cut;

  // parameters initialization
  void Initialization(double* Ipar);

public:
  TFitLine();
  TFitLine(TObjArray*);
  TFitLine(const TTrack&);
  ~TFitLine();  

  void Fit();

  TVector3 GetPosition(double t, 
		       double ux, double uy, double uz, 
		       double x0, double y0, double z0);
  TVector3 GetPosition(double t);
  TVector3 GetError2(double ) { TVector3 v(0.,0.,0.); return v; }
  TVector3 Evaluate(double r2, 
		    double ux, double uy, double uz, 
		    double x0, double y0, double z0);
  TVector3 Evaluate(double r2);
  TVector3 EvaluateErrors2(double ) { TVector3 v(0.,0.,0.); return v; }

  double GetParameter( double r2,
		       double ux, double uy, double uz, 
		       double x0, double y0, double z0);
  double GetParameter( double r2 );

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

  inline void SetChi2Cut(double cut) {fChi2Cut=cut;}
  inline double GetChi2Cut() const   {return fChi2Cut;}    
  inline void SetChi2Min(double min) {fChi2Min=min;}
  inline double GetChi2Min() const   {return fChi2Min;}

  virtual double MinDistPoint(TVector3&);
  double PointDistance2(double* par, double* point);
  virtual double MinRad();
  virtual double MinRad2();

  virtual bool IsGood();
  virtual void Reason();

  double Angle( TFitLine* );
  double CosAngle( TFitLine* );

  TVector3 Sagitta( TFitLine* );
  double Distance( TFitLine* ); 

  virtual void Print(Option_t *option="") const;
  virtual void Draw(Option_t *option="");

  ClassDef(TFitLine,1)
};

#endif
