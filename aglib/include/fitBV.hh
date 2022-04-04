#ifndef __FITBV__
#define  __FITBV__ 1

#include "Minuit2/FCNBase.h"
#include <vector>
#include <cmath>
#include <cassert>
#include <numeric>

class SkewGaussFunction 
{
public:
  SkewGaussFunction(double constant, double mean, double sig, double skew) :
    theConstant(constant), theMean(mean), theSigma(sig), theSkew(skew) {}

  SkewGaussFunction(const std::vector<double> p) :
    theConstant(p[0]), theMean(p[1]), theSigma(p[2]), theSkew(p[3]) {}
  
  ~SkewGaussFunction() {}
  
  inline double m() const {return theMean;}
  inline double s() const {return theSigma;}
  inline double c() const {return theConstant;}
  inline double k() const {return theSkew;}

  double operator()(double x) const 
  {
     double j = 0;
     if (x<m()) j = k()*(m()-x);
     double t=(x-m())/(s()-j);
     return c()*exp(-0.5*t*t);
  }

private:
  double theConstant;
  double theMean;
  double theSigma;
  double theSkew;
};



class SkewGaussFcn: public ROOT::Minuit2::FCNBase 
{
public:
  SkewGaussFcn(std::vector<int>& meas, const std::vector<double>& wei) :
  theMeasurements(meas), theWeights(wei),theErrorDef(1.){}

  ~SkewGaussFcn() {}
  
  virtual double operator()(const std::vector<double>& parameters) const;

  virtual double Up() const {return theErrorDef;}
  inline void SetErrorDef(double def) {theErrorDef = def;}

  std::vector<int> measurements() const {return theMeasurements;}
  std::vector<double> weights() const {return theWeights;}

private:
  std::vector<int> theMeasurements;
  std::vector<double> theWeights;
  double theErrorDef;
};

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
