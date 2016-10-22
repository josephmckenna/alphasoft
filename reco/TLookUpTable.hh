// Look-up Table class definition
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Nov 2014

#ifndef __TLOOKUPTABLE__
#define __TLOOKUPTABLE__ 1
#include <vector>
#include <string>

#include "TObject.h"
#include "TMath.h"
#include "Math/Interpolator.h"

class TLookUpTable: public TObject
{
private:
  ROOT::Math::Interpolator* finterpol_trad;
  ROOT::Math::Interpolator* finterpol_tphi;
  ROOT::Math::Interpolator* finterpol_tdrad = nullptr;
  ROOT::Math::Interpolator* finterpol_tdphi = nullptr;

  double fMaxTime, fMinTime;

  std::string gasdir;

  static TLookUpTable* fLookUpTable;
  static std::vector<double> get_a_b_r0(double B);
  static constexpr double deltat = 16.; // 16.404 for B = 1T, 15.99 for B = 0, but enough variation to make those the same.

public:
  TLookUpTable();
  TLookUpTable(const std::string &gas, double quencherFrac = 0.1, double B = 0);
  ~TLookUpTable();

  bool SetGas(const std::string &gas, double quencherFrac = 0.1, double B = 0);
  bool SetB(double B);
  double GetRadius(double t);
  double GetAzimuth(double t);

  double GetdRdt(double t);
  double GetdPhidt(double t);

  double GetdR(double t);
  double GetdPhi(double t);

  inline double GetMaxTime() const {return fMaxTime;}

  static double r_of_t(double t, double B = 1);
  static double dr_of_t(double t, double B = 1);
  static double phi_of_r(double r, double B = 1);

  static TLookUpTable* LookUpTableInstance();

ClassDef(TLookUpTable,1)
};

#endif
