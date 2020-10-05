#include "HelixFit.hh"
#include <cassert>

#include "Minuit2/VariableMetricMinimizer.h"
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MnUserParameterState.h"

#include <TError.h>

#include "TPCconstants.hh"

HelixFunction::HelixFunction(double f, double d, double c, 
			     double l, double z,
			     double eps) : fphi0(f), fDistance(d),
					   fCurvature(c),
					   flambda(l), fz0(z),
					   feps(eps)
{
  //  fx0=-fDistance*sin(fphi0); fy0=fDistance*cos(fphi0);
}


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
  double rho=1./2.*fCurvature;
  double u0=cos(fphi0),v0=sin(fphi0);
  double u1=u0/rho,v1=v0/rho;
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

double HelixFcnPos::operator()(const std::vector<double>& par) const 
{
  assert(par.size()==5);
  HelixFunction hel(par[0],par[1],par[2],par[3],par[4],1.);
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

double HelixFcnNeg::operator()(const std::vector<double>& par) const 
{
  assert(par.size()==5);
  HelixFunction hel(par[0],par[1],par[2],par[3],par[4],-1.);
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

HelixFit::HelixFit(std::vector<TSpacePoint*> p): theFCNpos(p),theFCNneg(p),fNpar(5),
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
  fDoF = p.size() - fNpar;
  fMomentum.SetXYZ(0.0,0.0,0.0);
  fMomentumError.SetXYZ(0.0,0.0,0.0);
}

void HelixFit::HelixFit::Fit()
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
   ROOT::Minuit2::FunctionMinimum minNeg = theMinimizer.Minimize(theFCNneg, fStart, fStep);   
   gErrorIgnoreLevel = error_level_save;

   // output
   ROOT::Minuit2::MnUserParameterState state;
   if( minPos.Fval() < minNeg.Fval() )
     {
       state = minPos.UserState();
       fBranch=1;
       fchi2 = minPos.Fval();
       fStat = minPos.IsValid();
     }
   else
     {
       state = minNeg.UserState();
       fBranch=-1;
       fchi2 = minNeg.Fval();
       fStat = minNeg.IsValid();
     }

   fphi0   = state.Value(0); ferr2phi0   = state.Error(0)*state.Error(0);
   fD      = state.Value(1); ferr2D      = state.Error(1)*state.Error(1);
   fc      = state.Value(2); ferr2c      = state.Error(2)*state.Error(2);
   flambda = state.Value(2); ferr2lambda = state.Error(3)*state.Error(3);
   fz0     = state.Value(4); ferr2z0     = state.Error(4)*state.Error(4);

   if( print_level > 0 )
     {
       std::cout<<"HelixFit::Fit() chi2 = "<<fchi2<<std::endl;
       std::cout<<"HelixFit::Fit() status: "<<fStat<<std::endl;
     }
}


void HelixFit::Print() const
{ 
  std::cout<<"HelixFit Result"<<std::endl;
  //  std::cout<<"# of points: "<<fNpoints<<std::endl;
  std::cout<<" ("<<std::setw(5)<<std::left<<GetX0()
	   <<", "<<std::setw(5)<<std::left<<GetY0()
	   <<", "<<std::setw(5)<<std::left<<fz0<<")\n"
    //<<" Rc = "<<std::setw(5)<<std::left<<fRc
	   <<" Phi0 = "<<std::setw(5)<<std::left<<fphi0
	   <<"    D = "<<std::setw(5)<<std::left<<fD
	   <<"    L = "<<std::setw(5)<<std::left<<flambda
	   <<std::endl;
  std::cout<<" a = "<<fa<<std::endl;
  std::cout<<" Branch : "<<fBranch<<std::endl;
  std::cout<<" chi^2: "<<GetChi2()<<"\tDegrees Of Freedom: "<<GetDoF()<<std::endl;
  if(fMomentum.Mag()!=0.0)
    {
      std::cout<<" Momentum = ("
	       <<std::setw(5)<<std::left<<fMomentum.X()<<", "
	       <<std::setw(5)<<std::left<<fMomentum.Y()<<", "
	       <<std::setw(5)<<std::left<<fMomentum.Z()<<") MeV/c"<<std::endl;
      std::cout<<" |p| = "<<fMomentum.Mag()
	       <<" MeV/c\t pT = "<<fMomentum.Perp()<<" MeV/c"<<std::endl;
    }
  // if(fResidual.Mag()!=0.0)
  //   std::cout<<"  Residual = ("
  // 	     <<std::setw(5)<<std::left<<fResidual.X()
  // 	     <<", "<<std::setw(5)<<std::left<<fResidual.Y()
  // 	     <<", "<<std::setw(5)<<std::left<<fResidual.Z()<<") mm"<<std::endl;
  // if(fResiduals2!=0.0) 
  //   std::cout<<"  Residuals Squared = "<<fResiduals2<<" mm^2"<<std::endl;
  // if(fParticle!=0)
  //   std::cout<<"PDG code "<<fParticle<<std::endl;
  //  std::cout<<" Status: "<<GetStatus()<<std::endl;
  std::cout<<"--------------------------------------------------------------------------"<<std::endl;
}
