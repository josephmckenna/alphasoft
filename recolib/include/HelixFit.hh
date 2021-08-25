#ifndef _MN_HELIXFIT_H_
#define _MN_HELIXFIT_H_ 1

#include <cmath>
#include <vector>
#include "Minuit2/FCNBase.h"
#include "TSpacePoint.hh"
#include "TPCconstants.hh"

#include <iostream>
#include <iomanip>

class HelixFunction {
public:
   HelixFunction(double f, double d, double c, 
                 double l, double z, double eps=1.): fphi0(f), fDistance(d),
                                                     fCurvature(c),
                                                     flambda(l), fz0(z),
                                                     feps(eps)
   {
      fRadiusCurv=1./2.*fCurvature;
   }

   virtual ~HelixFunction() {}

   double phi0() const { return fphi0;      }
   double D()    const { return fDistance;  }
   double c()    const { return fCurvature; }
   double Rc()   const { return fRadiusCurv; }
   double l()    const { return flambda;    }
   double z0()   const { return fz0;        }
   const double Branch() const { return feps; }

   virtual std::vector<double> operator()(double r) const;
   double arclength(double&r) const;

private:
   double fphi0;
   double fDistance;
   double fCurvature;
   double fRadiusCurv;

   double flambda;
   double fz0;

   const double feps;
   double beta(double& r) const;
};

class HelixFcn : public ROOT::Minuit2::FCNBase 
{
public:
   HelixFcn(const std::vector<TSpacePoint*>& points, double eps=1.) : 
      fSpacepoints(points),fErrorDef(1.),fEpsilon(eps)
   {}
   ~HelixFcn() {}
   
   virtual double operator()(const std::vector<double>&)    const;
   virtual double Up()                                      const {return fErrorDef;}
   inline const std::vector<TSpacePoint*>* GetSpacepoints() const {return &fSpacepoints;}

   inline void setErrorDef(double def) { fErrorDef = def; }
   //inline void SetBranch(double eps)   { fEpsilon = eps;  }

private:
   const std::vector<TSpacePoint*> fSpacepoints;
   double fErrorDef;
   const double fEpsilon;
};


class HelixFit
{
public:
   HelixFit(std::vector<TSpacePoint*>);
   ~HelixFit() {}

   inline void SetStart(double* s) { for(uint i=0; i<fNpar; ++i) fStart[i]=s[i]; }
   inline void SetStart(int i, double s) { fStart[i]=s; }
   inline const std::vector<double> GetStart() const { return fStart; }
  
   inline void SetStep(double* s) { for(uint i=0; i<fNpar; ++i) fStep[i]=s[i]; }
   inline const std::vector<double> GetStep() const { return fStep; }  

   void Fit();
   std::vector<double> Evaluate(double r) const;

   inline double GetChi2() const { return fchi2; }
   inline int GetStat()    const { return fStat; }
   inline int GetDoF()     const { return fDoF; }

   inline double GetC() const         {return fc;}
   inline double GetRc() const        {return fRc;}
   inline double GetPhi0() const      {return fphi0;}
   inline double GetD() const         {return fD;}
   inline double GetLambda() const    {return flambda;}
   inline double GetZ0() const        {return fz0;}
   inline double GetA() const         {return fa;}
   inline double GetErrC() const      {return ferr2c;}
   inline double GetErrRc() const     {return ferr2Rc;}
   inline double GetErrPhi0() const   {return ferr2phi0;}
   inline double GetErrD() const      {return ferr2D;}
   inline double GetErrLambda() const {return ferr2lambda;}
   inline double GetErrZ0() const     {return ferr2z0;}
   inline double GetX0() const        {return fx0;}
   inline double GetY0() const        {return fy0;}
   
   inline int GetBranch() const {return fBranch;}

   inline void SetPrintLevel(int l) { print_level = l; }
   void Print() const;

private:
   HelixFcn theFCNpos;
   HelixFcn theFCNneg;

   uint fNpar;
  
   std::vector<double> fStep;
   std::vector<double> fStart;

   double fc;
   double fRc;
   double fphi0;
   double fD;
   
   double flambda;
   double fz0;
   
   double fa;
   double fx0;
   double fy0;
   
   double ferr2c;
   double ferr2Rc;
   double ferr2phi0;
   double ferr2D;

   double ferr2lambda;
   double ferr2z0;

   int fBranch;

   int fStat;
   int fDoF;
   double fchi2;

   int print_level;
};

#endif // _MN_HELIXFIT_H_


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
