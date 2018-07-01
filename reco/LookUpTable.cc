// Look-up Table class implementation
// for ALPHA-g TPC analysis
// Author: A.Capra
// Date: Nov 2014

#include "LookUpTable.hh"
#include <TMath.h>
#include <TString.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>


LookUpTable::LookUpTable(int run):finterpol_tdrad(0),finterpol_tdphi(0),
				  fMinTime(-1.),fMaxTime(0.)
{
  if( SetRun( run ) )
    {
      fMinTime = fdrift.front();
      fMaxTime = fdrift.back();
      
      // finterpol_trad = new ROOT::Math::Interpolator(drift, radpos, ROOT::Math::Interpolation::kLINEAR);
      // finterpol_tphi = new ROOT::Math::Interpolator(drift, wire, ROOT::Math::Interpolation::kLINEAR);
      finterpol_trad = new ROOT::Math::Interpolator(fdrift, frad, ROOT::Math::Interpolation::kCSPLINE);
      finterpol_tphi = new ROOT::Math::Interpolator(fdrift, flor, ROOT::Math::Interpolation::kCSPLINE);
    }
}

LookUpTable::LookUpTable(double quencherFrac, double B):finterpol_tdrad(0),finterpol_tdphi(0),
							fMinTime(-1.),fMaxTime(0.)
{
  if( SetGas(quencherFrac, B) )
    {
      fMinTime = fdrift.front();
      fMaxTime = fdrift.back();
      
      // finterpol_trad = new ROOT::Math::Interpolator(drift, radpos, ROOT::Math::Interpolation::kLINEAR);
      // finterpol_tphi = new ROOT::Math::Interpolator(drift, wire, ROOT::Math::Interpolation::kLINEAR);
      finterpol_trad = new ROOT::Math::Interpolator(fdrift, frad, ROOT::Math::Interpolation::kCSPLINE);
      finterpol_tphi = new ROOT::Math::Interpolator(fdrift, flor, ROOT::Math::Interpolation::kCSPLINE);
    }
}

LookUpTable::~LookUpTable()
{
  if(finterpol_trad) delete finterpol_trad;
  if(finterpol_tphi) delete finterpol_tphi;
  if(finterpol_tdrad) delete finterpol_tdrad;
  if(finterpol_tdphi) delete finterpol_tdphi;
}

bool LookUpTable::SetRun( int run )
{
  TString lookup_name = TString::Format("../ana/LookUp_0.00T_STRR%d_fit.dat",run);
  std::ifstream lookup(lookup_name.Data());
  if( !lookup.good() )
    {
      while( run > 1000 )
	{
	  --run;
	  lookup_name = TString::Format("../ana/LookUp_0.00T_STRR%d_fit.dat",run);
	  std::cout<<"LookUpTable::SetRun "<<run<<"\t"<<lookup_name<<std::endl;
	  lookup.open(lookup_name.Data());
	  if( lookup.good() ) break;
	}
    }

  std::cout<<"LookUpTable::SetRun() "<<lookup_name<<" ... ";
  if( lookup.is_open() )
    std::cout<<"OK"<<std::endl;
  else
    {
      std::cout<<"FAIL"<<std::endl;
      return SetGas();
    }

  std::string head;
  std::getline(lookup,head);
  std::string col;
  std::getline(lookup,col);
 
  std::cout<<"LookUpTable:: "<<head<<std::endl;
  
  double r,t,w;
  while(1)
    {
      lookup>>t>>r>>w;
      if( !lookup.good() ) break;
      frad.push_back(r);
      fdrift.push_back(t);
      flor.push_back(w);
    }
  lookup.close();

  return true;
}

bool LookUpTable::SetGas(double quencherFrac, double B )
{
    std::cout << "LookUpTable::SetGas(" << quencherFrac << ", " << B << ')' << std::endl;
    // garfield++ remember to convert cm -> mm
    return false;
}

double LookUpTable::GetRadius(double t)
{
  // parameter in ns
  // returns mm
  if(t < fMinTime || t > fMaxTime) return 0.;
  else return finterpol_trad->Eval(t);
}

double LookUpTable::GetAzimuth(double t)
{
  // parameter in ns
  // returns radians
  if(t < fMinTime || t > fMaxTime) return 0.;
  else return finterpol_tphi->Eval(t);
}

double LookUpTable::GetdRdt(double t)
{
  // parameter in ns
  // returns mm
  //  return finterpol_trad->Deriv(t);
  int error_level_save = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  double r_t = finterpol_trad->Deriv(t);
  gErrorIgnoreLevel = error_level_save;
  return r_t;
}

double LookUpTable::GetdPhidt(double t)
{
  // parameter in ns
  // returns radians
  //  return finterpol_tphi->Deriv(t);
  int error_level_save = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  double p_t = finterpol_tphi->Deriv(t);
  gErrorIgnoreLevel = error_level_save;
  return p_t;
}

double LookUpTable::GetdR(double t)
{
  // parameter in ns
  // returns mm
  if(finterpol_tdrad) return finterpol_tdrad->Eval(t);
  else return 0;
}

double LookUpTable::GetdPhi(double t)  // FIXME: use real errors
{
  // parameter in ns
  // returns radians
  if(finterpol_tdphi) return finterpol_tdphi->Eval(t);
  else return 0;
}
