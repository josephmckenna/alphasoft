// Look-up Table class definition
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Nov 2014

#ifndef __LOOKUPTABLE__
#define __LOOKUPTABLE__ 1
#include <vector>
#include <string>
#include <map>

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

  double fMapBegin;
  std::vector<ROOT::Math::Interpolator*> finterpol_trad_zed;
  std::vector<ROOT::Math::Interpolator*> finterpol_tphi_zed;
  std::vector<std::vector<double>> frad_zed;   // mm
  std::vector<std::vector<double>> fdrift_zed; // ns
  std::vector<std::vector<double>> flor_zed;   // rad
  std::map<double,unsigned> fZed;   // mm

  double fMinTime;
  double fMaxTime;

  unsigned FindGridPoint(const double z);

public:
  LookUpTable(int run);
  LookUpTable(double quencherFrac, double B);
  LookUpTable(double quencherFrac);
  ~LookUpTable();

  bool SetRun(int run);
  bool SetGas(double quencherFrac, double B);
  bool SetGas(double quencherFrac);
  bool SetDefault();

  double GetRadius(const double t);
  double GetAzimuth(const double t);

  double GetdRdt(const double t);
  double GetdPhidt(const double t);

  double GetRadius(const double t, const double z);
  double GetAzimuth(const double t, const double z);

  double GetdRdt(const double t, const double z);
  double GetdPhidt(const double t, const double z);

  double GetdR(const double t);
  double GetdPhi(const double t);

  inline double GetMaxTime() const {return fMaxTime;}

  inline const std::map<double,unsigned>* GetZpositions() const { return &fZed; }
  inline double GetMapBegin() const {return fMapBegin;}
};

#endif
