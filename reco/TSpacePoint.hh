// SpacePoint class definition
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Nov 2014

#ifndef __TSPACEPOINT__
#define __TSPACEPOINT__ 1

#include "TObject.h"

class TSpacePoint: public TObject
{
private:
  int fw;
  int fp;
  double ft;
  double fH;

  double fx;
  double fy;
  double fz;
  double fr;
  double fphi;

  double ferrx;
  double ferry;
  double ferrz;
  double ferrr;
  double ferrphi;

public:
  TSpacePoint();

  TSpacePoint(int w, int p, double t,
	      double r, double phi,
	      double er,
	      double H=999999.);
  
  TSpacePoint(double x, double y, double z,
	      double ex, double ey, double ez);

  ~TSpacePoint() {};

  inline void SetPad(int p)      { fp=p; }
  inline void SetZ(double z)     { fz=z; }
  inline void SetErrZ(double ez) { ferrz=ez; }

  inline int GetWire() const {return fw;}
  inline int GetPad() const  {return fp;}

  inline double GetTime() const {return ft;}

  inline double GetHeight() const { return fH;}

  inline double GetX() const {return fx;}
  inline double GetY() const {return fy;}
  inline double GetZ() const {return fz;}

  inline double GetR() const   {return fr;}
  inline double GetPhi() const {return fphi;}

  inline double GetErrX() const {return ferrx;}
  inline double GetErrY() const {return ferry;}
  inline double GetErrZ() const {return ferrz;}

  inline double GetErrR()   const {return ferrr;}
  inline double GetErrPhi() const {return ferrphi;}

  double Distance(TSpacePoint*) const;
  double MeasureRad(TSpacePoint*) const;
  double MeasurePhi(TSpacePoint*) const;
  double MeasureZed(TSpacePoint*) const;
  double DistanceRphi(TSpacePoint*) const;

  static inline bool Order( TSpacePoint LHS, TSpacePoint RHS )
  {
    return LHS.fr > RHS.fr;
  }

  // static inline bool Order( TSpacePoint LHS, TSpacePoint RHS )
  // {
  //   return LHS.ft < RHS.ft;
  // }

  inline bool IsSortable() const { return true; }
  int Compare(const TObject*) const;

  bool IsGood(double&, double&) const;
  int Check(double&, double&) const;

  virtual void Print(Option_t *opt="xy") const;

ClassDef(TSpacePoint,1)
};

#endif
