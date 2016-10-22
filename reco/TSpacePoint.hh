// SpacePoint class definition
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Nov 2014

#ifndef __TSPACEPOINT__
#define __TSPACEPOINT__ 1

#include "TObject.h"

class TDigi;
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

  int fMCid;
  int fPDG;

public:
  TSpacePoint();
  TSpacePoint(TDigi*);
  // wire, pad, time, phi, z
  TSpacePoint(int, int, double, double, double);
  // wire, pad, G4 id, PD Gcode, time, phi, z
  TSpacePoint(int, int, int, int, double, double, double);
  // wire, pad, time, x, y, z, error x, error y, error z, pulse height
  TSpacePoint(int, int, double,
	      double, double, double,
	      double, double, double,
	      double H=999999.);
  // time, x, y, z, error x, error y, error z, pulse height
  TSpacePoint(double,
	      double, double, double,
	      double, double, double,
	      double H=999999.);
  // x, y, z, error x, error y, error z, pulse height
  TSpacePoint(double, double, double,
	      double, double, double,
	      double H=999999.);
  ~TSpacePoint() {};

  inline int GetWire() {return fw;}
  inline int GetPad() {return fp;}

  inline double GetTime() {return ft;}

  inline double GetHeight() { return fH;}

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

  inline int GetMCid() const {return fMCid;}
  inline int GetPDG() const  {return fPDG;}

  double Distance(TSpacePoint*);
  double MeasureRad(TSpacePoint*);
  double MeasurePhi(TSpacePoint*);
  double MeasureZed(TSpacePoint*);
  
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

ClassDef(TSpacePoint,1)
};

#endif
