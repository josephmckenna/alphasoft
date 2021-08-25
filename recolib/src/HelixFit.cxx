#include "HelixFit.hh"
#include <iostream>
#include <cassert>

#include "Minuit2/VariableMetricMinimizer.h"
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MnUserParameterState.h"

#include <TError.h>

#include "TPCconstants.hh"


double HelixFunction::beta(double& r) const
{
  double num=r*r-fDistance*fDistance;
  double den=1.+2.*fCurvature*fDistance;
  double frac=num/den;
  if(frac < 0. ) return 0.;
  return fCurvature*sqrt(frac);
}

std::vector<double> HelixFunction::operator()(double r) const 
{
  double b=beta(r);     double b2=b*b;
  double bbb=2.*feps*b*sqrt(1.-b2), bb=2.*b2;
  double x0=-fDistance*sin(fphi0), y0=fDistance*cos(fphi0);
  //fRadiusCurv=1./2.*fCurvature;
  double u0=cos(fphi0),v0=sin(fphi0);
  double u1=u0/fRadiusCurv,v1=v0/fRadiusCurv;
  double s = 1./fCurvature;
  if(feps>0.) s*=asin(b);
  else if(feps<0.) s*=(M_PI-asin(b));
  std::vector<double> xyz
  { 
    x0+u1*bbb-v1*bb,
    y0+v1*bbb+u1*bb,
      fz0+flambda*s
  };
  return xyz;
}

double HelixFcn::operator()(const std::vector<double>& par) const 
{
  assert(par.size()==5);
  HelixFunction hel(par[0],par[1],par[2],par[3],par[4],fEpsilon);
  double chi2=0.,tx,ty,tz;
  for(auto& p: fSpacepoints) 
    {
      double rad = p->GetR();
      std::vector<double> pos = hel(rad);
      tx=(pos[0]-p->GetX())/p->GetErrX();
      ty=(pos[1]-p->GetY())/p->GetErrY();
      tz=(pos[2]-p->GetZ())/p->GetErrZ(); 
      chi2+=tx*tx+ty*ty+tz*tz;
    }
  return chi2;
}


HelixFit::HelixFit(std::vector<TSpacePoint*> p): theFCNpos(p,1.),theFCNneg(p,-1.),fNpar(5),
						 fStep(5,1.e-3),fStart(5,0.0),
						 fc(ALPHAg::kUnknown),fRc(ALPHAg::kUnknown),
						 fphi0(ALPHAg::kUnknown),fD(ALPHAg::kUnknown),
						 flambda(ALPHAg::kUnknown),fz0(ALPHAg::kUnknown),
						 fa(ALPHAg::kUnknown),
						 fx0(ALPHAg::kUnknown),fy0(ALPHAg::kUnknown),
						 ferr2c(ALPHAg::kUnknown),ferr2Rc(ALPHAg::kUnknown),
						 ferr2phi0(ALPHAg::kUnknown),ferr2D(ALPHAg::kUnknown),
						 ferr2lambda(ALPHAg::kUnknown),ferr2z0(ALPHAg::kUnknown),
                                                 fBranch(0),fStat(-1),fchi2(-1.), print_level(-1)
{
  
  fDoF = 3.*p.size() - fNpar;
}

void HelixFit::Fit()
{
   // see:
   // https://root.cern.ch/root/htmldoc/guides/minuit2/Minuit2.html

   // create minimizer (default constructor)
   ROOT::Minuit2::VariableMetricMinimizer theMinimizer; 
   theMinimizer.Builder().SetPrintLevel(print_level);

   int error_level_save = gErrorIgnoreLevel;
   gErrorIgnoreLevel = kFatal;
   // minimize
   ROOT::Minuit2::FunctionMinimum minPos = theMinimizer.Minimize(theFCNpos, fStart, fStep);
   std::cout<<"FunctionMinimum + Fval="<<minPos.Fval()<<" EDM="<<minPos.Edm()<<" : "<<minPos.IsValid()<<std::endl;
   ROOT::Minuit2::FunctionMinimum minNeg = theMinimizer.Minimize(theFCNneg, fStart, fStep);   
   std::cout<<"FunctionMinimum - Fval="<<minNeg.Fval()<<" EDM="<<minNeg.Edm()<<" : "<<minNeg.IsValid()<<std::endl;
   gErrorIgnoreLevel = error_level_save;

   // output
   ROOT::Minuit2::MnUserParameterState state;
   if( minPos.Fval() < minNeg.Fval() && minPos.IsValid())
     {
       state = minPos.UserState();
       fBranch=1;
       fchi2 = minPos.Fval();
       fStat = 3; // using Minuit (old) convention
     }
   else if( minNeg.Fval()< minPos.Fval() && minNeg.IsValid() )
     {
       state = minNeg.UserState();
       fBranch=-1;
       fchi2 = minNeg.Fval();
       fStat = 3; // using Minuit (old) convention
     }
   else return;
     

   fphi0   = state.Value(0); ferr2phi0   = state.Error(0)*state.Error(0);
   fD      = state.Value(1); ferr2D      = state.Error(1)*state.Error(1);
   fc      = state.Value(2); ferr2c      = state.Error(2)*state.Error(2);
   flambda = state.Value(2); ferr2lambda = state.Error(3)*state.Error(3);
   fz0     = state.Value(4); ferr2z0     = state.Error(4)*state.Error(4);

   fRc    = 0.5/fc;           
   double errRc = 0.5*fRc*state.Error(2)/fc;
   ferr2Rc = errRc*errRc;
   
   fx0=-fD*sin(fphi0); fy0=fD*cos(fphi0);

   if( print_level > 0 )
     {
       std::cout<<"HelixFit::Fit() chi2 = "<<fchi2<<std::endl;
       std::cout<<"HelixFit::Fit() status: "<<fStat<<std::endl;
     }
}

std::vector<double> HelixFit::Evaluate(double r) const
{
  std::vector<double> pos(ALPHAg::_null,ALPHAg::_null + sizeof(ALPHAg::_null) / sizeof(double));
  if( fStat < 0 ) return pos;
  HelixFunction hel(fphi0,fD,fc,flambda,fz0);
  pos=hel(r);
  return pos;
}


void HelixFit::Print() const
{ 
  std::cout<<"HelixFit Result"<<std::endl;
  std::cout<<"# of points: "<<theFCNpos.GetSpacepoints()->size()<<std::endl;
  std::cout<<" ("<<std::setw(5)<<std::left<<GetX0()
	   <<", "<<std::setw(5)<<std::left<<GetY0()
	   <<", "<<std::setw(5)<<std::left<<fz0<<")\n"
 	   <<" Phi0 = "<<std::setw(5)<<std::left<<fphi0
	   <<"    D = "<<std::setw(5)<<std::left<<fD
	   <<"    L = "<<std::setw(5)<<std::left<<flambda
	   <<std::endl;
  std::cout<<" Branch : "<<fBranch<<std::endl;
  std::cout<<" chi^2: "<<GetChi2()<<"\tDegrees Of Freedom: "<<GetDoF()<<std::endl;
  std::cout<<"--------------------------------------------------------------------------"<<std::endl;
}
