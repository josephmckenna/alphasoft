#ifndef __FITSIGNALS__
#define  __FITSIGNALS__ 1

#include "Minuit2/FCNBase.h"
#include <vector>
#include <cmath>
#include <cassert>

#include "SignalsType.h"

class GaussFunction 
{
public:
  GaussFunction(double constant, double mean, double sig) :
    theConstant(constant), theMean(mean), theSigma(sig) {}

  GaussFunction(const std::vector<double> p) :
    theConstant(p[0]), theMean(p[1]), theSigma(p[2]) {}
  
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

class MultiGaussFunction 
{
public:
  
  MultiGaussFunction(const std::vector<double> constant, 
		     const std::vector<double> mean, 
		     const std::vector<double> sig)
  {
    Ngauss = int(constant.size());
    for( int i=0; i<Ngauss; ++i )
      Gfunc.emplace_back( constant.at(i), mean.at(i), sig.at(i) );
  }

  MultiGaussFunction(const std::vector<double> parameters)
  {
    assert(parameters.size()%3==0);
    Ngauss = parameters.size()/3;
    for( int i=0; i<Ngauss; ++i )
      {
	auto it = parameters.begin()+3*i;
	const std::vector<double> pars(it,it+3);
	Gfunc.emplace_back(pars);
      }
  }
  
  ~MultiGaussFunction() {}
  
  // double c(int i=0) const {return theConstant[i];}
  // double m(int i=0) const {return theMean[i];}
  // double s(int i=0) const {return theSigma[i];}
  double c(int i=0) const {return Gfunc[i].c();}
  double m(int i=0) const {return Gfunc[i].m();}
  double s(int i=0) const {return Gfunc[i].s();}

  double operator()(double x) const 
  {
    double sum=0;
    for( int i=0; i<Ngauss; ++i )
      sum+=Gfunc[i](x);
    return sum;
  }

private:
  int Ngauss;
  // std::vector<double> theConstant;
  // std::vector<double> theMean;
  // std::vector<double> theSigma;
  std::vector<GaussFunction> Gfunc;
};

class GaussFcn: public ROOT::Minuit2::FCNBase 
{
public:
  GaussFcn(std::vector<signal> &s):fSignals(s),theErrorDef(1.)
   {
      //      std::cout<<"GaussFcn signals size: "<<fSignals.size()<<std::endl;
   }
  ~GaussFcn(){}
  
  virtual double operator()(const std::vector<double>& parameters) const;

  virtual double Up() const {return theErrorDef;}
  inline void SetErrorDef(double def) {theErrorDef = def;}

  inline const std::vector<signal>* GetData() const { return &fSignals; }

  void TestSignals() const;

private:
  std::vector<signal> fSignals;
  double theErrorDef;
};

class fitSignals
{
public:
  //  fitSignals(std::vector<signal>);
  fitSignals(std::vector<signal>, int n=1);
  ~fitSignals() {}

  inline void SetStart(double* s) { for(uint i=0; i<fNpar; ++i) fStart[i]=s[i]; }
  inline void SetStart(int i, double s) { fStart[i]=s; }
  inline const std::vector<double> GetStart() const { return fStart; }
  
  inline void SetStep(double* s) { for(uint i=0; i<fNpar; ++i) fStep[i]=s[i]; }
  inline const std::vector<double> GetStep() const { return fStep; }  

  void Fit();

  void CalculateDoF();

  inline double GetChi2() const { return fchi2; }
  inline int GetStat()    const { return fStat; }
  inline int GetDoF()     const { return fDoF; }

  inline double GetAmplitude(int i=0) const      { return fAmplitude.at(i); }
  inline double GetMean(int i=0) const           { return fMean.at(i); }
  inline double GetSigma(int i=0) const          { return fSigma.at(i); }
  inline double GetAmplitudeError(int i=0) const { return fAmplitudeError.at(i); }
  inline double GetMeanError(int i=0) const      { return fMeanError.at(i); }
  inline double GetSigmaError(int i=0) const     { return fSigmaError.at(i);}

  inline void SetPrintLevel(int l) { print_level = l; }
  inline void Print() { 
     std::cout<<"fitSignals Result"<<std::endl;
     for( uint i=0; i<fNpar/3; ++i)
        {
           std::cout<<i<<") Amplitude: "<<GetAmplitude(i)<<"\tError: "<<GetAmplitudeError(i)<<std::endl;
           std::cout<<i<<") Mean: "<<GetMean(i)<<"\tError: "<<GetMeanError(i)<<std::endl;
           std::cout<<i<<") Sigma: "<<GetSigma(i)<<"\tError: "<<GetSigmaError(i)<<std::endl;
        }
     std::cout<<"chi^2: "<<GetChi2()<<"\tDegrees Of Freedom: "<<GetDoF()<<std::endl;
  }

private:
  GaussFcn theFCN;

  uint fNpar;
  
  std::vector<double> fStep;
  std::vector<double> fStart;
  int print_level;
  
  int fStat;
  int fDoF;
  double fchi2;

  std::vector<double> fAmplitude;
  std::vector<double> fMean;
  std::vector<double> fSigma;
  std::vector<double> fAmplitudeError;
  std::vector<double> fMeanError;
  std::vector<double> fSigmaError;
};

void SignalsStatistics(std::vector<signal>::const_iterator first,
		       std::vector<signal>::const_iterator last, 
		       double& mean, double& rms);

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
