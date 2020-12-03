#ifndef _MN_HELIXFIT_H_
#define _MN_HELIXFIT_H_ 1

#include <cmath>
#include <vector>
#include "Minuit2/FCNBase.h"
#include "TSpacePoint.hh"
#include <TVector3.h>
#include <iostream>
#include <iomanip>

class HelixFunction {
public:
   HelixFunction(double f, double d, double c, 
                 double l, double z, double eps=1.);

   ~HelixFunction() {}

   double phi0() const { return fphi0;      }
   double D()    const { return fDistance;  }
   double c()    const { return fCurvature; }
   double l()    const { return flambda;    }
   double z0()   const { return fz0;        }

   // double a()    const { return fa;  }
   // double x0()   const { return fx0; }
   // double y0()   const { return fy0; }

   std::vector<double> operator()(double r) const;

private:
   double fphi0;
   double fDistance;
   double fCurvature;

   double flambda;
   double fz0;

   double feps;

   // double fa;
   // double fx0;
   // double fy0;

   double beta(double& r) const;
};

class HelixFcnPos : public ROOT::Minuit2::FCNBase 
{
public:
   HelixFcnPos(const std::vector<TSpacePoint*>& points) : 
      fSpacepoints(points),fErrorDef(1.)
   {}
   ~HelixFcnPos() {}
   
   virtual double operator()(const std::vector<double>&) const;
   virtual double Up() const                               {return fErrorDef;}
   inline std::vector<TSpacePoint*> GetSpacepoints() const {return fSpacepoints;}

   inline void setErrorDef(double def) {fErrorDef = def;}

private:
   std::vector<TSpacePoint*> fSpacepoints;
   double fErrorDef;
};

class HelixFcnNeg : public ROOT::Minuit2::FCNBase 
{
public:
   HelixFcnNeg(const std::vector<TSpacePoint*>& points) : 
      fSpacepoints(points),fErrorDef(1.)
   {}
   ~HelixFcnNeg() {}
   
   virtual double operator()(const std::vector<double>&) const;
   virtual double Up() const                               {return fErrorDef;}
   inline std::vector<TSpacePoint*> GetSpacepoints() const {return fSpacepoints;}

   inline void setErrorDef(double def) {fErrorDef = def;}

private:
   std::vector<TSpacePoint*> fSpacepoints;
   double fErrorDef;
};


class HelixFit{
public:
   HelixFit(std::vector<TSpacePoint*>);
   ~HelixFit() {}

   inline void SetStart(double* s) { for(uint i=0; i<fNpar; ++i) fStart[i]=s[i]; }
   inline void SetStart(int i, double s) { fStart[i]=s; }
   inline const std::vector<double> GetStart() const { return fStart; }
  
   inline void SetStep(double* s) { for(uint i=0; i<fNpar; ++i) fStep[i]=s[i]; }
   inline const std::vector<double> GetStep() const { return fStep; }  

   void Fit();

   inline double GetChi2() const { return fchi2; }
   inline int GetStat()    const { return fStat; }
   inline int GetDoF()     const { return fDoF; }

   inline double GetC() const         {return fc;}
   //   inline double GetRc() const      {return fRc;}
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
   HelixFcnPos theFCNpos;
   HelixFcnNeg theFCNneg;

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

   TVector3 fMomentum;  // MeV/c
   TVector3 fMomentumError;

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
