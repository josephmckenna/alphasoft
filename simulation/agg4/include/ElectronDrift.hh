// TPC Electron Drift
// uses tabulated data from Garfield calculation 
// to determine for each primary ionization the 
// drift time and the Lorentz angle
//------------------------------------------------
// Author: A.Capra   Nov 2014
//------------------------------------------------

#ifndef __ELECTRONDRIFT__
#define __ELECTRONDRIFT__ 1

#include "Math/Interpolator.h"

class ElectronDrift
{
private:
  ROOT::Math::Interpolator* finterpol_rtime;
  ROOT::Math::Interpolator* finterpol_rphi;

  double* fAnodeSignal;
  double* fPadSignal;

  static const int fTimeBins = 10000;

  double* fInductionPads;
  double* fInductionAnodes;

  static ElectronDrift* fElectronDrift;

public:
  ElectronDrift();
  ~ElectronDrift();

  double GetTime(double r);
  double GetAzimuth(double r);

  double GetAnodeSignal(int t) const;
  double GetPadSignal(int t) const;
  inline double* GetAnodeSignal() const {return fAnodeSignal;}
  inline double* GetPadSignal() const {return fPadSignal;}

  inline double GetAnodeInduction(int i) const 
  { if(i<3) return fInductionAnodes[i]; 
    else return 0.;
  }
  inline double GetPadInduction(int i) const 
  { if(i<6) return fInductionPads[i];
    else return 0.;
  }

  static ElectronDrift* ElectronDriftInstance();

};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
