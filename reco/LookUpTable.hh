// Look-up Table class definition
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Nov 2014

#ifndef __LOOKUPTABLE__
#define __LOOKUPTABLE__ 1
#include <vector>
#include <string>

#include "Math/Interpolator.h"

class LookUpTable
{
private:
  std::vector<double> frad;   // mm
  std::vector<double> fdrift; // ns
  std::vector<double> flor;   // rad

  ROOT::Math::Interpolator* finterpol_trad;
  ROOT::Math::Interpolator* finterpol_tphi;
  ROOT::Math::Interpolator* finterpol_tdrad;
  ROOT::Math::Interpolator* finterpol_tdphi;

  double fMinTime;
  double fMaxTime;


public:
  LookUpTable(int run);
  LookUpTable(double quencherFrac = 0.3, double B = 0.);
  ~LookUpTable();

  bool SetRun(int run);
  bool SetGas(double quencherFrac = 0.3, double B = 0);
  double GetRadius(double t);
  double GetAzimuth(double t);

  double GetdRdt(double t);
  double GetdPhidt(double t);

  double GetdR(double t);
  double GetdPhi(double t);

  inline double GetMaxTime() const {return fMaxTime;}

};

#endif
