#ifndef __FITSIGNALS__
#define  __FITSIGNALS__ 1

#include "Minuit2/FCNBase.h"
#include <vector>
#include "SignalsType.h"

#include <math.h>

class GaussFunction 
{
public:
  
  GaussFunction(double constant, double mean, double sig) :
    theConstant(constant), theMean(mean), theSigma(sig) {}
  
  ~GaussFunction() {}
  
  double m() const {return theMean;}
  double s() const {return theSigma;}
  double c() const {return theConstant;}

  double operator()(double x) const 
  {
    return
      //      c()*exp(-0.5*(x-m())*(x-m())/(s()*s()))/(sqrt(2.*M_PI)*s());
      c()*exp(-0.5*(x-m())*(x-m())/(s()*s()));
  }

 private:
  double theConstant;
  double theMean;
  double theSigma;
};

class GaussFcn: public ROOT::Minuit2::FCNBase 
{
public:
  GaussFcn(std::vector<signal> &s):fSignals(s),theErrorDef(1.){}
  ~GaussFcn(){}
  
  virtual double operator()(const std::vector<double>& parameters) const;

  virtual double Up() const {return theErrorDef;}
  inline void SetErrorDef(double def) {theErrorDef = def;}

  inline const std::vector<signal>* GetData() const { return &fSignals; }

private:
  std::vector<signal> fSignals;
  double theErrorDef;
};

class fitSignals
{
 public:
  fitSignals(std::vector<signal>);
  ~fitSignals() {}

  inline void SetStart(double* s) { for(uint i=0; i<fNpar; ++i) fStart[i]=s[i]; }
  inline const std::vector<double> GetStart() const { return fStart; }
  
  inline void SetStep(double* s) { for(uint i=0; i<fNpar; ++i) fStep[i]=s[i]; }
  inline const std::vector<double> GetStep() const { return fStep; }  

  void Fit();

  void CalculateDoF();

  inline double GetChi2() const { return fchi2; }
  inline int GetStat()    const { return fStat; }
  inline int GetDoF()     const { return fDoF; }

  inline double GetAmplitude() const      { return fAmplitude; }
  inline double GetMean() const           { return fMean; }
  inline double GetSigma() const          { return fSigma; }
  inline double GetAmplitudeError() const { return fAmplitudeError; }
  inline double GetMeanError() const      { return fMeanError; }
  inline double GetSigmaError() const     { return fSigmaError;}

  inline void SetPrintLevel(int l) { print_level = l; }


private:
  GaussFcn theFCN;

  uint fNpar;
  
  std::vector<double> fStep;
  std::vector<double> fStart;
  int print_level;
  
  int fStat;
  int fDoF;
  double fchi2;

  double fAmplitude;
  double fMean;
  double fSigma;
  double fAmplitudeError;
  double fMeanError;
  double fSigmaError;
};
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
