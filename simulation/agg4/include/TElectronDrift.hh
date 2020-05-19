// TPC Electron Drift
// uses tabulated data from Garfield calculation 
// to determine for each primary ionization the 
// drift time and the Lorentz angle
//------------------------------------------------
// Author: A.Capra   Nov 2014
//------------------------------------------------

#ifndef __TELECTRONDRIFT__
#define __TELECTRONDRIFT__ 1

#include "TObject.h"
#include "Math/Interpolator.h"

class TElectronDrift: public TObject
{
private:
  ROOT::Math::Interpolator* finterpol_rtime;
  ROOT::Math::Interpolator* finterpol_rphi;

  double* fAnodeSignal;
  double* fPadSignal;

  static const int fTimeBins = 10000;

  double* fInductionPads;
  double* fInductionAnodes;

  static TElectronDrift* fElectronDrift;

public:
  TElectronDrift();
  ~TElectronDrift();

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

  //  double GetPadInduction(const double z, const double pos);

  static TElectronDrift* ElectronDriftInstance();

ClassDef(TElectronDrift,3)
};

#endif
